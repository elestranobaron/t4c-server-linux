/*
Module : HTTPSOCKET.H
Purpose: Defines the interface for a simple MFC socket wrapper class
Created: PJN / 22-04-1999
History: None

Copyright (c) 1999 by PJ Naughter.  
All rights reserved.

*/


/////////////////////////////// Defines ///////////////////////////////////////

#ifndef __HTTPSOCKET_H__
#define __HTTPSOCKET_H__

#ifndef _WINSOCKAPI_
#pragma message("W3MFC classes require afxsock.h or winsock.h in your PCH")
#endif


/////////////////////////////// Classes ///////////////////////////////////////

class CHttpSocket
{
public:
//Constructors / Destructors
  CHttpSocket();
  ~CHttpSocket();

//methods
  BOOL   Create();
  BOOL   Bind(const sockaddr_in* lpSocketAddress);
  BOOL   Accept(CHttpSocket& clientSocket, sockaddr_in& clientAddress);
  BOOL   Accept(SOCKET& clientSocket, sockaddr_in& clientAddress);
  BOOL   Listen();
  BOOL   Send(LPCSTR pszBuf, int nBuf);
  void   Close();
  int    Receive(LPSTR pszBuf, int nBuf);
  BOOL   ReadResponse(LPSTR pszBuffer, int nInitialBufSize, LPSTR pszTerminator, LPSTR* ppszOverFlowBuffer, DWORD dwTimeout, int nGrowBy=4096);
  BOOL   IsReadible(BOOL& bReadible, DWORD dwTimeout = 0);
  void   Attach(SOCKET s);
  SOCKET Detach();

protected:
  SOCKET m_hSocket;
};

#endif //__HTTPSOCKET_H__

