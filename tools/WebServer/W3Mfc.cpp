/*
Module : W3MFC.CPP
Purpose: Implementation for a simple MFC class encapsulation of a HTTP server
Created: PJN / 22-04-1999
History: PJN / 24-06-1999 1.  Implemented support for "HEAD" command
                          2.  Sample provided now also displays the HTTP verb used
                          3.  Sample provided now also displays the date and time of each request
                          4.  Now fully supports multiple virtual directories
                          5.  Now fully supports URL's with encoded characters
                          6.  Implemented support for "DELETE" command
                          7.  Now returns an "Allow:" HTTP header
                          8.  Timeout for requests is now 90 seconds if built for debug
                          9.  Now supports directory listing
                          10. User name is now displayed in the log window

Copyright (c) 1999 by PJ Naughter.  
All rights reserved.

*/

//////////////// Includes ////////////////////////////////////////////

#include "stdafx.h"
#include "HttpSocket.h"
#include "HttpClient.h"
#include "W3Mfc.h"



//////////////// Macros //////////////////////////////////////////////

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



//////////////// Implementation //////////////////////////////////////

CHttpDirectory::CHttpDirectory(const CHttpDirectory& dir)
{
  *this = dir;
}

CHttpDirectory& CHttpDirectory::operator=(const CHttpDirectory& dir)
{
  m_sAlias = dir.m_sAlias;
  m_sDirectory = dir.m_sDirectory;
  m_sDefaultFile = dir.m_sDefaultFile;
  m_bDirectoryListing = dir.m_bDirectoryListing;

  return *this;
}

void CHttpDirectory::SetAlias(const CString& sAlias)
{
  //Must be some alias
  ASSERT(sAlias.GetLength());

  //Ensure the virtual directory begins with a "\"
  if (sAlias.Find(_T('\\')) == -1)
  {
    if (sAlias.Find(_T('/')) == 0)
      m_sAlias = _T('\\') + sAlias.Mid(1);
    else
      m_sAlias = _T('\\') + sAlias;
  }  
  else
    m_sAlias = sAlias;

  //Ensure there is a \ on the end of the directory if 
  //length is greater than 1
  int nLen = m_sAlias.GetLength();
  if (nLen > 1 && (m_sAlias.GetAt(nLen - 1) != _T('\\')))
    m_sAlias += _T('\\');
}

void CHttpDirectory::SetDirectory(const CString& sDirectory)
{
  m_sDirectory = sDirectory;  
}

void CHttpDirectory::SetDefaultFile(const CString& sDefaultFile)
{
  m_sDefaultFile = sDefaultFile;
}






CHttpServerSettings::CHttpServerSettings()
{
  m_nPort = 80;                  //Default to the standard HTTP port
  m_bBind = FALSE;               //Default to not binding to a specific IP address

  #ifdef _DEBUG
  m_dwIdleClientTimeout = 90000; //Default to client idle timeout of 90 seconds (when in debug mode)
  #else
  m_dwIdleClientTimeout = 30000; //Default to client idle timeout of 30 seconds
  #endif

  //Default root directory will be where the exe is running from
  char sPath[_MAX_PATH];
  GetModuleFileName(NULL, sPath, _MAX_PATH);
  char sDrive[_MAX_DRIVE];   
  char sDir[_MAX_DIR];
  _splitpath_s(sPath, sDrive,_MAX_DRIVE, sDir,_MAX_DIR, NULL,0, NULL,0);
  _makepath_s(sPath, sDrive, sDir, NULL, NULL);
  CHttpDirectory dir;
  dir.SetDirectory(sPath);    
  dir.SetAlias("/");
  dir.SetDefaultFile(_T("default.htm")); //Default filename returned for root requests will be "default.htm"
  m_Directories.Add(dir);

  m_sServerName = _T("W3MFC/1.1"); //Default server name will be the name of the MFC classes i.e "W3MFC" plus the current version number 
  m_pRuntimeClientClass = RUNTIME_CLASS(CHttpClient); //Default class to use is CHttpClient
}

CHttpServerSettings::CHttpServerSettings(const CHttpServerSettings& settings)
{
  *this = settings;
}

CHttpServerSettings& CHttpServerSettings::operator=(const CHttpServerSettings& settings)
{
  m_nPort               = settings.m_nPort;     
  m_Directories.Copy(settings.m_Directories);   
  m_bBind               = settings.m_bBind;            
  m_BindAddress         = settings.m_BindAddress;      
  m_dwIdleClientTimeout = settings.m_dwIdleClientTimeout;
  m_sServerName         = settings.m_sServerName;
  m_pRuntimeClientClass = settings.m_pRuntimeClientClass;
  m_sUsername           = settings.m_sUsername;
  m_sPassword           = settings.m_sPassword;

  return *this;
}





IMPLEMENT_DYNAMIC(CHttpServer, CObject)

CHttpServer::CHttpServer() : m_ListenStartEvent(TRUE)
{
  m_pListenThread = NULL;
}

CHttpServer::~CHttpServer()
{
  Stop();
}

BOOL CHttpServer::Start(const CHttpServerSettings& settings)
{
  ASSERT(m_pListenThread == NULL); //Trying to start an already started server

  //Start the listener thread
  m_Settings = settings;
  m_bListenerRunningOK = FALSE;
  m_bRequestListenStop = FALSE; 
  m_ListenStartEvent.ResetEvent();
  m_pListenThread = AfxBeginThread(ListenSocketFunction, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
  if (m_pListenThread == NULL)
  {
    TRACE(_T("Failed to call to create listener thread, please check settings\n"));
    return FALSE;
  }
  m_pListenThread->m_bAutoDelete = FALSE;
  m_pListenThread->ResumeThread();

  //Wait until the thread has completely started and return the success indicator
  ::WaitForSingleObject(m_ListenStartEvent, INFINITE);
  return m_bListenerRunningOK;
}

BOOL CHttpServer::Wait()
{
  //If the listener thread is running, then just wait for it to exit
  if (m_pListenThread)
  {
    ::WaitForSingleObject(m_pListenThread->m_hThread, INFINITE);
    //delete m_pListenThread;
    //m_pListenThread = NULL;
    return TRUE;
  }
  else
  {
    TRACE(_T("Http server is not running, so no need to wait\n"));
    return FALSE;
  }
}

BOOL CHttpServer::Stop()
{
  //If the listener thread is running, then stop it
  if (m_pListenThread)
  {
    //Signal the listener thread to stop and wait for it to do so
    m_bRequestListenStop = TRUE;
    ::WaitForSingleObject(m_pListenThread->m_hThread, INFINITE);
    delete m_pListenThread;
    m_pListenThread = NULL;
  }

  return TRUE;
}

BOOL CHttpServer::ReverseDNSLookup(in_addr sin_addr, CString& sDomainName)
{
  BOOL bSuccess = FALSE;
  HOSTENT* pHostEnt = gethostbyaddr((const char*) &sin_addr, sizeof(sin_addr), AF_INET); 
  if (pHostEnt)
  {
    bSuccess = TRUE;
    sDomainName = pHostEnt->h_name;
  }

  return bSuccess;
}
                               
void CHttpServer::ListenSocketFunction()
{
  //Create the server socket
  CHttpSocket serverSocket;
  if (!serverSocket.Create())
  {
    TRACE(_T("Failed to create server socket, GetLastError:%d\n"), ::GetLastError());
    m_ListenStartEvent.SetEvent();
    return;
  }

  //Bind the server socket
  sockaddr_in socketAddress;
  socketAddress.sin_family = AF_INET;
  socketAddress.sin_port   = htons((u_short)m_Settings.m_nPort);
  if (m_Settings.m_bBind)
    socketAddress.sin_addr.s_addr = m_Settings.m_BindAddress.s_addr;  //Bind to a specific IP address
  else
    socketAddress.sin_addr.s_addr = htonl(INADDR_ANY); //Bind to any IP address
  if (!serverSocket.Bind(&socketAddress))
  {
    TRACE(_T("Failed to bind server socket, GetLastError:%d\n"), ::GetLastError());
    m_ListenStartEvent.SetEvent();
    return;
  }

  //Put the server socket in a listening state
  if (!serverSocket.Listen())
  {
    TRACE(_T("Failed to listen on server socket, GetLastError:%d\n"), ::GetLastError());
    m_ListenStartEvent.SetEvent();
    return;
  }

  //Run the server under a different account if configured to do so
  BOOL bUseAccount = (m_Settings.m_sUsername.GetLength() != 0);
  HANDLE hImpersonation = NULL;
  BOOL bLoggedOn = FALSE;
  BOOL bImpersonated = FALSE;
  if (bUseAccount)
  {
    LPTSTR pszUser = m_Settings.m_sUsername.GetBuffer(m_Settings.m_sUsername.GetLength());
    LPTSTR pszPassword = m_Settings.m_sPassword.GetBuffer(m_Settings.m_sPassword.GetLength());
    bLoggedOn = LogonUser(pszUser, NULL, pszPassword, LOGON32_LOGON_NETWORK, LOGON32_PROVIDER_DEFAULT, &hImpersonation);
    if (bLoggedOn)
      bImpersonated = ImpersonateLoggedOnUser(hImpersonation);
    else
      TRACE(_T("Failed to logon using user name: %s, GetLastError:%d\n"), pszUser, ::GetLastError());
    m_Settings.m_sUsername.ReleaseBuffer();
    m_Settings.m_sPassword.ReleaseBuffer();
  }  

  //Handle the case if the impersonation failed
  if (bUseAccount && !bImpersonated)
  {
    //Logout from the user account
    if (bLoggedOn)
      CloseHandle(hImpersonation);

    TRACE(_T("Failed to impersonate using supplied user credentials\n"));
    m_ListenStartEvent.SetEvent();
    return;
  }

  //Create the client class instance
  CHttpClient* pClient = (CHttpClient*) m_Settings.m_pRuntimeClientClass->CreateObject();
  ASSERT(pClient);
  ASSERT(pClient->IsKindOf(RUNTIME_CLASS(CHttpClient)));
  pClient->SetServer(this);

  //We're now ready for accepting client connections, inform
  //the main thread that everthing is ok
  m_bListenerRunningOK = TRUE;
  m_ListenStartEvent.SetEvent();

  //Wait for any incoming connections and the signal to 
  //exit the thread m_ListenStopEvent
  while (!m_bRequestListenStop)
  {
    BOOL bReadible;
    if (serverSocket.IsReadible(bReadible, 1000))
    {
      if (bReadible)
      {
        SOCKET clientSocket;
        sockaddr_in clientAddress;
        if (serverSocket.Accept(clientSocket, clientAddress))
        {
          //Let a client class instance handle the work
          pClient->HandleClient(clientSocket, clientAddress);
        }
        else
          TRACE(_T("An error occurred accepting a client connection, GetLastError:%d\n"), ::GetLastError());
      } 
    }
    else
      TRACE(_T("An error occurred checking the readibility of the listening socket, GetLastError:%d\n"), ::GetLastError());
  }

  //Don't forget to free up the memory allocated
  delete pClient;

  //Revert back to normal security settings
  if (bUseAccount)
  {
    //Revert to the usual security settings
    if (bImpersonated)
      RevertToSelf();

    //Logout from the user account
    if (bLoggedOn)
      CloseHandle(hImpersonation);
  }
}

UINT CHttpServer::ListenSocketFunction(LPVOID pParam)
{
  //Get back the "this" pointer from the pParam parameter
  CHttpServer* pServer = (CHttpServer*) pParam;
  ASSERT(pServer);
  ASSERT(pServer->IsKindOf(RUNTIME_CLASS(CHttpServer)));

  //Call the run method of the CHttpServer instance
  pServer->ListenSocketFunction();

  //Return the thread exit code
  return TRUE;
}












