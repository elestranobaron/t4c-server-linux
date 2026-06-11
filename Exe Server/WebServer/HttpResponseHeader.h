/*
Module : HTTPRESPONSEHEADER.H
Purpose: Defines the interface for a class to simplify sending Http response headers
Created: PJN / 22-04-1999
History: None

Copyright (c) 1999 by PJ Naughter.  
All rights reserved.

*/


/////////////////////////////// Defines ///////////////////////////////////////

#ifndef __HTTPRESPONSEHEADER_H__
#define __HTTPRESPONSEHEADER_H__



/////////////////////////////// Includes //////////////////////////////////////

#include "HttpSocket.h"



/////////////////////////////// Classes ///////////////////////////////////////

//Class which is used to simplify forming and returning a header to the client
class CHttpResponseHeader
{
public:
//Methods
  void    AddStatusCode(int nStatusCode);
  void    AddContentLength(int nSize);
  void    AddContentType(const CString& sMediaType);
  void    AddDate(const SYSTEMTIME& st);
  void    AddLastModified(const SYSTEMTIME& st);
  void    AddWWWBasicAuthenticate(const CString& sRealm);
  void    AddExpires(const SYSTEMTIME& st);
  void    AddLocation(const CString& sLocation);
  void    AddServer(const CString& sServer);
  void    AddW3MfcAllowFields();
  CString DateToStr(const SYSTEMTIME& st);
  BOOL    Send(CHttpSocket& socket);

protected:
  CString m_sHeader;
};

#endif //__HTTPRESPONSEHEADER_H__

