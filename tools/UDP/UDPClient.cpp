#include "UDPClient.h"
#include <iostream>
#include <process.h>
#include <tchar.h>
#include <direct.h>


unsigned char COMM_key[]={
      0xC4 ,0x9B ,0x1B ,0x30 ,0xC5 ,0x0D ,0x58 ,0xB7, 0x10 ,0xC6 ,0xFD ,0x78 ,0xBB ,0x11 ,0x89 ,0xF3,
      0xCE ,0x7F ,0x33 ,0x3D ,0xA9 ,0x3F ,0xBA ,0x99, 0x55 ,0x9B ,0xD4 ,0x30 ,0x9F ,0xD8 ,0x55 ,0x54,
      0x4A ,0xED ,0x94 ,0xA3 ,0x33 ,0x09 ,0x4A ,0xE1, 0x08 ,0x00 ,0x01 ,0xA8 ,0x50 ,0x1E ,0xF6 ,0x82,
      0x05 ,0xC5 ,0xD6 ,0x5E ,0xF6 ,0x56 ,0x47 ,0x57, 0xAF ,0x49 ,0x82 ,0x5A ,0x98 ,0xC7 ,0x34 ,0xF1,
      0xE0 ,0x4A ,0xB4 ,0x96 ,0x7A ,0xEF ,0x2D ,0x2C, 0x3D ,0x66 ,0x60 ,0x99 ,0x42 ,0x52 ,0x83 ,0x53,
      0x74 ,0xFF ,0xA8 ,0x17 ,0xCF ,0xC6 ,0x82 ,0x4D, 0x57 ,0x16 ,0xB3 ,0xEB ,0x42 ,0xFD ,0x26 ,0x8B,
      0x05 ,0x64 ,0x2A ,0xC6 ,0xE0 ,0x12 ,0x33 ,0xDC, 0x70 ,0x6D ,0x38 ,0x19 ,0x20 ,0x1D ,0xA9 ,0x1B,
      0x03 ,0x89 ,0xA8 ,0xBA ,0x30 ,0x0D ,0x93 ,0xC9, 0xC5 ,0xCF ,0xF1 ,0xE2 ,0x1A ,0xD7 ,0x1D ,0xB1,
      0x2D ,0x3C ,0xBA ,0x29 ,0x91 ,0x53 ,0x1D ,0x60, 0x54 ,0xD5 ,0x5D ,0x0D ,0xCC ,0x43 ,0xE2 ,0xD9,
      0xF0 ,0xF5 ,0xE4 ,0xBE ,0xB9 ,0xA5 ,0x9C ,0x7B, 0xE2 ,0x4F ,0x56 ,0x85 ,0x2C ,0x63 ,0x03 ,0xAA,
      0x3E ,0xB2 ,0x30 ,0xA4 ,0xE1 ,0x49 ,0x62 ,0x76, 0xB8 ,0x94 ,0x40 ,0xB5 ,0xB3 ,0xBA ,0x12 ,0xC4,
      0x8B ,0xF5 ,0x2A ,0x8F ,0xC6 ,0x07 ,0x44 ,0x32, 0xA5 ,0x7C ,0xFE ,0x91 ,0x5E ,0x53 ,0x24 ,0x45,
      0xD1 ,0xC2 ,0xE4 ,0xB1 ,0x2D ,0x9B ,0x16, 0xF2 ,0xF8 ,0x65 ,0xFB ,0x8B ,0xAE ,0xBB ,0xD2 ,0xD5,
      0x8E ,0xA2 ,0xF4 ,0xC6 ,0x8C ,0x45 ,0x0A, 0x69 ,0x88 ,0x2F ,0x22 ,0x9F ,0xA8 ,0x03 ,0x3A ,0x9C,
      0xC1 ,0xA4 ,0x72 ,0x0A ,0x79 ,0xA3 ,0x7B, 0x3A ,0xAD ,0x40 ,0xE6 ,0x49 ,0xD6 ,0xC0 ,0xFD ,0x47,
      0xF1 ,0x57 ,0xFB ,0x3D ,0xCD ,0x99 ,0x5A, 0x1C ,0x44 ,0x82 ,0x3C ,0x8A ,0x45 ,0x0C ,0x40 ,0x07,
      0x26 ,0xD1 ,0xB1 ,0xA7 ,0xE6 ,0x92 ,0x1D, 0x4D ,0xAF ,0x60 ,0x9D ,0xE7 ,0x86 ,0x82 ,0xAB ,0x93,
      0xEE ,0xAC ,0x39 ,0x0E ,0xA4 ,0x7A ,0xBD, 0x8E ,0xD3 ,0xCD ,0x05 ,0x6A ,0xAE ,0x46 ,0x6D ,0x22,
      0x5A ,0x04 ,0xBC ,0xC2 ,0x6D ,0xC2 ,0xB7, 0x25 ,0x1A ,0x3E ,0xF7 ,0xA0 ,0x56 ,0xFB ,0x37 ,0x72,
      0x00 ,0x7B ,0xE7 ,0x92 ,0x2B ,0x60 ,0x0C, 0xDA ,0x6F ,0xAB ,0x77 ,0x99 ,0x9B ,0xCC ,0x3D ,0xC4,
      0xF7 ,0x36 ,0xEC ,0xD5 ,0x49 ,0xCC ,0x42, 0xFB ,0x45 ,0x92 ,0x0F ,0xEC ,0x20 ,0x65 ,0x38 ,0xDD,
      0xDE ,0x7F ,0x1A ,0x92 ,0x05 ,0x61 ,0x7F, 0x59 ,0x90 ,0xF5 ,0xCB ,0xB1 ,0x08 ,0xF7 ,0x67 ,0x07,
      0xA0 ,0xDB ,0x48 ,0xD5 ,0x8B ,0xF6 ,0x53, 0x49 ,0xCA ,0x58 ,0x3E ,0x85 ,0xFD ,0x38 ,0x8A ,0x0F,
      0x2D ,0xBB ,0x5A ,0xEA ,0x63 ,0x14 ,0x89, 0xA5 ,0xF0 ,0xC4 ,0x7B ,0x8B ,0x2D ,0x60 ,0xE7 ,0x46,
      0xBB ,0x61 ,0x9B ,0x44 ,0x17 ,0x4F ,0xC2, 0xC9 ,0x84 ,0xC8 ,0x1D ,0x66 ,0x48 ,0x2E ,0x46 ,0x81,
      0x03 ,0x93 ,0xDD ,0xD8 ,0xB4 ,0xC0 ,0x22, 0x96 ,0x8A ,0x73 ,0x40 ,0x84 ,0xE2 ,0xF4 ,0x19 ,0x73,
      0x42 ,0x99 ,0x76 ,0x1E ,0xCB ,0x05 ,0x54, 0xE7 ,0x5B ,0x86 ,0xC6 ,0x99 ,0x41 ,0xC4 ,0xEC ,0x47,
      0x3A ,0x41 ,0x3D ,0x15 ,0x70 ,0x3E ,0x83, 0x28 ,0x9A ,0x14 ,0x2A ,0xC6 ,0x95 ,0x0A ,0x5B ,0x81,
      0x2E ,0xDC ,0x91 ,0x3C ,0x3E ,0x12 ,0x63, 0x3C ,0xCC ,0x93 ,0x21 ,0xCA ,0xAA ,0x9F ,0x4A ,0x12,
      0xE9 ,0x3F ,0x53 ,0x9A ,0x52 ,0xA8 ,0x26, 0x20 ,0x13 ,0x31 ,0xE3 ,0xEC ,0xD3 ,0xE1 ,0x23 ,0x6E,
      0xB8 ,0xC4 ,0xE8 ,0xB8 ,0x4C ,0xAF ,0x88, 0x55 ,0x14 ,0x9D ,0x2E ,0xF5 ,0xE4 ,0xB0 ,0xD4 ,0x8E,
      0x6C ,0x47 ,0x39 ,0xA2 ,0x52 ,0x57 ,0xC5, 0xE1 ,0xF8 ,0x0F ,0x44 ,0x32 ,0x36 ,0x72 ,0xCD ,0xF1
               };

typedef struct _PacketHeaderDebug
{
	unsigned short  ushID      : 12,     // Packet ID            //max 0x0FFF  (4095)
					ushNeedAck : 1,      // Packet need ack
					ushCompress: 1,      // Packet compressed
					ushSplit   : 1,      // Packet is splitted
					ushReserved: 1;
	unsigned char   uchCRC8;
	unsigned char   uchPartNumber;
} PacketHeaderDebug;

//--METHOD IMPLEMENTATION-------------------------------------------------
//
/*! Default constructor
*/
//------------------------------------------------------------------------
UDPClient::UDPClient()
:   m_pSocket1(NULL)
   ,m_pSocket2(NULL)
	,m_hReceiveThread1(NULL)
   ,m_hReceiveThread2(NULL)
	,m_bReceiveThread1(FALSE)
   ,m_bReceiveThread2(FALSE)
	,m_pFcnRecvData(NULL)
{
   ::InitializeCriticalSection(&crLost);
   ::InitializeCriticalSection(&m_crLockCallbackMultipleIP);
   vConnectionLost.clear();

   m_pSendTmpBuffer = new char[PACKET_MAX_SIZE];

   char pszBuffer[MAX_PATH*2];
   int loop = GetModuleFileName( GetModuleHandle( NULL ), pszBuffer, _MAX_PATH * 2 );		
   do
   {
	   loop--;
   } while( pszBuffer[ loop ] != '\\' && loop >= 0 );
   // End string after backslash.
   pszBuffer[ loop + 1 ] = 0;
   int iSvrID = 0;
   char strIDF[512];
   sprintf_s(strIDF,512,"%ssvr.ID",pszBuffer);
   FILE *pfID = NULL;
   fopen_s(&pfID,strIDF,"rt");
   if(pfID)
   {
	   char *pstrRead;
	   char strLine[1024];
	   pstrRead = fgets(strLine,1024,pfID);
	   if(pstrRead)
		   iSvrID = atoi(strLine);
	   fclose(pfID);
   }
   if(iSvrID < 0 || iSvrID >9)
	   iSvrID = 0;


   SYSTEMTIME sysTime; 
   GetLocalTime(&sysTime);
   sprintf_s(m_strLogFileName,1024,"C:\\!!LOG_UDP_ALL");
   _mkdir(m_strLogFileName);
   sprintf_s(m_strLogFileName,1024,"C:\\!!LOG_UDP_ALL\\LogAll%d_%04d-%02d-%02d %02dh%02d_%02d.txt",iSvrID,
			 sysTime.wYear, sysTime.wMonth,sysTime.wDay,sysTime.wHour, sysTime.wMinute,sysTime.wSecond);
}

//--METHOD IMPLEMENTATION-------------------------------------------------
//
/*! Destructor
*/
//------------------------------------------------------------------------
UDPClient::~UDPClient()
{
    Disconnect();

    if(m_pSendTmpBuffer)
       delete []m_pSendTmpBuffer;
    m_pSendTmpBuffer = NULL;

    ::DeleteCriticalSection(&crLost);
    ::DeleteCriticalSection(&m_crLockCallbackMultipleIP);
}

//--METHOD IMPLEMENTATION-------------------------------------------------
//
/*! Sets the receive callback and parameters

\param pCallbackRecvData Callback
\param pCallbackParam Callback parameter (can be NULL)
\param ReceiveBufferSize Receive buffer size (see notes for recommended size)
when the packet size is (the size of size have to be uint32_t).

\return An error code.

notes: Recommended receive buffer size is 2 times the expected data size.

\exception
*/
//------------------------------------------------------------------------
int UDPClient::SetReceiveParameters(pfNMSocketRecvData pCallbackRecvData,void* pCallbackParam,size_t ReceiveBufferSize)
{
	m_pFcnRecvData  = pCallbackRecvData;
	m_pFcnRecvParam = pCallbackParam;
	m_ReceiveBufferSize = ReceiveBufferSize;

	return eNO_ERROR;
}




int UDPClient::StartServer(LPCTSTR Address, int Port, bool bBuffered)
{
    int Ret;
    WSADATA wsaData = { 0 };

    // Initialize winsock
    if ((Ret = WSAStartup(MAKEWORD(2,0), &wsaData)) != 0)
    {
        return eERROR_INITIALIZATION;
    }

    // Create the socket
    SOCKET hSocket = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, WSA_FLAG_OVERLAPPED);
    //SOCKET hSocket = socket(AF_INET, SOCK_DGRAM,0);
    if (hSocket == INVALID_SOCKET)
    {
        WSACleanup();
        return eERROR_INITIALIZATION;
    }

    //Set socket All Options now...
    {
       BOOL optval = TRUE;
       int bufsize = (1024 * 1024)*10; //4 megs buffer

       int iErr;
       
       //Set socket options...
       //reuse adresss
       iErr = setsockopt( hSocket,SOL_SOCKET, SO_REUSEADDR, (char *) &optval , sizeof( BOOL ));
       //Allow Broadcast
       iErr = setsockopt( hSocket,SOL_SOCKET, SO_BROADCAST, (char *) &optval , sizeof( BOOL ));
       //Buffser size
       iErr = setsockopt( hSocket,SOL_SOCKET, SO_RCVBUF   , (char *) &bufsize, sizeof bufsize);
       iErr = setsockopt( hSocket,SOL_SOCKET, SO_SNDBUF   , (char *) &bufsize, sizeof bufsize);
    }
    

    memset(&m_sockAddrLocal1  , 0, sizeof(m_sockAddrLocal1));
    memset(&m_sockAddrSender1 , 0, sizeof(m_sockAddrSender1));
    

    m_sockAddrLocal1.sin_family = AF_INET;
    m_sockAddrLocal1.sin_port = htons(Port);
    m_sockAddrLocal1.sin_addr.S_un.S_addr = inet_addr(Address);

    if (bind(hSocket, (SOCKADDR *) & m_sockAddrLocal1, sizeof (m_sockAddrLocal1)) == 0)
    {
        m_pSocket1 = new NMSocket;
        m_pSocket1->Init(hSocket);

        UINT threadId;
        m_hReceiveThread1 = (HANDLE)_beginthreadex( NULL, 0, fReceiveData1 , this, 0, &threadId ); //OK: thread qui recoit les packets (SERVER ONLY)
 		  SetThreadPriority( m_hReceiveThread1      , THREAD_PRIORITY_ABOVE_NORMAL  );
    }
    else
    {
        closesocket(hSocket);
        WSACleanup();
        return eERROR_INITIALIZATION;
    }
    
    return eNO_ERROR;
}

int UDPClient::StartServer2(LPCTSTR Address, int Port, bool bBuffered)
{
   // Create the socket
   SOCKET hSocket = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, WSA_FLAG_OVERLAPPED);
   //SOCKET hSocket = socket(AF_INET, SOCK_DGRAM,0);
   if (hSocket == INVALID_SOCKET)
   {
      return eERROR_INITIALIZATION;
   }

   //Set socket All Options now...
   {
      BOOL optval = TRUE;
      int bufsize = (1024 * 1024)*10; //4 megs buffer

      int iErr;

      //Set socket options...
      //reuse adresss
      iErr = setsockopt( hSocket,SOL_SOCKET, SO_REUSEADDR, (char *) &optval , sizeof( BOOL ));
      //Allow Broadcast
      iErr = setsockopt( hSocket,SOL_SOCKET, SO_BROADCAST, (char *) &optval , sizeof( BOOL ));
      //Buffser size
      iErr = setsockopt( hSocket,SOL_SOCKET, SO_RCVBUF   , (char *) &bufsize, sizeof bufsize);
      iErr = setsockopt( hSocket,SOL_SOCKET, SO_SNDBUF   , (char *) &bufsize, sizeof bufsize);
   }


   memset(&m_sockAddrLocal2  , 0, sizeof(m_sockAddrLocal2));
   memset(&m_sockAddrSender2 , 0, sizeof(m_sockAddrSender2));


   m_sockAddrLocal2.sin_family = AF_INET;
   m_sockAddrLocal2.sin_port = htons(Port);
   m_sockAddrLocal2.sin_addr.S_un.S_addr = inet_addr(Address);

   if (bind(hSocket, (SOCKADDR *) & m_sockAddrLocal2, sizeof (m_sockAddrLocal2)) == 0)
   {
      m_pSocket2 = new NMSocket;
      m_pSocket2->Init(hSocket);

      UINT threadId;
      m_hReceiveThread2 = (HANDLE)_beginthreadex( NULL, 0, fReceiveData2 , this, 0, &threadId ); //OK: thread qui recoit les packets (SERVER ONLY)
      SetThreadPriority( m_hReceiveThread2     , THREAD_PRIORITY_ABOVE_NORMAL  );
   }
   else
   {
      closesocket(hSocket);
      return eERROR_INITIALIZATION;
   }

   return eNO_ERROR;
}




//--METHOD IMPLEMENTATION-------------------------------------------------
//
/*! Disconnect the client from the server.

\param Address Server address.
\param Port Server port.

\return An error code.
*/
//------------------------------------------------------------------------
void UDPClient::Disconnect()
{
   //Close receive thread
   if (m_pSocket1)
   {
      // Close the socket
      closesocket(m_pSocket1->m_hSocket);
   }
   if (m_pSocket2)
   {
      // Close the socket
      closesocket(m_pSocket2->m_hSocket);
   }

   //Close receive thread
   if(m_hReceiveThread1)
   {
      m_bReceiveThread1  = FALSE;
      if(::WaitForSingleObject(m_hReceiveThread1, 2000) == WAIT_TIMEOUT ) 
         TerminateThread( m_hReceiveThread1, 0x666 );
      m_hReceiveThread1 = NULL;
   }

   if(m_hReceiveThread2)
   {
      m_bReceiveThread2  = FALSE;
      if(::WaitForSingleObject(m_hReceiveThread2, 2000) == WAIT_TIMEOUT ) 
         TerminateThread( m_hReceiveThread2, 0x666 );
      m_hReceiveThread2 = NULL;
   }

   if (m_pSocket1)
   {
      delete m_pSocket1;
      m_pSocket1 = NULL;
   }

   if (m_pSocket2)
   {
      delete m_pSocket2;
      m_pSocket2 = NULL;
   }

   
   
}


int UDPClient::SendTo(sockaddr_in sockToO,sockaddr_in sockToI,char* pBuff, size_t BuffLenght)
{
   if(sockToI.sin_addr.S_un.S_un_b.s_b1 == m_sockAddrLocal1.sin_addr.S_un.S_un_b.s_b1 &&
      sockToI.sin_addr.S_un.S_un_b.s_b2 == m_sockAddrLocal1.sin_addr.S_un.S_un_b.s_b2 &&
      sockToI.sin_addr.S_un.S_un_b.s_b3 == m_sockAddrLocal1.sin_addr.S_un.S_un_b.s_b3 &&
      sockToI.sin_addr.S_un.S_un_b.s_b4 == m_sockAddrLocal1.sin_addr.S_un.S_un_b.s_b4    )
   {
      DWORD BytesSent = 0;
      DWORD Flags = 0;
      WSABUF DataBuf;
      DataBuf.len = static_cast<u_long>(BuffLenght);

      DataBuf.buf = pBuff;
      int Ret = WSASendTo( m_pSocket1->m_hSocket, &DataBuf, 1, &BytesSent, 0,(sockaddr *)&sockToO, sizeof( sockaddr_in ),&m_pSocket1->m_OverlappedSend, NULL);

      if (Ret == SOCKET_ERROR) 
      {
         Ret = WSAGetLastError();
         if (Ret != WSA_IO_PENDING) 
         {
            EnterCriticalSection(&crLost);
            vConnectionLost.push_back( sockToO );
            LeaveCriticalSection(&crLost);
            return eERROR_COULD_NOT_SEND;
         }
      }
   }
   else
   {
      DWORD BytesSent = 0;
      DWORD Flags = 0;
      WSABUF DataBuf;
      DataBuf.len = static_cast<u_long>(BuffLenght);

      DataBuf.buf = pBuff;
      int Ret = WSASendTo( m_pSocket2->m_hSocket, &DataBuf, 1, &BytesSent, 0,(sockaddr *)&sockToO, sizeof( sockaddr_in ),&m_pSocket2->m_OverlappedSend, NULL);

      if (Ret == SOCKET_ERROR) 
      {
         Ret = WSAGetLastError();
         if (Ret != WSA_IO_PENDING) 
         {
            EnterCriticalSection(&crLost);
            vConnectionLost.push_back( sockToO );
            LeaveCriticalSection(&crLost);
            return eERROR_COULD_NOT_SEND;
         }
      }
   }
      


   return eNO_ERROR;
}







ULONG UDPClient::GetIPAddress( LPCTSTR strHostName )
{
   LPHOSTENT   lphostent;
   ULONG       uAddr = INADDR_NONE;
   TCHAR       strLocal[MAX_PATH] = { 0 };

   // if no name specified, get local
   if ( NULL == strHostName )
   {
      GetLocalName(strLocal, sizeof(strLocal));
      strHostName = strLocal;
   }

   LPCTSTR strHost = strHostName;

   // Check for an Internet Protocol dotted address string
   uAddr = inet_addr( strHost );

   if ( (INADDR_NONE == uAddr) && (strcmp( strHost, "255.255.255.255" )) )
   {
      // It's not an address, then try to resolve it as a hostname
      if ( lphostent = gethostbyname( strHost ) )
         uAddr = *((ULONG *) lphostent->h_addr_list[0]);
   }

   return ntohl( uAddr );
}

bool UDPClient::GetLocalName(LPTSTR strName, UINT nSize)
{
   if (strName != NULL && nSize > 0)
   {
      char strHost[MAX_PATH] = { 0 };

      // get host name, if fail, SetLastError is set
      if (SOCKET_ERROR != gethostname(strHost, sizeof(strHost)))
      {
         struct hostent* hp;
         hp = gethostbyname(strHost);
         if (hp != NULL) {
            strncpy_s(strHost, hp->h_name, MAX_PATH);
         }

         // check if user provide enough buffer
         if (strlen(strHost) > nSize)
         {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return false;
         }

         // Unicode conversion
         _tcscpy_s(strName,MAX_PATH, strHost);
         return true;
      }
   }
   else
      SetLastError(ERROR_INVALID_PARAMETER);
   return false;
}


UINT UDPClient::fReceiveData1(LPVOID pParameter)
{
    UDPClient* me = reinterpret_cast<UDPClient*>(pParameter);
    NMSocket* pSocket = me->m_pSocket1;

    me->m_bReceiveThread1 = TRUE;

    DWORD Flags;
    WSABUF WsaBuffer;

    DWORD RecvBytes;
    int nLen = sizeof( sockaddr_in );
    BYTE* pData = new BYTE[PACKET_MAX_SIZE];

    while (me->m_bReceiveThread1)
    {
        RecvBytes = 0;	
        Flags = 0;
        size_t ReservedSize = PACKET_MAX_SIZE;
        WsaBuffer.buf = reinterpret_cast<char*>(pData);
        WsaBuffer.len = static_cast<u_long>(ReservedSize);

        int Ret = WSARecvFrom(pSocket->m_hSocket, &WsaBuffer, 1, &RecvBytes, &Flags,(sockaddr *)&me->m_sockAddrSender1, &nLen,&pSocket->m_OverlappedRecv, NULL);
        if (Ret == SOCKET_ERROR)
        {
            Ret = WSAGetLastError();
            if (Ret == WSA_IO_PENDING)
            {
                // ...then wait for the IO to complete.
                Ret = WSAWaitForMultipleEvents(1, &pSocket->m_OverlappedRecv.hEvent, TRUE, WSA_INFINITE, TRUE);
                WSAResetEvent(pSocket->m_OverlappedRecv.hEvent);
                if (Ret != WSA_WAIT_EVENT_0)
                {
                }

                // Get the result now that the event was signaled.
                Ret = WSAGetOverlappedResult(pSocket->m_hSocket, &pSocket->m_OverlappedRecv, &RecvBytes, FALSE, &Flags);
                if (Ret == FALSE)
                {
                }
                
                if (me->m_pFcnRecvData && RecvBytes >0 && Ret)
                {
                   ::EnterCriticalSection(&me->m_crLockCallbackMultipleIP);
				   PacketHeaderDebug *pHeader = NULL;
				   BOOL bSkip = FALSE;
				   if(RecvBytes >=4)
				   {
					   pHeader= (PacketHeaderDebug*)pData;
					   if(pHeader->ushCompress || pHeader->uchPartNumber > 32)
						   bSkip = TRUE;
				   }

				   /////////////////////////////////////////////////////////////////////
				   //DEBUG UDP ALL LOGS....
				   /*
					   try
					   {
						   if(!bSkip && RecvBytes >20)
						   {
							   SYSTEMTIME sysTime; 
							   GetLocalTime(&sysTime);
							   FILE *pft = NULL;
							   fopen_s(&pft,me->m_strLogFileName,"a+");
							   if(pft)
							   {
								   fprintf(pft,"R->%04d-%02d-%02d %02dh%02d.%02d %04d bytes (%d.%d.%d.%d) :",
									   sysTime.wYear, sysTime.wMonth,sysTime.wDay,sysTime.wHour, sysTime.wMinute,sysTime.wSecond,
									   RecvBytes,
									   me->m_sockAddrSender1.sin_addr.S_un.S_un_b.s_b1,me->m_sockAddrSender1.sin_addr.S_un.S_un_b.s_b2,
									   me->m_sockAddrSender1.sin_addr.S_un.S_un_b.s_b3,me->m_sockAddrSender1.sin_addr.S_un.S_un_b.s_b4) ;
								   int iMax = 10;
								   if(RecvBytes < 10)
									   iMax = RecvBytes;
								   for(int p=0;p<iMax;p++)
									  fprintf(pft,"[%02X] ",pData[p]);

								   if(RecvBytes >=4 && pHeader)
								   {
									   if(bSkip)
										   fprintf(pft,"***REJECT*** ");
									   fprintf(pft,"  (ID=%d,%d,%d,%d,%d   %d   %d)",pHeader->ushID,pHeader->ushNeedAck,pHeader->ushCompress,pHeader->ushSplit,pHeader->ushReserved,pHeader->uchCRC8,pHeader->uchPartNumber);
								   }

								   if(RecvBytes <= 10)
									  fprintf(pft,"\n");
								   else
									  fprintf(pft,"...\n");
								   fclose(pft);
								}
						   }
					   }
					   catch (...)
					   {

					   }
				   */
				   //
				   /////////////////////////////////////////////////////////////////////
					
				   
					try
					{
						if(!bSkip)
							(*me->m_pFcnRecvData)(me->m_sockAddrLocal1,me->m_sockAddrSender1,reinterpret_cast<unsigned char*>(&pData[0]), RecvBytes,0,0,me->m_pFcnRecvParam);
					}
					catch (...)
					{
						
					}
                   ::LeaveCriticalSection(&me->m_crLockCallbackMultipleIP);
                }
            }
            else
            {
            }
        }
        else
        {
            if (me->m_pFcnRecvData  && RecvBytes >0)
            {
               ::EnterCriticalSection(&me->m_crLockCallbackMultipleIP);

			   PacketHeaderDebug *pHeader = NULL;
			   BOOL bSkip = FALSE;
			   if(RecvBytes >=4)
			   {
				   pHeader= (PacketHeaderDebug*)pData;
				   if(pHeader->ushCompress || pHeader->uchPartNumber > 32)
					   bSkip = TRUE;
			   }

			   /////////////////////////////////////////////////////////////////////
			   //DEBUG UDP ALL LOGS....
			   /*
			   try
			   {
				   if(!bSkip && RecvBytes >20)
				   {
					   SYSTEMTIME sysTime; 
					   GetLocalTime(&sysTime);
					   FILE *pft = NULL;
					   fopen_s(&pft,me->m_strLogFileName,"a+");
					   if(pft)
					   {
						   fprintf(pft,"R->%04d-%02d-%02d %02dh%02d.%02d %04d bytes (%d.%d.%d.%d) :",
							   sysTime.wYear, sysTime.wMonth,sysTime.wDay,sysTime.wHour, sysTime.wMinute,sysTime.wSecond,
							   RecvBytes,
							   me->m_sockAddrSender1.sin_addr.S_un.S_un_b.s_b1,me->m_sockAddrSender1.sin_addr.S_un.S_un_b.s_b2,
							   me->m_sockAddrSender1.sin_addr.S_un.S_un_b.s_b3,me->m_sockAddrSender1.sin_addr.S_un.S_un_b.s_b4) ;
						   int iMax = 10;
						   if(RecvBytes < 10)
							   iMax = RecvBytes;
						   for(int p=0;p<iMax;p++)
							   fprintf(pft,"[%02X] ",pData[p]);

						   if(RecvBytes >=4 && pHeader)
						   {
							   if(bSkip)
								   fprintf(pft,"***REJECT*** ");
							   fprintf(pft,"  (ID=%d,%d,%d,%d,%d   %d   %d)",pHeader->ushID,pHeader->ushNeedAck,pHeader->ushCompress,pHeader->ushSplit,pHeader->ushReserved,pHeader->uchCRC8,pHeader->uchPartNumber);
						   }


						   if(RecvBytes <= 10)
							   fprintf(pft,"\n");
						   else
							   fprintf(pft,"...\n");
						   fclose(pft);
					   }
				   }
			   }
			   catch (...)
			   {

			   }
			   */
			   //
			   /////////////////////////////////////////////////////////////////////

			   try
			   {
				   if(!bSkip)
					 (*me->m_pFcnRecvData)(me->m_sockAddrLocal1,me->m_sockAddrSender1,reinterpret_cast<unsigned char*>(&pData[0]), RecvBytes,0,0,me->m_pFcnRecvParam);
			   }
			   catch (...)
			   {

			   }
               
               ::LeaveCriticalSection(&me->m_crLockCallbackMultipleIP);
            }
        }

        Sleep(0);
    }


    if(pData)
        delete []pData;
    pData = NULL;

    return 0;
}

UINT UDPClient::fReceiveData2(LPVOID pParameter)
{
   UDPClient* me = reinterpret_cast<UDPClient*>(pParameter);
   NMSocket* pSocket = me->m_pSocket2;

   me->m_bReceiveThread2 = TRUE;

   DWORD Flags;
   WSABUF WsaBuffer;

   DWORD RecvBytes;
   int nLen = sizeof( sockaddr_in );
   BYTE* pData = new BYTE[PACKET_MAX_SIZE];

   while (me->m_bReceiveThread2)
   {
      RecvBytes = 0;	
      Flags = 0;
      size_t ReservedSize = PACKET_MAX_SIZE;
      WsaBuffer.buf = reinterpret_cast<char*>(pData);
      WsaBuffer.len = static_cast<u_long>(ReservedSize);

      int Ret = WSARecvFrom(pSocket->m_hSocket, &WsaBuffer, 1, &RecvBytes, &Flags,(sockaddr *)&me->m_sockAddrSender2, &nLen,&pSocket->m_OverlappedRecv, NULL);
      if (Ret == SOCKET_ERROR)
      {
         Ret = WSAGetLastError();
         if (Ret == WSA_IO_PENDING)
         {
            // ...then wait for the IO to complete.
            Ret = WSAWaitForMultipleEvents(1, &pSocket->m_OverlappedRecv.hEvent, TRUE, WSA_INFINITE, TRUE);
            WSAResetEvent(pSocket->m_OverlappedRecv.hEvent);
            if (Ret != WSA_WAIT_EVENT_0)
            {
            }

            // Get the result now that the event was signaled.
            Ret = WSAGetOverlappedResult(pSocket->m_hSocket, &pSocket->m_OverlappedRecv, &RecvBytes, FALSE, &Flags);
            if (Ret == FALSE)
            {
            }

            if (me->m_pFcnRecvData && RecvBytes >0 && Ret)
            {
               ::EnterCriticalSection(&me->m_crLockCallbackMultipleIP);
               (*me->m_pFcnRecvData)(me->m_sockAddrLocal2,me->m_sockAddrSender2,reinterpret_cast<unsigned char*>(&pData[0]), RecvBytes,0,0,me->m_pFcnRecvParam);
               ::LeaveCriticalSection(&me->m_crLockCallbackMultipleIP);
            }
         }
         else
         {
         }
      }
      else
      {
         if (me->m_pFcnRecvData  && RecvBytes >0)
         {
            ::EnterCriticalSection(&me->m_crLockCallbackMultipleIP);
            (*me->m_pFcnRecvData)(me->m_sockAddrLocal2,me->m_sockAddrSender2,reinterpret_cast<unsigned char*>(&pData[0]), RecvBytes,0,0,me->m_pFcnRecvParam);
            ::LeaveCriticalSection(&me->m_crLockCallbackMultipleIP);
         }
      }

      Sleep(0);
   }


   if(pData)
      delete []pData;
   pData = NULL;

   return 0;
}


vector< sockaddr_in > *UDPClient::GetLostConnections( void )
/******************************************************************************/
{
	EnterCriticalSection(&crLost);
	return &vConnectionLost;
}
/******************************************************************************/
// Frees the lost connection vector fetched with GetLostConnection.
void UDPClient::FreeLostConnections(BOOL boFlushList)
/******************************************************************************/
{
	if( boFlushList )
	{
		// Flush the vector.
		vConnectionLost.erase( vConnectionLost.begin(), vConnectionLost.end() );
	}
	LeaveCriticalSection(&crLost);
}