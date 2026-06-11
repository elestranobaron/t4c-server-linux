#pragma once

#include "Socket.h"
#include <map>
#include <vector>

using namespace std;

#define PACKET_MAX_SIZE                    1024//2048

typedef struct _sPacketData
{
   unsigned char *pData;
   DWORD          dwDataLength;
   void          *pParent;
}sPacketData;



class UDPClient
{
public:
   enum eError
   {
      eNO_ERROR = 0,
      eERROR_INITIALIZATION,
      eERROR_INVALID_PARAMETER,
      eERROR_COULD_NOT_SEND,
   };

   UDPClient();
   ~UDPClient();

   int SetReceiveParameters(pfNMSocketRecvData pCallbackRecvData,void* pCallbackParam,size_t ReceiveBufferSize);

   int StartServer(LPCTSTR Address, int Port, bool bBuffered);
   int StartServer2(LPCTSTR Address, int Port, bool bBuffered);
   void Disconnect();

   int SendTo(sockaddr_in sockToO,sockaddr_in sockToI,char* pBuff, size_t BuffLenght);


   vector< sockaddr_in > *GetLostConnections( void );
   void FreeLostConnections(BOOL boFlushList);

private:
   UDPClient(const UDPClient& rOther);
   UDPClient& operator=(const UDPClient& rOther);

   static UINT CALLBACK fReceiveData1(LPVOID pParameter);
   static UINT CALLBACK fReceiveData2(LPVOID pParameter);

   ULONG GetIPAddress( LPCTSTR strHostName );
   bool GetLocalName(LPTSTR strName, UINT nSize);
   

protected:
   // Receive callback and parameters
   pfNMSocketRecvData m_pFcnRecvData;

   void* m_pFcnRecvParam;
   size_t m_ReceiveBufferSize;
   char* m_pSendTmpBuffer;

   BOOL m_bReceiveThread1;
   NMSocket* m_pSocket1;
   sockaddr_in  m_sockAddrLocal1;
   sockaddr_in  m_sockAddrSender1;
   HANDLE m_hReceiveThread1;
   


   BOOL m_bReceiveThread2;
   NMSocket* m_pSocket2;
   sockaddr_in  m_sockAddrLocal2;
   sockaddr_in  m_sockAddrSender2;
   HANDLE m_hReceiveThread2;



   CRITICAL_SECTION crLost;
   vector< sockaddr_in > vConnectionLost;

   CRITICAL_SECTION m_crLockCallbackMultipleIP;


   char m_strLogFileName[1024];

   
};


