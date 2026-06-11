/*
Module : HTTPRESPONSEHEADER.CPP
Purpose: Implementation for a class to simplify sending Http response headers
Created: PJN / 22-04-1999
History: None                    

Copyright (c) 1999 by PJ Naughter.  
All rights reserved.

*/

//////////////// Includes ////////////////////////////////////////////

#include "stdafx.h"
#include <afxpriv.h>
#include "HttpResponseHeader.h"




//////////////// Macros //////////////////////////////////////////////

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



//////////////// Implementation //////////////////////////////////////

void CHttpResponseHeader::AddStatusCode(int nStatusCode)
{
  CString sLine;
  switch (nStatusCode)
  {
    case 200: sLine = _T("HTTP/1.0 200 OK\r\n");                    break;
    case 201: sLine = _T("HTTP/1.0 201 Created\r\n");               break;
    case 202: sLine = _T("HTTP/1.0 202 Accepted\r\n");              break;
    case 204: sLine = _T("HTTP/1.0 204 No Content\r\n");            break;
    case 300: sLine = _T("HTTP/1.0 300 Multiple Choices\r\n");      break;
    case 301: sLine = _T("HTTP/1.0 301 Moved Permanently\r\n");     break;
    case 302: sLine = _T("HTTP/1.0 302 Moved Temporarily\r\n");     break;
    case 304: sLine = _T("HTTP/1.0 304 Not Modified\r\n");          break;
    case 400: sLine = _T("HTTP/1.0 400 Bad Request\r\n");           break;
    case 401: sLine = _T("HTTP/1.0 401 Unauthorized\r\n");          break;
    case 403: sLine = _T("HTTP/1.0 403 Forbidden\r\n");             break;
    case 404: sLine = _T("HTTP/1.0 404 Not Found\r\n");             break;
    case 500: sLine = _T("HTTP/1.0 500 Internal Server Error\r\n"); break;
    case 501: sLine = _T("HTTP/1.0 501 Not Implemented\r\n");       break;
    case 502: sLine = _T("HTTP/1.0 502 Bad Gateway\r\n");           break;
    case 503: sLine = _T("HTTP/1.0 503 Service Unavailable\r\n");   break;
    default: sLine.Format(_T("HTTP/1.0 %d\r\n"), nStatusCode);      break;
  }
  m_sHeader += sLine;
}

void CHttpResponseHeader::AddContentLength(int nSize)
{
  CString sLine;
  sLine.Format(_T("Content-Length: %d\r\n"), nSize);
  m_sHeader += sLine;
}

void CHttpResponseHeader::AddContentType(const CString& sMediaType)
{
  CString sLine;
  sLine.Format(_T("Content-Type: %s\r\n"), sMediaType);
  m_sHeader += sLine;
}

CString CHttpResponseHeader::DateToStr(const SYSTEMTIME& st)
{
  static char* sMonth[] =  
  {
    _T("Jan"),
    _T("Feb"),
    _T("Mar"), 
    _T("Apr"),
    _T("May"),
    _T("Jun"),
    _T("Jul"),
    _T("Aug"),
    _T("Sep"),
    _T("Oct"),
    _T("Nov"),
    _T("Dec"),
  };

  static char* sDay[] =
  {
    _T("Sun"),
    _T("Mon"),
    _T("Tue"), 
    _T("Wed"),
    _T("Thu"),
    _T("Fri"),
    _T("Sat"),
  };

  CString sDate;
  sDate.Format(_T("%s, %02d %s %04d %02d:%02d:%02d GMT"), sDay[st.wDayOfWeek], 
               st.wDay, sMonth[st.wMonth], st.wYear, st.wHour, st.wMinute, st.wSecond);
  return sDate;
}

void CHttpResponseHeader::AddW3MfcAllowFields()
{
  CString sLine = _T("Allow: GET, HEAD, DELETE\r\n");
  m_sHeader += sLine;
}

void CHttpResponseHeader::AddDate(const SYSTEMTIME& st)
{
  CString sDate = DateToStr(st);
  CString sLine;
  sLine.Format(_T("Date: %s\r\n"), sDate);
  m_sHeader += sLine;
}

void CHttpResponseHeader::AddLastModified(const SYSTEMTIME& st)
{
  CString sDate = DateToStr(st);
  CString sLine;
  sLine.Format(_T("Last-Modified: %s\r\n"), sDate);
  m_sHeader += sLine;
}

void CHttpResponseHeader::AddExpires(const SYSTEMTIME& st)
{
  CString sDate = DateToStr(st);
  CString sLine;
  sLine.Format(_T("Expires: %s\r\n"), sDate);
  m_sHeader += sLine;
}

void CHttpResponseHeader::AddWWWBasicAuthenticate(const CString& sRealm)
{
  CString sLine;
  sLine.Format(_T("WWW-Authenticate: Basic realm=%s\r\n"), sRealm);
  m_sHeader += sLine;
}

void CHttpResponseHeader::AddLocation(const CString& sLocation)
{
  CString sLine;
  sLine.Format(_T("Location: %s\r\n"), sLocation);
  m_sHeader += sLine;
}

void CHttpResponseHeader::AddServer(const CString& sServer)
{
  CString sLine;
  sLine.Format(_T("Server: %s\r\n"), sServer);
  m_sHeader += sLine;    
}

BOOL CHttpResponseHeader::Send(CHttpSocket& socket)
{
	//For correct operation of the T2A macro, see MFC Tech Note 59
	USES_CONVERSION;

  //Validate what we are about to send
  ASSERT(m_sHeader.GetLength());

  //Add the "\r\n" separator onto the header
  m_sHeader += _T("\r\n");

  //Convert to Ascii
  char* pszHeader = T2A((LPTSTR) (LPCTSTR) m_sHeader);
  int nLength = strlen(pszHeader);

  //Send it down the socket  
  return socket.Send(pszHeader, nLength);
}

