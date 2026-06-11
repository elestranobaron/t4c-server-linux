#pragma once

#ifdef _WIN32
#pragma comment (lib,"Ws2_32.lib")
#include <winsock2.h>
#else
#include "../PLAYERS.H"
#include <netinet/in.h>
#endif

#include <ostream>

#ifdef _WIN32
#define NM_IPV4_B1(a) ((a).S_un.S_un_b.s_b1)
#define NM_IPV4_B2(a) ((a).S_un.S_un_b.s_b2)
#define NM_IPV4_B3(a) ((a).S_un.S_un_b.s_b3)
#define NM_IPV4_B4(a) ((a).S_un.S_un_b.s_b4)
#define NM_IPV4_ADDR(a) ((a).S_un.S_addr)
#else
#include <arpa/inet.h>
#define NM_IPV4_B1(a) (static_cast<unsigned char>((ntohl((a).s_addr) >> 24) & 0xFF))
#define NM_IPV4_B2(a) (static_cast<unsigned char>((ntohl((a).s_addr) >> 16) & 0xFF))
#define NM_IPV4_B3(a) (static_cast<unsigned char>((ntohl((a).s_addr) >> 8) & 0xFF))
#define NM_IPV4_B4(a) (static_cast<unsigned char>(ntohl((a).s_addr) & 0xFF))
#define NM_IPV4_ADDR(a) ((a).s_addr)
#endif

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

#ifdef _WIN32
	WSAOVERLAPPED m_OverlappedRecv;
	WSAOVERLAPPED m_OverlappedSend;
#endif

private:
	NMSocket(const NMSocket& rOther);
	NMSocket& operator=(const NMSocket& rOther);
};
