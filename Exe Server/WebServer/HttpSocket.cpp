/*
Module : HTTPSOCKET.CPP
Purpose: Implementation for a simple MFC socket wrapper class
Created: PJN / 22-04-1999
History: None                    

Copyright (c) 1999 by PJ Naughter.  
All rights reserved.

*/

//////////////// Includes ////////////////////////////////////////////

#include "stdafx.h"
#include "../resource.h"
#include "HttpSocket.h"



//////////////// Macros //////////////////////////////////////////////

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



//////////////// Implementation //////////////////////////////////////

CHttpSocket::CHttpSocket()
{
  m_hSocket = INVALID_SOCKET; //default to an invalid socket descriptor
}

CHttpSocket::~CHttpSocket()
{
  Close();
}

BOOL CHttpSocket::Create()
{
  m_hSocket = ::socket(AF_INET, SOCK_STREAM, 0);
  return (m_hSocket != INVALID_SOCKET);
}

BOOL CHttpSocket::Bind(const sockaddr_in* lpSocketAddress)
{
  ASSERT(m_hSocket != INVALID_SOCKET);

  return (::bind(m_hSocket, (const sockaddr*) lpSocketAddress, sizeof(sockaddr_in)) != SOCKET_ERROR);
}

BOOL CHttpSocket::Accept(SOCKET& clientSocket, sockaddr_in& clientAddress)
{
  ASSERT(m_hSocket != INVALID_SOCKET);
  
  int nSize = sizeof(sockaddr_in);
  clientSocket = ::accept(m_hSocket, (sockaddr*) &clientAddress, &nSize);
 
  return (clientSocket != INVALID_SOCKET);
}

BOOL CHttpSocket::Listen()
{
  ASSERT(m_hSocket != INVALID_SOCKET);

  return (::listen(m_hSocket, SOMAXCONN) != SOCKET_ERROR);
}

BOOL CHttpSocket::Send(LPCSTR pszBuf, int nBuf)
{
  ASSERT(m_hSocket != INVALID_SOCKET);

  return (::send(m_hSocket, pszBuf, nBuf, 0) != SOCKET_ERROR);
}

int CHttpSocket::Receive(LPSTR pszBuf, int nBuf)
{
  ASSERT(m_hSocket != INVALID_SOCKET);

  return ::recv(m_hSocket, pszBuf, nBuf, 0); 
}

void CHttpSocket::Close()
{
	if (m_hSocket != INVALID_SOCKET)
	{
		VERIFY(SOCKET_ERROR != ::closesocket(m_hSocket));
		m_hSocket = INVALID_SOCKET;
	}
}

BOOL CHttpSocket::IsReadible(BOOL& bReadible, DWORD dwTimeout)
{
  ASSERT(m_hSocket != INVALID_SOCKET);

  timeval timeout;
  timeout.tv_sec = dwTimeout / 1000;
  timeout.tv_usec = dwTimeout % 1000;
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(m_hSocket, &fds);
  int nStatus = ::select(0, &fds, NULL, NULL, &timeout);
  if (nStatus == SOCKET_ERROR)
    return FALSE;
  else
  {
    bReadible = !(nStatus == 0);
    return TRUE;
  }
}

void CHttpSocket::Attach(SOCKET s)
{
  ASSERT(s != INVALID_SOCKET);
  m_hSocket = s;
}

SOCKET CHttpSocket::Detach()
{
  SOCKET s = m_hSocket;
  m_hSocket = INVALID_SOCKET;
  return s;
}

BOOL CHttpSocket::ReadResponse(LPSTR pszBuffer, int nInitialBufSize, LPSTR pszTerminator, LPSTR* ppszOverFlowBuffer, DWORD dwTimeout, int nGrowBy)
{
  ASSERT(ppszOverFlowBuffer);          //Must have a valid string pointer
  ASSERT(*ppszOverFlowBuffer == NULL); //Initially it must point to a NULL string

  //must have been created first
  ASSERT(m_hSocket != INVALID_SOCKET);

  //The local variables which will receive the data
  LPSTR pszRecvBuffer = pszBuffer;
  int nBufSize = nInitialBufSize;
  
  //retrieve the reponse using until we
	//get the terminator or a timeout occurs
	BOOL bFoundTerminator = FALSE;
	int nReceived = 0;
	DWORD dwStartTicks = ::GetTickCount();
	while (!bFoundTerminator)
	{
		//Has the timeout occured
		if ((::GetTickCount() - dwStartTicks) >	dwTimeout)
    {
		  pszBuffer[nReceived] = '\0';
      TRACE(_T("Timed out waiting for response from socket, Response:%s\n"), pszBuffer);
			return FALSE;
    }

    //check the socket for readability
    BOOL bReadible;
    if (!IsReadible(bReadible))
    {
	    pszBuffer[nReceived] = '\0';
      TRACE(_T("An error occurred checking the readibility of the socket, Response:%s\n"), pszBuffer);
      return FALSE;
    }
    else if (!bReadible) //no data to receive, just loop around
    {
      Sleep(250); //Sleep for a while before we loop around again
      continue;
    }

		//receive the data from the socket
    int nBufRemaining = nBufSize-nReceived-1; //Allows allow one space for the NULL terminator
    if (nBufRemaining<0)
      nBufRemaining = 0;
	  int nData = Receive(pszRecvBuffer+nReceived, nBufRemaining);

    //Reset the idle timeout if data was received
    if (nData)
    {
			dwStartTicks = ::GetTickCount();

      //Increment the count of data received
		  nReceived += nData;							   
    }

    //If an error occurred receiving the data
		if (nData == SOCKET_ERROR)
		{
      //NULL terminate the data received
      if (pszRecvBuffer)
		    pszBuffer[nReceived] = '\0';

      TRACE(_T("An error occured reading from the socket, Response:%s\n"), pszBuffer);
		  return FALSE; 
		}
		else
		{
      //NULL terminate the data received
      if (pszRecvBuffer)
		    pszRecvBuffer[nReceived] = '\0';

      if (nBufRemaining-nData == 0) //No space left in the current buffer
      {
        //Allocate the new receive buffer
        nBufSize += nGrowBy; //Grow the buffer by the specified amount
        LPSTR pszNewBuf = new char[nBufSize];

        //copy the old contents over to the new buffer and assign 
        //the new buffer to the local variable used for retreiving 
        //from the socket
        if (pszRecvBuffer)
          strcpy_s(pszNewBuf,nInitialBufSize, pszRecvBuffer);
        pszRecvBuffer = pszNewBuf;

        //delete the old buffer if it was allocated
        if (*ppszOverFlowBuffer)
          delete [] *ppszOverFlowBuffer;
        
        //Remember the overflow buffer for the next time around
        *ppszOverFlowBuffer = pszNewBuf;        
      }
		}

    //Check to see if the terminator character(s) have been found
		bFoundTerminator = (strstr(pszRecvBuffer, pszTerminator) != NULL);
	}

	//Remove the terminator from the response data
  pszRecvBuffer[nReceived - strlen(pszTerminator)] = '\0';

  return TRUE;
}

