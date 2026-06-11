//--FILE DEFINITION--------------------------------------------------------
//
/*! \file
   File name:  PacketManager.h
*/
//-------------------------------------------------------------------------
#pragma once
#pragma warning( disable:4786 )

#include "UDPClient.h"
#ifdef _WIN32
#include "../Zlib/zlib.h"
#else
#include <zlib.h>
#endif

//#define ENABLE_TRACELOG_FILE1


using namespace std;

#define COMM_INTR_PROTOTYPE void *pThis,sockaddr_in sockAddrO,sockaddr_in sockAddrI, LPBYTE lpbBuffer, int nBufferSize, int iReserved

typedef void( *COMM_INTR_CALLBACK )( COMM_INTR_PROTOTYPE );


#define PORT_SERVER                  11677

#define WAIT_QUEUE_MAX                1000

#define PACKET_ID_MAX                 4095
#define NBR_RECEIVED_REGISTRED_ID      256 //512 //2048
#define NBR_PACKET_PENDING_MAX          16 //128 //2048
#define NBR_PACKET_SPLIT_MAX            16 //32 //64
#define PACKET_BACKLOG_TIMEOUT       15000
#define PACKET_SPLIT_TIMEOUT         10000

#define PACKET_DATA_MAX              (PACKET_MAX_SIZE - 16)  //16 is total of all header structure
#define PACKET_DATA_MAX_SPLIT_PART      32 //64


typedef struct _PacketHeader
{
   unsigned short  ushID      : 12,     // Packet ID            //max 0x0FFF  (4095)
                   ushNeedAck : 1,      // Packet need ack
                   ushCompress: 1,      // Packet compressed
                   ushSplit   : 1,      // Packet is splitted
				   ushReserved: 1;
   unsigned char   uchCRC8;
   unsigned char   uchPartNumber;
} PacketHeader;

/* Structures ecrites telles quelles sur le fil (memcpy dans pBuffer). `unsigned long`
 * fait 8 octets en LP64 : les entetes passaient de 16 a 28 octets, et les fragments
 * (pieceSize 1008 + 28 = 1036 > PACKET_MAX_SIZE 1024) etaient silencieusement jetes
 * a la reception — d'ou la perte des gros paquets (inview > ~1 Ko compresse).
 * On force des champs 32 bits = format Win32 d'origine (total entetes 16 octets). */
typedef struct _PacketHeaderSplit
{
   unsigned int  uchSplitNO      : 6,
                 uchPartNbr      : 5,
                 uchMaxPart      : 5,
                 ushDataSize     : 16;
} PacketHeaderSplit;

typedef struct _PacketHeaderComp
{
   unsigned int  ulUnpackData;
   unsigned int  ulPackData;
} PacketHeaderComp;

typedef struct _UDPPacket
{
   sockaddr_in          sockAddrO; //adresse du player pour tirer les packet chez eux
   sockaddr_in          sockAddrI; //adresse de la carte binder pour tirer sur la bonne interface reseau

   BOOL                 boDelete;
   BOOL                 boAddPending;
   BOOL                 boAckReceived;
   unsigned char        *pBuffer; 
   unsigned long        ulBufferLength;
   unsigned long        uldwTimeout;
   unsigned long        uldwTimeoutMegaT;
   unsigned long        uldwTimeoutMegaE;
   unsigned long        ulAckDelay;
   unsigned long        ulNbrAck;

   PacketHeader         *pHeader;
   PacketHeaderSplit    *pHeaderSplit;
   PacketHeaderComp     *pHeaderComp;
   unsigned char        *puchData;
   unsigned long        ulDataLength;
} UDPPacket;

typedef struct _UDPPacketSplit
{
   vector<unsigned char> lPartReceive;   // List of part already received
   UDPPacket *pPak;
} UDPPacketSplit;

typedef void (*pfProcessCallback)(void *pThis,sockaddr_in sockAddrO,sockaddr_in sockAddrI,unsigned char* pData, int iDataLenght, int iReserved);

class CUDPConnection;
class NMPacketManager
{
public:

	NMPacketManager();
	~NMPacketManager();

   bool Init(pfProcessCallback pfProcessCallback,void *pThis,bool bStartAsServer,int iPort1,const char *pstrIP1,int iPort2,const char *pstrIP2);

     
   void       SendUDPPacket( sockaddr_in sockAddrO,sockaddr_in sockAddrI, unsigned char* pBuffer, int nBufferSize, DWORD dwAckDelay, 
                             DWORD dwMaxAck,bool bCompress,bool bAutoCompress =false,bool bSendDirect = false);

   UDPPacket *PacketAlloc(int size);
   void       PacketFree(UDPPacket *packet);
   void       PacketAllocAndCopy( UDPPacket *pNPacket,UDPPacket *packet);
   inline void PostReceivePacket   (UDPPacket* pPacket);
   inline void PostSendPacket      (UDPPacket* pPacket);
   inline void PostSendPacketMega  (UDPPacket* pPacket);
   inline void PostLostConnection  (CUDPConnection* pConnection); // 

   int  GetIntrQueueSize();
   unsigned char   CalcChecksumComp2(unsigned char	*pData, unsigned long dwNbrData);

   vector< sockaddr_in > *GetLostConnections( void );
   void FreeLostConnections(BOOL boFlushList);
   
   
   int  GetNbrConnection();
   void ForceStopServer(){m_bServerStarted = FALSE;}
   BOOL GetServerStarted(){return m_bServerStarted;}

  
protected:
   

private:
   inline CUDPConnection* GetConnection( sockaddr_in sockAddr ,bool bCreate);

   void  SendAckPacket(UDPPacket* pPacket) ;
   void  AnalyzePacket(UDPPacket *packet);
   bool  IsChksumOK(unsigned char *pData, unsigned long dwNbrData,unsigned char uchCheckSum);
   
   
   

   static int  ReceiveUDPCallback (sockaddr_in sockAddrInIP,sockaddr_in sockAddr,unsigned char* pData, size_t DataLenght, int iQueueNbrItems, int iNbrPacketLost, void* pParam);
   
   
   static void UDPPreSendPacketFct(LPVOID lpParam);
   
   static void UDPSplitListFct( LPVOID lpParam );
   
   static UINT CALLBACK UDPLostConnectionThread ( LPVOID lpParam ); 
   static UINT CALLBACK UDPSendPacketFct(LPVOID lpParam);
   static UINT CALLBACK UDPProcessPacketFct(LPVOID lpParam);
   static UINT CALLBACK UDPMaintenancePacket( LPVOID lpParam );
   static UINT CALLBACK OneShotEnableCommThread ( LPVOID lpParam ); 



public:
   BOOL  m_bServerStarted;
   BOOL  m_bCanStartComm;
   DWORD m_dwGlobalNbrRetry;
   DWORD m_dwGlobalNbrLost;
   DWORD m_dwNbrPacketSend;
   DWORD m_dwNbrPacketRecv;
   DWORD m_dwNbrPacketRecvAlreadyRegistred;

   DWORD *m_pdwNbrPacketSendbyPacket;
   DWORD *m_pdwNbrPacketRecvbyPacket;
protected:
	char m_strLogFileName[1024];

   

private:
   //typedef map< sockaddr_in, CUDPConnection* > UDPConnectionMap;
   //UDPConnectionMap m_UDPConnections;

   typedef struct _sConnectionList
   {
      sockaddr_in sockIN;
      CUDPConnection*pConn;
   }sConnectionList;

   vector< sConnectionList > m_UDPConnectionsList;
   

   CUDPConnection  *m_pUDPClientConnections;

   void *m_pParent;
   UDPClient *m_pUDP;
   bool m_bInit;

   pfProcessCallback m_pfProcessCallback;

   BOOL      m_bUDPMaintenancePacket;
   HANDLE    m_hUDPMaintenancePacket;

   //revived packet
   BOOL      m_bUDPProcessPacketThread;
   HANDLE    m_hUDPProcessPacketThread;
   HANDLE    m_hUDPReceivePacketIO;
   //Sending packet
   BOOL      m_bUDPSendPacketThread;
   HANDLE    m_hUDPSendPacketThread;
   HANDLE    m_hUDPSendPacketIO;
   //Free Lost Connection
   BOOL      m_bLostConnThread;
   HANDLE    m_hLostConnThread;
   HANDLE    m_hLostConnIO;



   int m_uiNbrInterpQueueCnt;
   CRITICAL_SECTION m_crConnectionLock;
   CRITICAL_SECTION m_crInterLockCnt;
};


class CUDPConnection
{
public:

   CUDPConnection();
   ~CUDPConnection();


   int InitializeComm(const sockaddr_in &sAddr, NMPacketManager *pNMPacketManager);
   void ResetTimeout();
   void DestroyAllPending();
   bool DestroyPending (unsigned short packetID);
   inline WORD GetPacketID();
   inline WORD GetPacketSplitID();
   inline void RegisterReceivedPacketID(unsigned short usgPacketID);
   inline bool AlreadyReceivedPacket(unsigned short ushPacketID);


   inline bool AddPending(UDPPacket* pPending); // Add this packet to the list of 'waiting for ack' packets
   inline bool AddPacketFragment(UDPPacket* pFragment);
   inline void VerifyTimedoutPending();
   inline void VerifyTimedoutFragments();
   inline bool ConnectionHasTimedout() ;

   //mega Packet
   inline bool AddPacketToMegaPack(UDPPacket* pPack);
   inline void VerifyMegaPack();

  
     
   
public:
   NMPacketManager	*m_pNMPacketManager;  
   sockaddr_in    m_sockAddr;        

   vector<UDPPacket*> m_pendingPackets;   // List of packets that did not receive an ACK yet.
   vector<UDPPacketSplit>  m_FragmentsPackets; // List of fragmented packets.

protected:
   UDPPacket *m_pMegaPack;


protected:

   CRITICAL_SECTION m_crPendingLock;
   CRITICAL_SECTION m_crSplitLock;
   CRITICAL_SECTION m_crMegaPackLock;
   unsigned long	 m_dwConnectionTimeout;
   unsigned short  m_ushCurrentID;
   unsigned short  m_ushCurrentSplitID;
   unsigned short  m_ushOffsetID;
   long            m_lReceivedPacketID[NBR_RECEIVED_REGISTRED_ID];


};

