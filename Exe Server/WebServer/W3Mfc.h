/*
Module : W3MFC.H
Purpose: Defines the interface for a simple MFC class encapsulation of a HTTP server
Created: PJN / 22-04-1999
History: None

Copyright (c) 1999 by PJ Naughter.  
All rights reserved.

*/


/////////////////////////////// Defines ///////////////////////////////////////

#ifndef __W3MFC_H__
#define __W3MFC_H__



/////////////////////////////// Includes //////////////////////////////////////

#include "HttpMimeManager.h"



/////////////////////////////// Classes ///////////////////////////////////////


//The values relating to a single directory which the web server handles
class CHttpDirectory
{
public:
//Constructors / Destructors
  CHttpDirectory() : m_bDirectoryListing(FALSE) {};
  CHttpDirectory(const CHttpDirectory& dir);

//Methods
  CHttpDirectory& operator=(const CHttpDirectory& dir);

//Accessors / Mutators
  void SetAlias(const CString& sAlias);
  void SetDirectory(const CString& sDirectory);
  void SetDefaultFile(const CString& sDefaultFile);
  void SetDirectoryListing(BOOL bListing) { m_bDirectoryListing = bListing; };
  CString GetAlias() const { return m_sAlias; };
  CString GetDirectory() const { return m_sDirectory; };
  CString GetDefaultFile() const { return m_sDefaultFile; };
  BOOL    GetDirectoryListing() const { return m_bDirectoryListing; };

protected:
//Member variables
  CString m_sAlias;            //The directory which clients see e.g. "/cgi-bin"
  CString m_sDirectory;        //The local directory to map requests to
  CString m_sDefaultFile;      //The file to send when requesting this direcory without a filename
  BOOL    m_bDirectoryListing; //If TRUE then a directory listing will be returned in preference to the default file
};

//The settings which the web server uses in the call to CHttpServer::Start
class CHttpServerSettings
{
public:
//Constructors / Destructors
  CHttpServerSettings();
  CHttpServerSettings(const CHttpServerSettings& settings);

//Methods
  CHttpServerSettings& operator=(const CHttpServerSettings& settings);

//Member variables
  unsigned short                          m_nPort;                    //The port on which to run the web server
  CArray<CHttpDirectory, CHttpDirectory&> m_Directories;              //Directories served up by this server
  BOOL                                    m_bBind;                    //Should the server be bound to an address
  in_addr                                 m_BindAddress;              //The IP address to bind to (if m_bBind is set)
  DWORD                                   m_dwIdleClientTimeout;      //Timeout in ms to wait for client requests
  CString                                 m_sServerName;              //The Web server name to return in HTTP headers
  CRuntimeClass*                          m_pRuntimeClientClass;      //The runtime class of the client class to use, normally is CHttpClient
  CString                                 m_sUsername;                //The account to run the web server under
  CString                                 m_sPassword;                //The account's password
};



//The actual web server
class CHttpServer : CObject
{
public:
//Constructors / Destructors
  CHttpServer();
  ~CHttpServer();

//Methods
  BOOL Start(const CHttpServerSettings& settings);
  BOOL Stop();
  BOOL Wait();
  static BOOL ReverseDNSLookup(in_addr sin_addr, CString& sDomainName);

protected:
  DECLARE_DYNAMIC(CHttpServer)
  static UINT ListenSocketFunction(LPVOID pParam);
  void ListenSocketFunction();

  CWinThread*         m_pListenThread;
  CEvent              m_ListenStartEvent;
  BOOL                m_bRequestListenStop;
  BOOL                m_bListenerRunningOK;
  CHttpServerSettings m_Settings;
  CHttpMimeManager    m_Mime;
};

#endif //__W3MFC_H__

