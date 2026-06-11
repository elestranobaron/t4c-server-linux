/*
Module : HTTPCLIENT.H
Purpose: Defines the interface for the CHttpRequest & CHttpClient classes
Created: PJN / 22-04-1999
History: None

Copyright (c) 1999 by PJ Naughter.  
All rights reserved.

*/


/////////////////////////////// Defines ///////////////////////////////////////

#ifndef __HTTPCLIENT_H__
#define __HTTPCLIENT_H__



/////////////////////////////// Includes //////////////////////////////////////

#include "HttpSocket.h"
#include "W3Mfc.h"



/////////////////////////////// Classes ///////////////////////////////////////

//Class which represents a request from a HTTP client
class CHttpRequest
{
public:
//Constructors / Destructors
  CHttpRequest();

//Enums for m_Verb
	enum HttpVerb
  {
		HTTP_VERB_POST      = 0,
		HTTP_VERB_GET       = 1,
		HTTP_VERB_HEAD      = 2,
		HTTP_VERB_PUT       = 3,
		HTTP_VERB_LINK      = 4,
		HTTP_VERB_DELETE    = 5,
		HTTP_VERB_UNLINK    = 6,
		HTTP_VERB_UNKNOWN   = 7,
	};

//Enums for Authorization type
  enum HttpAuthorization
  {
    HTTP_AUTHORIZATION_ANONYMOUS = 0,
    HTTP_AUTHORIZATION_PLAINTEXT = 1,
  };  

//Methods
  CHttpRequest& operator=(const CHttpRequest& request);

//Member variables
  sockaddr_in       m_ClientAddress;           //The IP address where the request originated from
  HttpVerb          m_Verb;                    //GET, PUT etc
  CString           m_sURL;                    //The URL of the request
  CString           m_sExtra;                  //Any part of the URL after the "?"
  DWORD             m_dwHttpVersion;           //The HTTP Version Number of the HTTP client request
                                               //encoded as MAKELONG(Minor, Major)
  BOOL              m_bIfModifiedSincePresent; //Is the If-Modified-Since header present
  SYSTEMTIME        m_IfModifiedSince;         //The actual If-Modified-Since header
  HttpAuthorization m_AuthorizationType;       //What authorization method is being used
  CString           m_sUsername;               //username if plaintext authorization is being used
  CString           m_sPassword;               //password if plaintext authorization is being used
};



//forward declaration
class CHttpServer;



//Class which handles the HTTP client connection
class CHttpClient : public CObject
{
public:
//Constructors / Destructors
  CHttpClient();
  ~CHttpClient();

//Methods
  void SetServer(CHttpServer* pServer);
  void HandleClient(SOCKET hSocket, sockaddr_in ClientAddress);

protected:
//Methods
  BOOL ParseRequest(LPSTR pszRequest);
  BOOL ParseSimpleRequestLine(LPSTR pszLine);
  virtual void ReturnErrorMessage(int nStatusCode);
  virtual void ReturnRedirectMessage(const CString& sURL);
  virtual void ReturnFileDeletedOkMessage(const CString& sFile);
  virtual void ReturnBasicUnauthorizedMessage(const CString& sRealm);
  virtual void TransmitFile(const CString& sFile);
  virtual void TransmitDirectory(const CString& sDirectory);
  virtual BOOL MapURLToLocalFilename(CString& sLocalFile, BOOL& bDirectory);
  virtual BOOL AllowThisConnection();
  virtual CHttpDirectory* GetVirtualDirectory(const CString& sDirectory);

//Statics
  static int HexDigit(char c);
  static CString ConvertUnixToWindows(const CString& sURL);
  static CString URLDecode(const CString& sURL);
  static BOOL LoadHTMLResource(UINT nID, char*& pszHTML, DWORD& dwSize);
  static BOOL ParseDate(const CString& sField, SYSTEMTIME& time);
  static BOOL SplitRequestLine(LPSTR pszLine, CString& sField, CString& sValue);
  static BOOL ParseWeekDay(char* pszToken, int& nWeekDay);
  static BOOL ParseMonth(char* pszToken, int& nMonth);
  static BOOL ParseAuthorizationBasic(const CString& sField, CString& sUsername, CString& sPassword);
  static BOOL IsFileDirectory(const CString& sFile);

//Member variables
  CHttpServer* m_pServer;
  CHttpSocket  m_Socket;
  CHttpRequest m_Request;

  DECLARE_DYNCREATE(CHttpClient)
};

#endif //__HTTPCLIENT_H__

