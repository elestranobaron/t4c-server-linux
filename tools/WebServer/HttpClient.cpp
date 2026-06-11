/*
Module : HTTPCLIENT.CPP
Purpose: Implementation for the CHttpClient class
Created: PJN / 22-04-1999

Copyright (c) 1999 by PJ Naughter.  
All rights reserved.

*/


//////////////// Includes ////////////////////////////////////////////

#include "stdafx.h"
#include "..\resource.h"
#include <afxpriv.h>
#include "W3Mfc.h"
#include "HttpResponseHeader.h"
#include "Base64.h"
#include "HttpClient.h"



//////////////// Macros //////////////////////////////////////////////


//Taken from the winuser.h file in the platform SDK.
#ifndef RT_HTML
#define RT_HTML         MAKEINTRESOURCE(23)
#endif


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



//////////////// Implementation //////////////////////////////////////

CHttpRequest::CHttpRequest()
{
  m_Verb = HTTP_VERB_UNKNOWN;
  m_dwHttpVersion = 0;
  m_bIfModifiedSincePresent = FALSE; 
  ZeroMemory(&m_IfModifiedSince, sizeof(SYSTEMTIME));
  m_AuthorizationType = HTTP_AUTHORIZATION_ANONYMOUS;
}

CHttpRequest& CHttpRequest::operator=(const CHttpRequest& request)
{
  m_Verb                    = request.m_Verb;
  m_ClientAddress           = request.m_ClientAddress;
  m_sURL                    = request.m_sURL;
  m_sExtra                  = request.m_sExtra;
  m_dwHttpVersion           = request.m_dwHttpVersion;
  m_bIfModifiedSincePresent = request.m_bIfModifiedSincePresent; 
  CopyMemory(&m_IfModifiedSince, &request.m_IfModifiedSince, sizeof(SYSTEMTIME));
  m_AuthorizationType       = request.m_AuthorizationType;
  m_sUsername               = request.m_sUsername;
  m_sPassword               = request.m_sPassword;

  return *this;
}





IMPLEMENT_DYNCREATE(CHttpClient, CObject)

CHttpClient::CHttpClient()
{
}

void CHttpClient::SetServer(CHttpServer* pServer)
{
  //Validate our parameters
  ASSERT(pServer);

  m_pServer = pServer;
}

CHttpClient::~CHttpClient()
{
  m_pServer = NULL;
}

BOOL CHttpClient::AllowThisConnection()
{
  return TRUE;
}

void CHttpClient::HandleClient(SOCKET hSocket, sockaddr_in ClientAddress)
{
  //Validate our parameters
  ASSERT(hSocket != INVALID_SOCKET);
  ASSERT(m_pServer);

  //Hive away the parameters
  m_Socket.Attach(hSocket);
  CopyMemory(&m_Request.m_ClientAddress, &ClientAddress, sizeof(sockaddr_in));

  //Read the client request
  char sRequest[1024];
  LPSTR pszOverFlowBuffer = NULL;
  char* sRequestReturned = sRequest;
  if (m_Socket.ReadResponse(sRequest, 1024, "\r\n\r\n", &pszOverFlowBuffer, m_pServer->m_Settings.m_dwIdleClientTimeout))
  {
    //Point to the overflow buffer if it is valid
    if (pszOverFlowBuffer)
      sRequestReturned = pszOverFlowBuffer;

    //Parse the client request
    if (ParseRequest(sRequestReturned) && AllowThisConnection())
    {
      //Impersonate the client credentials if authorization type is PLAINTEXT
      HANDLE hImpersonation = NULL;
      BOOL bLoggedOn = FALSE;
      if (m_Request.m_AuthorizationType == CHttpRequest::HTTP_AUTHORIZATION_PLAINTEXT)
      {
        LPTSTR pszUser = m_Request.m_sUsername.GetBuffer(m_Request.m_sUsername.GetLength());
        LPTSTR pszPassword = m_Request.m_sPassword.GetBuffer(m_Request.m_sPassword.GetLength());
        bLoggedOn = LogonUser(pszUser, NULL, pszPassword, LOGON32_LOGON_NETWORK, LOGON32_PROVIDER_DEFAULT, &hImpersonation);
        if (bLoggedOn)
          ImpersonateLoggedOnUser(hImpersonation);
        else
          TRACE(_T("Failed to logon using user name: %s, GetLastError:%d\n"), pszUser, ::GetLastError());
        m_Request.m_sUsername.ReleaseBuffer();
        m_Request.m_sPassword.ReleaseBuffer();
      }  

      if (m_Request.m_Verb == CHttpRequest::HTTP_VERB_GET || m_Request.m_Verb == CHttpRequest::HTTP_VERB_HEAD)
      {
        CString sLocalFile;
        BOOL bDirectory;
        if (MapURLToLocalFilename(sLocalFile, bDirectory))
        {
          if (bDirectory)
            TransmitDirectory(sLocalFile); //Return a directory listing back to the client
          else
            TransmitFile(sLocalFile); //Return the file back to the client
        }
        else
          ReturnErrorMessage(404); //Not Found
      }
      else if (m_Request.m_Verb == CHttpRequest::HTTP_VERB_DELETE)
      {
        //By default, only allow deletion of a file if we are using authorization
        if (m_Request.m_AuthorizationType != CHttpRequest::HTTP_AUTHORIZATION_ANONYMOUS)
        {
          CString sLocalFile;
          BOOL bDirectory;
          if (MapURLToLocalFilename(sLocalFile, bDirectory) && !bDirectory)
          {
            if (DeleteFile(sLocalFile))
              ReturnFileDeletedOkMessage(sLocalFile);
            else
            {
              if (::GetLastError() == ERROR_ACCESS_DENIED && !bDirectory)
                ReturnBasicUnauthorizedMessage(m_Request.m_sURL);
              else 
                ReturnErrorMessage(500); //Internal server error
            }
          }
          else
            ReturnErrorMessage(404); //Not Found
        }
        else
          ReturnBasicUnauthorizedMessage(m_Request.m_sURL); //Not authorized
      }
      else
        ReturnErrorMessage(501); //Not implemented

      //Restore our usual security priviledges
      if (m_Request.m_AuthorizationType == CHttpRequest::HTTP_AUTHORIZATION_PLAINTEXT)
      {
        //Revert to the usual security settings
        RevertToSelf();

        //Logout from the user account
        if (bLoggedOn)
          CloseHandle(hImpersonation);
      }
    }
    else
      ReturnErrorMessage(400); //Bad Request
  }

  //Don't forget to delete the overflow buffer if any
  if (pszOverFlowBuffer)
    delete [] pszOverFlowBuffer;

  //Close the socket
  m_Socket.Close();

  //Reset the request data
  m_Request = CHttpRequest();
}

BOOL CHttpClient::ParseSimpleRequestLine(LPSTR pszLine)
{
  //Validate our parameters
  ASSERT(pszLine);

  BOOL bSuccess = FALSE;

  //First parse out the VERB
  char *next_token1;
  char seps[] = " ";
  char* pszVerb = strtok_s(pszLine, seps,&next_token1);
  if (pszVerb)
  {
    if (_strcmpi(pszVerb, "GET") == 0)
      m_Request.m_Verb = CHttpRequest::HTTP_VERB_GET;
    else if (_strcmpi(pszVerb, "POST") == 0)
      m_Request.m_Verb = CHttpRequest::HTTP_VERB_POST;
    else if (_strcmpi(pszVerb, "HEAD") == 0)
      m_Request.m_Verb = CHttpRequest::HTTP_VERB_HEAD;
    else if (_strcmpi(pszVerb, "PUT") == 0)
      m_Request.m_Verb = CHttpRequest::HTTP_VERB_PUT;
    else if (_strcmpi(pszVerb, "LINK") == 0)
      m_Request.m_Verb = CHttpRequest::HTTP_VERB_LINK;
    else if (_strcmpi(pszVerb, "DELETE") == 0)
      m_Request.m_Verb = CHttpRequest::HTTP_VERB_DELETE;
    else if (_strcmpi(pszVerb, "UNLINK") == 0)
      m_Request.m_Verb = CHttpRequest::HTTP_VERB_UNLINK;
    else
      m_Request.m_Verb = CHttpRequest::HTTP_VERB_UNKNOWN;

    //Parse out the URL
    char* pszURL = strtok_s(NULL, seps,&next_token1);
    if (pszURL)
    {
      //Convert any embedded escape sequences to their unencoded format
      m_Request.m_sURL = URLDecode(pszURL);

      //Handle the case where the username and password
      //are passed as part of the URL
      int nAmpersand = m_Request.m_sURL.Find(_T('@'));
      if (nAmpersand != -1)
      {
        CString sAuth = m_Request.m_sURL.Left(nAmpersand);
        int nColon = m_Request.m_sURL.Find(_T(':'));
        if (nColon != -1)
        {
          m_Request.m_sUsername = m_Request.m_sURL.Left(nColon);
          m_Request.m_sPassword = m_Request.m_sURL.Mid(nColon+1, nAmpersand-nColon-1);
        }
        else
          m_Request.m_sUsername = sAuth;

        m_Request.m_sURL = m_Request.m_sURL.Right(m_Request.m_sURL.GetLength() - nAmpersand - 1);
        m_Request.m_AuthorizationType = CHttpRequest::HTTP_AUTHORIZATION_PLAINTEXT;
      }

      //Handle the Search path i.e everything after the ?
      int nQuestion = m_Request.m_sURL.Find(_T('?'));
      if (nQuestion != -1)
      {
        m_Request.m_sExtra = m_Request.m_sURL.Right(m_Request.m_sURL.GetLength() - nQuestion - 1);
        m_Request.m_sURL = m_Request.m_sURL.Left(nQuestion);
      }

      //Parse out the HTTP version
      char* pszVersion = strtok_s(NULL, seps,&next_token1);
      if (pszVersion)
      {
        if (strstr(pszVersion, "HTTP/") == pszVersion)
        {
          char sepsVer[] = ".";
          char* pszMajorVersion = strtok_s(pszVersion+5, sepsVer,&next_token1);
          if (pszMajorVersion)
          {
            WORD wMajorVersion = (WORD) atoi(pszMajorVersion);
            char* pszMinorVersion = strtok_s(NULL, sepsVer,&next_token1);
            if (pszMinorVersion)
            {
              WORD wMinorVersion = (WORD) atoi(pszMinorVersion);
              m_Request.m_dwHttpVersion = MAKELONG(wMinorVersion, wMajorVersion);
              bSuccess = TRUE;
            }
          }
        }
      }
      else
      {
        //No version included in the request, so set it to HTTP v0.9
        m_Request.m_dwHttpVersion = MAKELONG(9, 0);
        bSuccess = m_Request.m_Verb == CHttpRequest::HTTP_VERB_GET; //"GET" is only allowed with HTTP v0.9
      }  
    }
  }

  return bSuccess;
}

BOOL CHttpClient::ParseWeekDay(char* pszToken, int& nWeekDay)
{
  BOOL bSuccess = TRUE;
  if (strcmp(pszToken, "Sun") == 0 || strcmp(pszToken, "Sunday") == 0)
    nWeekDay = 0;
  else if (strcmp(pszToken, "Mon") == 0 || strcmp(pszToken, "Monday") == 0)
    nWeekDay = 1;
  else if (strcmp(pszToken, "Tue") == 0 || strcmp(pszToken, "Tuesday") == 0)
    nWeekDay = 2;
  else if (strcmp(pszToken, "Wed") == 0 || strcmp(pszToken, "Wednesday") == 0)
    nWeekDay = 3;
  else if (strcmp(pszToken, "Thu") == 0 || strcmp(pszToken, "Thursday") == 0)
    nWeekDay = 4;
  else if (strcmp(pszToken, "Fri") == 0 || strcmp(pszToken, "Friday") == 0)
    nWeekDay = 5;
  else if (strcmp(pszToken, "Sat") == 0 || strcmp(pszToken, "Saturday") == 0)
    nWeekDay = 6;
  else
    bSuccess = FALSE;
  return bSuccess;
}

BOOL CHttpClient::ParseMonth(char* pszToken, int& nMonth)
{
  BOOL bSuccess = TRUE;
  if (strcmp(pszToken, "Jan") == 0)
    nMonth = 1;
  else if (strcmp(pszToken, "Feb") == 0)
    nMonth = 2;
  else if (strcmp(pszToken, "Mar") == 0)
    nMonth = 3;
  else if (strcmp(pszToken, "Apr") == 0)
    nMonth = 4;
  else if (strcmp(pszToken, "May") == 0)
    nMonth = 5;
  else if (strcmp(pszToken, "Jun") == 0)
    nMonth = 6;
  else if (strcmp(pszToken, "Jul") == 0)
    nMonth = 7;
  else if (strcmp(pszToken, "Aug") == 0)
    nMonth = 8;
  else if (strcmp(pszToken, "Sep") == 0)
    nMonth = 9;
  else if (strcmp(pszToken, "Oct") == 0)
    nMonth = 10;
  else if (strcmp(pszToken, "Nov") == 0)
    nMonth = 11;
  else if (strcmp(pszToken, "Dec") == 0)
    nMonth = 12;
  else
    bSuccess = FALSE;
  return bSuccess;
}

BOOL CHttpClient::ParseDate(const CString& sField, SYSTEMTIME& time)
{
  //This method understands RFC 1123, RFC 850 and asctime formats

	//For correct operation of the T2A macro, see MFC Tech Note 59
  USES_CONVERSION;

  BOOL bSuccess = FALSE;

  //Make a local copy of the field we are going to parse
  char* pszField = T2A((LPTSTR) (LPCTSTR) sField);

  //Http times never include a millisecond field, so just set it to zero
  time.wMilliseconds = 0;

  int nLength = strlen(pszField);
  if (nLength > 5)
  {
    if (pszField[3] == ',') //Parsing a RFC 1123 format date
    {
      //First the weekday
      char seps[] = ", :";
      char *next_token1;
      char* pszToken = strtok_s(pszField, seps,&next_token1);
      if (pszToken == NULL)
        return FALSE;
      int nWeekDay;
      bSuccess = ParseWeekDay(pszToken, nWeekDay);
      if (bSuccess)
        time.wDayOfWeek = (WORD) nWeekDay;

      //Then the day of the month
      pszToken = strtok_s(NULL, seps,&next_token1);
      if (pszToken == NULL)
        return FALSE;
      time.wDay = (WORD) atoi(pszToken);

      //Then the month
      pszToken = strtok_s(NULL, seps,&next_token1);
      if (pszToken == NULL)
        return FALSE;
      int nMonth = 0;
      bSuccess = bSuccess && ParseMonth(pszToken, nMonth);
      if (bSuccess)
        time.wMonth = (WORD) nMonth;

      //And the year
      pszToken = strtok_s(NULL, seps,&next_token1);
      if (pszToken == NULL)
        return FALSE;
      time.wYear = (WORD) atoi(pszToken);

      //And the hour
      pszToken = strtok_s(NULL, seps,&next_token1);
      if (pszToken == NULL)
        return FALSE;
      time.wHour = (WORD) atoi(pszToken);

      //And the minute
      pszToken = strtok_s(NULL, seps,&next_token1);
      if (pszToken == NULL)
        return FALSE;
      time.wMinute = (WORD) atoi(pszToken);

      //And the second
      pszToken = strtok_s(NULL, seps,&next_token1);
      if (pszToken == NULL)
        return FALSE;
      time.wSecond = (WORD) atoi(pszToken);
    }
    else if (pszField[3] == ' ') //Parsing an asctime format date
    {
      //First the weekday
      char seps[] = ", :";
      char *next_token1;
      char* pszToken = strtok_s(pszField, seps,&next_token1);
      if (pszToken == NULL)
        return FALSE;
      int nWeekDay;
      bSuccess = ParseWeekDay(pszToken, nWeekDay);
      if (bSuccess)
        time.wDayOfWeek = (WORD) nWeekDay;

      //Then the month
      pszToken = strtok_s(NULL, seps,&next_token1);
      if (pszToken == NULL)
        return FALSE;
      int nMonth = 0;
      bSuccess = bSuccess && ParseMonth(pszToken, nMonth);
      if (bSuccess)
        time.wMonth = (WORD) nMonth;

      //Then the day of the month
      pszToken = strtok_s(NULL, seps,&next_token1);
      if (pszToken == NULL)
        return FALSE;
      time.wDay = (WORD) atoi(pszToken);

      //And the hour
      pszToken = strtok_s(NULL, seps,&next_token1);
      if (pszToken == NULL)
        return FALSE;
      time.wHour = (WORD) atoi(pszToken);

      //And the minute
      pszToken = strtok_s(NULL, seps,&next_token1);
      if (pszToken == NULL)
        return FALSE;
      time.wMinute = (WORD) atoi(pszToken);

      //And the second
      pszToken = strtok_s(NULL, seps,&next_token1);
      if (pszToken == NULL)
        return FALSE;
      time.wSecond = (WORD) atoi(pszToken);

      //And the year
      pszToken = strtok_s(NULL, seps,&next_token1);
      if (pszToken == NULL)
        return FALSE;
      time.wYear = (WORD) atoi(pszToken);
    }
    else //Must be a RFC 850 format date
    {
      //First the weekday
      char seps[] = ", :-";
      char *next_token1;
      char* pszToken = strtok_s(pszField, seps,&next_token1);
      if (pszToken == NULL)
        return FALSE;
      int nWeekDay;
      bSuccess = ParseWeekDay(pszToken, nWeekDay);
      if (bSuccess)
        time.wDayOfWeek = (WORD) nWeekDay;

      //Then the day of the month
      pszToken = strtok_s(NULL, seps,&next_token1);
      if (pszToken == NULL)
        return FALSE;
      time.wDay = (WORD) atoi(pszToken);

      //Then the month
      pszToken = strtok_s(NULL, seps,&next_token1);
      if (pszToken == NULL)
        return FALSE;
      int nMonth = 0;
      bSuccess = bSuccess && ParseMonth(pszToken, nMonth);
      if (bSuccess)
        time.wMonth = (WORD) nMonth;

      //And the year (2 Digits only, so make some intelligent assumptions)
      pszToken = strtok_s(NULL, seps,&next_token1);
      if (pszToken == NULL)
        return FALSE;
      time.wYear = (WORD) atoi(pszToken);
      if (time.wYear < 50)
        time.wYear += 2000;
      else if (time.wYear < 100)
        time.wYear += 1900; 

      //And the hour
      pszToken = strtok_s(NULL, seps,&next_token1);
      if (pszToken == NULL)
        return FALSE;
      time.wHour = (WORD) atoi(pszToken);

      //And the minute
      pszToken = strtok_s(NULL, seps,&next_token1);
      if (pszToken == NULL)
        return FALSE;
      time.wMinute = (WORD) atoi(pszToken);

      //And the second
      pszToken = strtok_s(NULL, seps,&next_token1);
      if (pszToken == NULL)
        return FALSE;
      time.wSecond = (WORD) atoi(pszToken);
    }
  }

  return bSuccess;
}

BOOL CHttpClient::ParseAuthorizationBasic(const CString& sField, CString& sUsername, CString& sPassword)
{
	//For correct operation of the T2A macro, see MFC Tech Note 59
  USES_CONVERSION;

  BOOL bSuccess = FALSE;

  //Make a local copy of the field we are going to parse
  char* pszField = T2A((LPTSTR) (LPCTSTR) sField);

  //Parse out the base64 encoded username and password 
  char seps[] = " ";
  char *next_token1;
  char* pszToken = strtok_s(pszField, seps,&next_token1);
  if (pszToken && strcmp(pszToken, "Basic") == 0)
  {
    pszToken = strtok_s(NULL, seps,&next_token1);
    if (pszToken)
    {
      //Decode the base64 string passed to us
      CString sInput(pszToken);
      CString sOutput;
      CBase64Decoder decoder;
      if (decoder.Decode(sInput, sOutput))
      {
        int nColon = sOutput.Find(_T(":"));
        if (nColon != -1)
        {
          sUsername = sOutput.Left(nColon);
          sPassword = sOutput.Right(sOutput.GetLength()-nColon-1);
          bSuccess = TRUE;
        }
      }
    }
  }

  return bSuccess;
}

BOOL CHttpClient::SplitRequestLine(LPSTR pszLine, CString& sField, CString& sValue)
{
  BOOL bSuccess = FALSE;
  
  //Find the first ":" in the line
  CString sLine(pszLine);
  int nColon = sLine.Find(_T(':'));
  if (nColon != -1)
  {
    sField = sLine.Left(nColon);
    sValue = sLine.Right(sLine.GetLength()-nColon-1);

    //Trim any leading and trailing spaces
    sField.TrimLeft();
    sField.TrimRight();
    sValue.TrimLeft();
    sValue.TrimRight();
  
    bSuccess = TRUE;
  }

  return bSuccess;
}

BOOL CHttpClient::ParseRequest(LPSTR pszRequest)
{ 
  BOOL bSuccess = FALSE;
 
  //Process each line
  BOOL bFirstLine = TRUE;
  LPSTR pszLine = pszRequest;
  LPSTR pszTerminator = strstr(pszLine, "\n");
  BOOL bMoreLines = TRUE;
  do 
  {
    if (pszTerminator)
    {
      pszTerminator[0] = '\0';
      char* pszPrevTerminator = pszTerminator-1;
      if (pszPrevTerminator && pszPrevTerminator[0] == '\r')
        pszPrevTerminator[0] = '\0';
    }  
    else
      bMoreLines = FALSE;

    //Parse the current request line
    CString sField;
    CString sValue;
    if (bFirstLine)
    {
      //Handle the first line
      bSuccess = ParseSimpleRequestLine(pszLine);
      bFirstLine = FALSE;
    }
    else if (SplitRequestLine(pszLine, sField, sValue))
    {
      //Handle any other request headers  

      if (sField == _T("If-Modified-Since"))
      {
        //Handle the If-Modified-Since header
        SYSTEMTIME time;
        if (ParseDate(sValue, time))
        {
          m_Request.m_bIfModifiedSincePresent = TRUE; 
          CopyMemory(&m_Request.m_IfModifiedSince, &time, sizeof(SYSTEMTIME));
        }
      }
      else if (sField == _T("Authorization"))
      {
        //Handle the Authorization header
        CString sUsername;
        CString sPassword;
        if (ParseAuthorizationBasic(sValue, sUsername, sPassword))
        {
          m_Request.m_AuthorizationType = CHttpRequest::HTTP_AUTHORIZATION_PLAINTEXT;
          m_Request.m_sUsername = sUsername;
          m_Request.m_sPassword = sPassword;
        }
      }
    }

    //Move onto the next line
    if (pszTerminator)
    {
      pszLine = pszTerminator+1;
      pszTerminator = strstr(pszLine, "\n");
    }
  }
  while (bMoreLines);

  return bSuccess;
}

void CHttpClient::ReturnErrorMessage(int nStatusCode)
{
  //Form the body of the response
  char* pszBody = NULL;
  DWORD dwBodyLength = 0;
  switch (nStatusCode)
  {
    case 400: VERIFY(LoadHTMLResource(IDH_400, pszBody, dwBodyLength)); break; //Bad Request
    case 404: VERIFY(LoadHTMLResource(IDH_404, pszBody, dwBodyLength)); break; //Not found  
    case 500: VERIFY(LoadHTMLResource(IDH_500, pszBody, dwBodyLength)); break; //Internal server error 
    case 501: VERIFY(LoadHTMLResource(IDH_501, pszBody, dwBodyLength)); break; //Not implemented
    default:  ASSERT(FALSE);                                            break;
  }

  //Form the header of the response
  if (m_Request.m_dwHttpVersion > MAKELONG(9, 0)) //No header sent for HTTP v0.9
  {
    CHttpResponseHeader responseHdr;
    responseHdr.AddStatusCode(nStatusCode);
    SYSTEMTIME st;
    GetSystemTime(&st);
    responseHdr.AddDate(st);
    responseHdr.AddServer(m_pServer->m_Settings.m_sServerName);
    responseHdr.AddW3MfcAllowFields();
    responseHdr.AddContentLength(dwBodyLength);
    responseHdr.AddContentType(_T("text/html"));

    //Send the header
    responseHdr.Send(m_Socket);
  }

  //Send the body
  m_Socket.Send(pszBody, dwBodyLength);

  //Don't forget to free up the memory
  delete [] pszBody;
}

BOOL CHttpClient::LoadHTMLResource(UINT nID, char*& pszHTML, DWORD& dwSize)
{
  BOOL bSuccess = FALSE;

  HMODULE hModule = AfxGetResourceHandle();
  HRSRC hRsrc = ::FindResource(hModule, MAKEINTRESOURCE(nID), RT_HTML);
  if (hRsrc)
  {
    //Load up the resource
    dwSize = ::SizeofResource(hModule, hRsrc); 
    HGLOBAL hGlobal = ::LoadResource(AfxGetResourceHandle(), hRsrc);

    //Allocate a new char array and copy the HTML resource into it 
    if (hGlobal)
    {
      pszHTML = new char[dwSize + 1];
      char* pszResource = (char*) ::LockResource(hGlobal);
      if (pszResource)
      {
        strncpy_s(pszHTML,dwSize + 1, pszResource, dwSize);
        pszHTML[dwSize] = _T('\0');
        bSuccess = TRUE;
      }
      else
        TRACE(_T("Failed to load HTML resource, GetLastError:%d\n"), GetLastError());
    }
  }

  return bSuccess;
}

void CHttpClient::ReturnRedirectMessage(const CString& sURL)
{
  //Form the body of the response
  char* pszBody;
  DWORD dwBodyLength;
  VERIFY(LoadHTMLResource(IDH_301, pszBody, dwBodyLength));

  //Form the header of the response
  CHttpResponseHeader responseHdr;
  responseHdr.AddStatusCode(301);
  SYSTEMTIME st;
  GetSystemTime(&st);
  responseHdr.AddDate(st);
  responseHdr.AddServer(m_pServer->m_Settings.m_sServerName);
  responseHdr.AddW3MfcAllowFields();
  responseHdr.AddLocation(sURL);
  responseHdr.AddContentLength(dwBodyLength);
  responseHdr.AddContentType(_T("text/html"));

  //Send the header
  responseHdr.Send(m_Socket);

  //Send the body
  m_Socket.Send(pszBody, dwBodyLength);

  //Don't forget to free up the memory
  delete [] pszBody;
}

void CHttpClient::ReturnBasicUnauthorizedMessage(const CString& sRealm)
{
  //Form the body of the response
  char* pszBody;
  DWORD dwBodyLength;
  VERIFY(LoadHTMLResource(IDH_401, pszBody, dwBodyLength));

  //Form the header of the response
  CHttpResponseHeader responseHdr;
  responseHdr.AddStatusCode(401);
  SYSTEMTIME st;
  GetSystemTime(&st);
  responseHdr.AddDate(st);
  responseHdr.AddServer(m_pServer->m_Settings.m_sServerName);
  responseHdr.AddW3MfcAllowFields();
  responseHdr.AddWWWBasicAuthenticate(sRealm);
  responseHdr.AddContentLength(dwBodyLength);
  responseHdr.AddContentType(_T("text/html"));

  //Send the header
  responseHdr.Send(m_Socket);

  //Send the body
  m_Socket.Send(pszBody, dwBodyLength);

  //Don't forget to free up the memory
  delete [] pszBody;
}

void CHttpClient::ReturnFileDeletedOkMessage(const CString& /*sFile*/)
{
  //Form the body of the response
  char* pszBody;
  DWORD dwBodyLength;
  VERIFY(LoadHTMLResource(IDH_FILE_DELETED_OK, pszBody, dwBodyLength));

  //Form the header of the response
  CHttpResponseHeader responseHdr;
  responseHdr.AddStatusCode(200);
  SYSTEMTIME st;
  GetSystemTime(&st);
  responseHdr.AddDate(st);
  responseHdr.AddServer(m_pServer->m_Settings.m_sServerName);
  responseHdr.AddW3MfcAllowFields();
  responseHdr.AddContentLength(dwBodyLength);
  responseHdr.AddContentType(_T("text/html"));

  //Send the header
  responseHdr.Send(m_Socket);

  //Send the body
  m_Socket.Send(pszBody, dwBodyLength);

  //Don't forget to free up the memory
  delete [] pszBody;
}

void CHttpClient::TransmitDirectory(const CString& sDirectory)
{
	//For correct operation of the T2A macro, see MFC Tech Note 59
  USES_CONVERSION;

  //Look for all files in the specified directory
  CFileFind finder;
  CString strWildcard(sDirectory);
  strWildcard += _T("\\*.*");
  BOOL bWorking = finder.FindFile(strWildcard);
  if (bWorking)
  {
    //Load up the template of the body
    CString sTemp;
    sTemp.LoadString(IDS_DIRECTORY_LISTING_HEADER);
    CString sBody(sTemp);

    FILETIME ftLastModified;
    ZeroMemory(&ftLastModified, sizeof(FILETIME));
    BOOL bHaveLastModified = FALSE;

    //Iterate through all the files in this directory
    sBody += _T("<table>\r\n");
    while (bWorking)
    {
      bWorking = finder.FindNextFile();
      if (bWorking && !finder.IsDots())
      {
        //Get the last modified time for the file
        CString sLine;
        FILETIME ft;
        finder.GetLastWriteTime(&ft);
        if (CompareFileTime(&ft, &ftLastModified) > 0)
        {
          CopyMemory(&ftLastModified, &ft, sizeof(FILETIME));
          bHaveLastModified = TRUE;
        }

        //Get the URL of the file
        CString sFilename = finder.GetFileName();
        CString sURL = m_Request.m_sURL + sFilename;

        //Get the last modified date as a string
        char sDate[20];
        SYSTEMTIME st;
        FileTimeToSystemTime(&ft, &st);
        GetDateFormat(LOCALE_SYSTEM_DEFAULT, LOCALE_NOUSEROVERRIDE, &st, NULL, sDate, 20);

        //Get the last modified time as a string
        char sTod[20];
        GetTimeFormat(LOCALE_SYSTEM_DEFAULT, LOCALE_NOUSEROVERRIDE, &st, NULL, sTod, 20);

        //Form all the info into a row of the table
        sLine.Format(_T("<tr>\r\n<td><a href=%s>%s</a></td><td>%dKB</td><td>%s</td><td>%s</td>\r\n</tr>"), 
                     sURL, sFilename, (finder.GetLength()+1023)/1024, sDate, sTod);
        sBody += sLine;
      }
    }
    finder.Close();
    sBody += _T("</table>\r\n");
    sTemp.LoadString(IDS_DIRECTORY_LISTING_FOOTER);
    sBody += sTemp;

    //replace any "%1"'s with m_Request.m_sURL
    int nPercent = sBody.Find(_T("%1"));
    while (nPercent != -1)
    {
      sBody = sBody.Left(nPercent) + m_Request.m_sURL + sBody.Right(sBody.GetLength() - nPercent - 2);
      nPercent = sBody.Find(_T("%1"));
    }

    //Make a local copy of the field we are going to parse
    char* pszBody = T2A((LPTSTR) (LPCTSTR) sBody);

    //Get the current system time in UTC
    SYSTEMTIME stCurTime;
    FILETIME ftCurTime;
    ::GetSystemTime(&stCurTime);
    ::SystemTimeToFileTime(&stCurTime, &ftCurTime);

    //Form the header of the response
    CHttpResponseHeader responseHdr;
    responseHdr.AddStatusCode(200);
    responseHdr.AddDate(stCurTime);
    responseHdr.AddServer(m_pServer->m_Settings.m_sServerName);
    responseHdr.AddW3MfcAllowFields();
    SYSTEMTIME stLastModified;
    if (bHaveLastModified && ::FileTimeToSystemTime(&ftLastModified, &stLastModified))
      responseHdr.AddLastModified(stLastModified);
    int nBodyLength = strlen(pszBody);
    responseHdr.AddContentLength(nBodyLength);
    responseHdr.AddContentType(_T("text/html"));

    //Send the header
    responseHdr.Send(m_Socket);

    //Send the body
    m_Socket.Send(pszBody, nBodyLength);
  }
  else
    ReturnErrorMessage(500); //Internal server error
}

void CHttpClient::TransmitFile(const CString& sFile)
{
  CHttpResponseHeader responseHdr;
  HANDLE hFile = ::CreateFile(sFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
  if (hFile != INVALID_HANDLE_VALUE)
  {
    //Get the last modified time for the file / directory
    FILETIME ftFile;
    FILETIME ftRequest;
    SYSTEMTIME stFile;
    BOOL bHaveFileTime = ::GetFileTime(hFile, NULL, NULL, &ftFile) && 
                         ::FileTimeToSystemTime(&ftFile, &stFile);

    //Get the current system time in UTC
    SYSTEMTIME stCurTime;
    FILETIME ftCurTime;
    ::GetSystemTime(&stCurTime);
    ::SystemTimeToFileTime(&stCurTime, &ftCurTime);

    //Ensure that the file time is not past the server time
    if (CompareFileTime(&ftFile, &ftCurTime) == 1)
    {
      CopyMemory(&ftFile, &ftCurTime, sizeof(FILETIME));
      CopyMemory(&stFile, &stCurTime, sizeof(SYSTEMTIME));
    }

    //Handle conditional GET of the file
    if (m_Request.m_Verb == CHttpRequest::HTTP_VERB_GET && 
        bHaveFileTime && m_Request.m_bIfModifiedSincePresent &&
        ::SystemTimeToFileTime(&m_Request.m_IfModifiedSince, &ftRequest) &&
        ::CompareFileTime(&ftFile, &ftRequest) != 1)
    {
      //Form the header
      responseHdr.AddStatusCode(304); //File Not modified
      responseHdr.AddDate(stCurTime);
      responseHdr.AddServer(m_pServer->m_Settings.m_sServerName);
      responseHdr.AddW3MfcAllowFields();
      responseHdr.AddLastModified(stFile);
      responseHdr.AddContentLength(0);
      responseHdr.AddContentType(_T("text/html"));

      //Send the header
      responseHdr.Send(m_Socket);

      //No body is sent for a 304 status
    }
    else
    {
      //Get the length of the file
      DWORD dwFileLength = GetFileSize(hFile, NULL);

      if (m_Request.m_dwHttpVersion > MAKELONG(9, 0)) //No header sent for Http 0.9
      {
        //Get the extension of the file we are about to return
        //and find the mime type for it
        char sExt[_MAX_EXT];
        _splitpath_s(sFile, NULL,0, NULL,0, NULL,0, sExt,_MAX_EXT);
        CString sMime = m_pServer->m_Mime.GetMimeType(sExt);

        //Form the header of the response
        responseHdr.AddStatusCode(200);
        responseHdr.AddDate(stCurTime);
        responseHdr.AddServer(m_pServer->m_Settings.m_sServerName);
        responseHdr.AddW3MfcAllowFields();
        if (bHaveFileTime)
          responseHdr.AddLastModified(stFile);
        responseHdr.AddContentLength(dwFileLength);
        responseHdr.AddContentType(sMime);

        //Send the header
        responseHdr.Send(m_Socket);
      }

      //Send back the file contents (if not a HEAD request)
      if (m_Request.m_Verb == CHttpRequest::HTTP_VERB_GET)
      {
        char sBuf[4096];
        DWORD dwBytesRead = 0;
        do 
        {
          if (::ReadFile(hFile, sBuf, 4096, &dwBytesRead, NULL) && dwBytesRead)
            m_Socket.Send(sBuf, dwBytesRead);
        } 
        while (dwBytesRead);
      }
    }

    //Don't forget to close the file
    CloseHandle(hFile);
  }
  else
  {
    if (::GetLastError() == ERROR_ACCESS_DENIED && !IsFileDirectory(sFile))
      ReturnBasicUnauthorizedMessage(m_Request.m_sURL);
    else
      ReturnErrorMessage(404); //File not found
  }
}

BOOL CHttpClient::IsFileDirectory(const CString& sFile)
{
  BOOL bDirectory = FALSE;

  WIN32_FIND_DATA wfd;
  HANDLE hFind = FindFirstFile(sFile, &wfd);
  if (hFind != INVALID_HANDLE_VALUE)
  {
    if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      bDirectory = TRUE;
    FindClose(hFind);
  }

  return bDirectory;
}

CHttpDirectory* CHttpClient::GetVirtualDirectory(const CString& sDirectory)
{
  CHttpDirectory* pDirectory = NULL;
  for (int i=0; i<m_pServer->m_Settings.m_Directories.GetSize() && pDirectory == NULL; i++)  
  {
    CHttpDirectory& dir = m_pServer->m_Settings.m_Directories.ElementAt(i);
    if (sDirectory.CompareNoCase(dir.GetAlias()) == 0)
      pDirectory = &dir;
  }

  return pDirectory;  
}

CString CHttpClient::ConvertUnixToWindows(const CString& sURL)
{
  //Convert the requested URL from the unix "/" format to the Dos/Windows "\" equivalent
  CString sWinURL(sURL);
  int nOffset = -1;
  do
  {
    nOffset = sWinURL.Find(_T('/'));
    if (nOffset != -1)
      sWinURL.SetAt(nOffset, _T('\\'));
  }
  while (nOffset != -1);

  return sWinURL;
}

int CHttpClient::HexDigit(char c)
{
  int rVal = -1;

  if (_istdigit(c))
    rVal = c - _T('0');
  else if (c >= _T('A') && c <= _T('F'))
    rVal = 10 + c - _T('A');
  else if (c >= _T('a') && c <= _T('f'))
    rVal = 10 + c - _T('a');

  return rVal;
}

CString CHttpClient::URLDecode(const CString& sURL)
{
  CString sDecodedURL;
  int nLength = sURL.GetLength();
  for (int i=0; i<nLength; i++)
  {
    char c1 = sURL[i];
    if (c1 != _T('%'))
      sDecodedURL += c1;
    else
    {
      if (i < nLength-2)
      {
        int msb = HexDigit(sURL[i+1]);
        int lsb = HexDigit(sURL[i+2]);
        if (msb != -1 && lsb != -1)
        {
          int nChar = (msb << 4) + lsb;
          sDecodedURL += char(nChar);
          i += 2;
        }
        else
          sDecodedURL += c1;
      }
      else
        sDecodedURL += c1;
    }
  }

  return sDecodedURL;
}

BOOL CHttpClient::MapURLToLocalFilename(CString& sLocalFile, BOOL& bDirectory)
{
  //Setup the default return value from this function
  BOOL bSuccess = FALSE;

  //Convert from Unix to Windows format
  CString sClientURL = ConvertUnixToWindows(m_Request.m_sURL);
  ASSERT(sClientURL.Find(_T('/')) == -1);

  //As a security precaution do not allow any URL's which 
  //contains any relative parts in it
  if (sClientURL.Find(_T("..")) == -1)
  {
    //Find the virtual directory for this request
    CHttpDirectory* pDirectory = NULL;
    char sDrive[_MAX_DRIVE];
    char sDir[_MAX_DIR];
    char sFname[_MAX_FNAME];
    char sExt[_MAX_EXT];
    CString sVirtualDir(sClientURL);
    do
    {
      _splitpath_s(sVirtualDir, sDrive,_MAX_DRIVE, sDir,_MAX_DIR, sFname,_MAX_FNAME, sExt,_MAX_EXT);
      if (_tcslen(sDir))
      {
        pDirectory = GetVirtualDirectory(sDir);
        if (pDirectory == NULL)
        {
          sVirtualDir = sDir;
          sVirtualDir = sVirtualDir.Left(sVirtualDir.GetLength()-1);
        }
      }
    }
    while (pDirectory == NULL && _tcslen(sDir));

    if (pDirectory)
    {
      ASSERT(pDirectory->GetDirectory().GetLength());
      ASSERT(pDirectory->GetAlias().GetLength());
      ASSERT(pDirectory->GetDefaultFile().GetLength());

      //Ignore the alias part of the URL now that we have got the virtual directory
      CString sAlias = pDirectory->GetAlias();
      CString sRelativeFile(sClientURL);
      sRelativeFile = sRelativeFile.Right(sRelativeFile.GetLength() - sAlias.GetLength());

      //Form the local filename from the requested URL
      sLocalFile = pDirectory->GetDirectory(); 
      sLocalFile += _T("\\");

      //Asking for the default filename
      if (sRelativeFile.IsEmpty())
      {
        bDirectory = pDirectory->GetDirectoryListing();
        if (!bDirectory)
          sLocalFile += pDirectory->GetDefaultFile(); 
      }
      else
      {
        //Ensure that we don't have two "\" separating the filename from the directory
        if (sClientURL.Find(_T('\\')) == 0)
          sLocalFile += sRelativeFile.Right(sRelativeFile.GetLength());
        else
          sLocalFile += sRelativeFile; 

        bDirectory = pDirectory->GetDirectoryListing() && IsFileDirectory(sLocalFile);
        if (bDirectory)
        {
          if (m_Request.m_sURL.Right(0) != _T('/'))
            m_Request.m_sURL += _T('/');
        }
      }

      bSuccess = TRUE;
    }
  }

  return bSuccess;
}
