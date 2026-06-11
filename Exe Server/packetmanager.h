#if !defined(AFX_PACKETMANAGER_H__17AFD9C8_48B7_11D2_83F1_00E02922FA40__INCLUDED_)
#define AFX_PACKETMANAGER_H__17AFD9C8_48B7_11D2_83F1_00E02922FA40__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "TFCPacket.h"
#include "SharedStructures.h"
#include "Players.h"
#include "SendPacketVisitor.h"
#ifdef _WIN32
#include "WebServer\W3Mfc.h"
#include "WebServer\HttpClient.h"
#endif

#define COMM_INTR_PROTOTYPE void *pThis,sockaddr_in sockAddrO,sockaddr_in sockAddrI, LPBYTE lpbBuffer, int nBufferSize, int iReserved
class NMPacketManager;

class CPacketManager
{
public:
	static void Create();
	static void Destroy();
	static void StopComm( void );
	static void PacketInterpret( COMM_INTR_PROTOTYPE );
	static void SendPacket( TFCPacket &pPacket, sockaddr_in sockAddrO, sockaddr_in sockAddrI, int nBroadcastRange, WorldPos wlPos, BOOL boBroadcastSend, SendPacketVisitor *visit = NULL, bool inGame = true , bool bUseLevelRange=false, int iLevelMin = 0, int iLevelMax = 0);
	static NMPacketManager *GetCommCenter( void );
	static bool ValidPacket(LPBYTE lpbBuffer, int nBufferSize,sockaddr_in sockAddr);
#ifdef _WIN32
	static void StartWebServer();
	static void StopWebServer();
#endif

protected:

private:
	static NMPacketManager *lpComm;
#ifdef _WIN32
	static CHttpServer m_WebServer;
#endif
	static char m_strLogFileName[1024];

};

#endif // !defined(AFX_PACKETMANAGER_H__17AFD9C8_48B7_11D2_83F1_00E02922FA40__INCLUDED_)
