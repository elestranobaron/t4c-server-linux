//--FILE DEFINITION--------------------------------------------------------
//
/*! \file
   File name:  NMSocket.h
*/
//-------------------------------------------------------------------------
#pragma once



#pragma comment (lib,"Ws2_32.lib")
#include <winsock2.h>
#include <ostream>

typedef int (*pfNMSocketRecvData)(sockaddr_in sockAddrInIP,sockaddr_in sockAddr,unsigned char* pData, size_t DataLenght, int iQueueNbrItems, int iNbrPacketLost, void* pParam);
typedef int (*pfNMSocketClosed)(void* pParam);
typedef int (*pfNMSocketClosedInternal)(void* pThis, SOCKET pSocket, void* pParam);

class NMSocket
{
public:
	enum eError
	{
		eNO_ERROR = 0,
		eERROR_INVALID_PARAMETER,
		eERROR_INITIALIZATION,
	};

	NMSocket();
	~NMSocket();

	int Init(const SOCKET& rSocketHandle);
	int DeInit();

	SOCKET m_hSocket;

 	WSAOVERLAPPED m_OverlappedRecv;
 	WSAOVERLAPPED m_OverlappedSend;


private:
	NMSocket(const NMSocket& rOther);
	NMSocket& operator=(const NMSocket& rOther);
	
protected:

};

