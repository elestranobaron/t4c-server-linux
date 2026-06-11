//--FILE DEFINITION--------------------------------------------------------
//
/*! \file
   File name:  NMPacketManager.cpp
*/
//-------------------------------------------------------------------------



#include "..\stdafx.h"
#include <mmsystem.h>
#include <direct.h>

#include <process.h>
#include "NMPacketManager.h"

#pragma comment(lib, "Zlib/libzlib-vc91.lib")

#include "..\TFC_Main.h"
#include "..\TFC Server.h"

#include "..\T4CLog.h"
#include "..\DeadlockDetector.h"
#include "..\ThreadMonitor.h"
#include "..\PlayerManager.h"

extern CTFCServerApp theApp;


#define  WAIT_THREAD( __thread ) if( WaitForSingleObject( __thread, 500 ) == WAIT_TIMEOUT ) {\
                                     TerminateThread( __thread, 1 ); }

#define  MEM_COMPRESS_NEED(ActualLen) (ULONG)(ActualLen *1.1 + 12);

unsigned short Reverse16 (unsigned short n) /* 16 Bits */
{
   __asm mov ax,n
      __asm rol ax,8
      /* Return with result in ax */
}


//--METHOD IMPLEMENTATION-------------------------------------------------
//
/*! Default constructor
*/
//------------------------------------------------------------------------
NMPacketManager::NMPacketManager()
{
   
   m_pUDP        = NULL;
   m_bInit       = false;
   
   m_bUDPMaintenancePacket   = TRUE;
   m_hUDPMaintenancePacket   = NULL;

   m_bUDPProcessPacketThread = TRUE;
   m_hUDPProcessPacketThread = NULL;
   m_hUDPReceivePacketIO     = NULL;

   m_bUDPSendPacketThread = TRUE;
   m_hUDPSendPacketThread = NULL;
   m_hUDPSendPacketIO     = NULL;

   m_bLostConnThread = TRUE;
   m_hLostConnThread = NULL;
   m_hLostConnIO     = NULL;

   m_bServerStarted  = FALSE;
   m_bCanStartComm   = FALSE;

   m_pUDPClientConnections = NULL;

   UINT threadId;
   (HANDLE)_beginthreadex( NULL, 0, OneShotEnableCommThread, this, 0, &threadId ); //OK: thread qui attend un delai X avant de demarrer la COMM.
   
   InitializeCriticalSection(&m_crConnectionLock);
   InitializeCriticalSection(&m_crInterLockCnt);

   m_pdwNbrPacketSendbyPacket = NULL;
   m_pdwNbrPacketRecvbyPacket = NULL;

   SYSTEMTIME sysTime; 
   GetLocalTime(&sysTime);
   sprintf_s(m_strLogFileName,1024,"C:\\!!LOG_UDP_ALL");
   _mkdir(m_strLogFileName);
   sprintf_s(m_strLogFileName,1024,"C:\\!!LOG_UDP_ALL\\LogSplit_%04d-%02d-%02d %02dh%02d_%02d.txt",
	   sysTime.wYear, sysTime.wMonth,sysTime.wDay,sysTime.wHour, sysTime.wMinute,sysTime.wSecond);

}

//--METHOD IMPLEMENTATION-------------------------------------------------
//
/*! Destructor
*/
//------------------------------------------------------------------------
NMPacketManager::~NMPacketManager()
{
   if(m_pUDP)
      m_pUDP->Disconnect();

   m_bUDPMaintenancePacket        = FALSE;
   m_bUDPProcessPacketThread      = FALSE;
   m_bUDPSendPacketThread         = FALSE;
   m_bLostConnThread              = FALSE;
      

   // Signal sending and interpretation queues to start running.
   if(m_hUDPReceivePacketIO)
      CancelIo( m_hUDPReceivePacketIO);
   if(m_hUDPSendPacketIO)
      CancelIo( m_hUDPSendPacketIO);
   if(m_hLostConnIO)
      CancelIo( m_hLostConnIO);

   Sleep(2000);
  

   //Detruit toutes les connections...
   EnterCriticalSection(&m_crConnectionLock);

   for(int i = 0; i < m_UDPConnectionsList.size(); i++ )
   {
      if(m_UDPConnectionsList[i].pConn)
         delete m_UDPConnectionsList[i].pConn;
      m_UDPConnectionsList[i].pConn = NULL;
   }
   m_UDPConnectionsList.clear();

   if(m_pUDPClientConnections)
      delete m_pUDPClientConnections;
   m_pUDPClientConnections = NULL;
   LeaveCriticalSection(&m_crConnectionLock);

   if(m_hUDPProcessPacketThread)
      WAIT_THREAD( m_hUDPProcessPacketThread );
   if(m_hUDPSendPacketThread)
      WAIT_THREAD( m_hUDPSendPacketThread );
   if(m_hUDPMaintenancePacket)
      WAIT_THREAD( m_hUDPMaintenancePacket );
   if(m_hLostConnThread)
      WAIT_THREAD( m_hLostConnThread );

  
   m_hUDPProcessPacketThread   = NULL;
   m_hUDPSendPacketThread      = NULL;
   m_hUDPMaintenancePacket     = NULL;
   m_hLostConnThread           = NULL;
   

	if(m_pUDP)
   {
      delete m_pUDP;
      m_pUDP = NULL;
   }

	if(m_pdwNbrPacketSendbyPacket)
		delete m_pdwNbrPacketSendbyPacket;
	m_pdwNbrPacketSendbyPacket = NULL;

	if(m_pdwNbrPacketRecvbyPacket)
		delete m_pdwNbrPacketRecvbyPacket;
	m_pdwNbrPacketRecvbyPacket = NULL;


   DeleteCriticalSection(&m_crConnectionLock);
   DeleteCriticalSection(&m_crInterLockCnt);
}

CUDPConnection* NMPacketManager::GetConnection(sockaddr_in sockAddr,bool bCreate)
{
   for(int i=0; i<m_UDPConnectionsList.size(); i++)
   {
      //on compare le port en premier...
      if(m_UDPConnectionsList[i].sockIN.sin_port == sockAddr.sin_port)
      {
         //meme port on compare le IP
         if(m_UDPConnectionsList[i].sockIN.sin_addr.S_un.S_un_b.s_b1 == sockAddr.sin_addr.S_un.S_un_b.s_b1 &&
            m_UDPConnectionsList[i].sockIN.sin_addr.S_un.S_un_b.s_b2 == sockAddr.sin_addr.S_un.S_un_b.s_b2 &&
            m_UDPConnectionsList[i].sockIN.sin_addr.S_un.S_un_b.s_b3 == sockAddr.sin_addr.S_un.S_un_b.s_b3 &&
            m_UDPConnectionsList[i].sockIN.sin_addr.S_un.S_un_b.s_b4 == sockAddr.sin_addr.S_un.S_un_b.s_b4    )
         {
            return m_UDPConnectionsList[i].pConn;
            //NMNMNMNM A Valider que des fois la connection ets retourner et que sa en cree pas en boucle...
         }
      }
   }
   //on a pas trouver on cree un nouveau :P
   sConnectionList newConnection;
   newConnection.sockIN = sockAddr;
   newConnection.pConn = new CUDPConnection();
   newConnection.pConn->InitializeComm(sockAddr, this);
   m_UDPConnectionsList.push_back(newConnection);
   return newConnection.pConn;
}



bool NMPacketManager::Init(pfProcessCallback pfProcessCallback,void *pThis,bool bStartAsServer,int iPort1,const char *pstrIP1,int iPort2,const char *pstrIP2)
{
   if(m_bInit)
      return false;

   m_dwGlobalNbrRetry                = 0;
   m_dwGlobalNbrLost                 = 0;
   m_dwNbrPacketSend                 = 0;
   m_dwNbrPacketRecv                 = 0;
   m_dwNbrPacketRecvAlreadyRegistred = 0;

   m_pdwNbrPacketSendbyPacket = new DWORD[0xFFFF];
   m_pdwNbrPacketRecvbyPacket = new DWORD[0xFFFF];
   for(int i=0;i<0xFFFF;i++)
   {
	   m_pdwNbrPacketSendbyPacket[i] = 0;
	   m_pdwNbrPacketRecvbyPacket[i] = 0;
   }

   m_pParent = pThis;

   m_pfProcessCallback = pfProcessCallback;

   m_pUDP = new UDPClient();
   m_pUDP->SetReceiveParameters(ReceiveUDPCallback,this,PACKET_MAX_SIZE);	// Receive buffer size

   int iErr1;
   int iErr2 = 0;
   if(bStartAsServer)
   {
      iErr1 = m_pUDP->StartServer(pstrIP1,iPort1,false);
      if(!iErr1 && strcmp(pstrIP2,"")!=0)
      {
         iErr2 = m_pUDP->StartServer2(pstrIP2,iPort2,false);
      }
   }
  
   if(!iErr1 && !iErr2)
   {
      m_bInit               = true;
      m_uiNbrInterpQueueCnt = 0;

      m_hUDPReceivePacketIO = CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL, 0, 1 );
      m_hUDPSendPacketIO    = CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL, 0, 1 );
      m_hLostConnIO         = CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL, 0, 1 );
      
      UINT threadId;
      m_hUDPMaintenancePacket   = (HANDLE)_beginthreadex( NULL, 0, UDPMaintenancePacket    , this, 0, &threadId ); //OK: thread qui verifie les packet pending splitter et autres
      m_hUDPProcessPacketThread = (HANDLE)_beginthreadex( NULL, 0, UDPProcessPacketFct     , this, 0, &threadId ); //OK: thread qui process et CALLBACK les donnees au parent
      m_hUDPSendPacketThread    = (HANDLE)_beginthreadex( NULL, 0, UDPSendPacketFct        , this, 0, &threadId ); //OK: thread qui envoie les packets au client
      m_hLostConnThread         = (HANDLE)_beginthreadex( NULL, 0, UDPLostConnectionThread , this, 0, &threadId ); //OK: thread qui efface les connection detruites...
    
      SetThreadPriority( m_hUDPSendPacketThread      , THREAD_PRIORITY_HIGHEST );
      SetThreadPriority( m_hUDPProcessPacketThread   , THREAD_PRIORITY_ABOVE_NORMAL );
   }

   return m_bInit;
}

int NMPacketManager::GetNbrConnection()
{
   int iNbr = 0;
   EnterCriticalSection(&m_crConnectionLock);
   iNbr = m_UDPConnectionsList.size();
   LeaveCriticalSection(&m_crConnectionLock);
   return iNbr;
}
 

int NMPacketManager::ReceiveUDPCallback(sockaddr_in sockAddrInIP,sockaddr_in sockAddr,unsigned char* pData, size_t DataLenght, int iQueueNbrItems, int iNbrPacketLost, void* pParam)
{
   NMPacketManager* me = reinterpret_cast<NMPacketManager*>(pParam);

   if(!me->m_bServerStarted) //server not started, we not accept packet...
      return 0;
   
   if( DataLenght < sizeof(PacketHeader) || DataLenght > PACKET_MAX_SIZE)
   {
      //AddToLogFile(TRUE,"+++ Receive < %d or > %d byte0 == \n", sizeof(PacketHeader) ,PACKET_MAX_SIZE);
      if(DataLenght == 1 && pData[ 0 ] == 0xF1)
      {
         //AddToLogFile(TRUE,"+++ Receive WatchDOG\n");
         //une commande du watch DOG...
         //lors du send sera traiter et rempli avecd es donn en consequence,
         //le protocole n'est pas utiliser avec cette commande...
         UDPPacket *pPacket = me->PacketAlloc(7);
         pPacket->ulBufferLength = 1;
         pPacket->pBuffer[ 0 ]   = 0xF1;
         pPacket->sockAddrO    = sockAddr; 
         pPacket->sockAddrI    = sockAddrInIP; 
         pPacket->boDelete     = TRUE; 
         pPacket->boAckReceived= FALSE;
         pPacket->boAddPending = FALSE;
         pPacket->ulNbrAck     = 0;
         pPacket->ulAckDelay   = 0;

         me->PostSendPacket(pPacket); //oki watchdog resend direct...
                  
      }
      else
      {
         //on fait rien avec les donnee
         //on supprime meme pas les donnee les low level s'en charge...
         //on fait que proteger les class au dessus des packet
         //de trop grande taille ou trop petit
      }
      return 0;
   }
   
   UDPPacket *pPacket = me->PacketAlloc(DataLenght);
   try
   {
      if(pPacket)
      {
         pPacket->ulBufferLength = DataLenght;
         memcpy(pPacket->pBuffer,pData,DataLenght);      //copie toutes les donnees
         pPacket->sockAddrO = sockAddr;                  //copie le socket de celui qui a transmit
         pPacket->sockAddrI = sockAddrInIP;              //copie le socket de celui qui a transmit

         //ici on valid le checksum dur les donnee autres que le headers...
         //tous les packet invalides seront filtrer ici...
         bool bCheckSumOK = me->IsChksumOK(pPacket->pBuffer,pPacket->ulBufferLength,pPacket->pHeader->uchCRC8);
         if(!bCheckSumOK)
         {
            //AddToLogFile(TRUE,"   -> Invalid Checksum\n");
            me->PacketFree(pPacket); //RECV Le Packet a un MAUVAIS, traiter et detruit.
            return 0;
         }

         //si le packet demande un ACK, on envoie aussi le ACK
         if(pPacket->pHeader->ushNeedAck)
            me->SendAckPacket(pPacket);
         
         //we receive any packet from this USER, we reset the timeout of connection...
         EnterCriticalSection(&me->m_crConnectionLock);
         try
         {
            CUDPConnection* lpConnection = me->GetConnection( pPacket->sockAddrO ,true);
            if(lpConnection )
            {
               lpConnection->ResetTimeout();
               if(pPacket->ulBufferLength == sizeof(PacketHeader) && pPacket->pHeader->ushNeedAck == 0)
               {
                  // on a recu un acknoledge... on cherche la parity de cet acknoledge et on elimine le pending
                  lpConnection->DestroyPending(pPacket->pHeader->ushID);
                  me->PacketFree(pPacket); //RECV Le Packet est un ACKNOLEDGE, traiter et detruit.
               }
               else if(pPacket->pHeader->ushSplit) //Si c'est un packet splitter, on gere les fragment...
               {
                  pPacket->pHeaderSplit = (PacketHeaderSplit*)(pPacket->pBuffer+sizeof(PacketHeader));
                  pPacket->puchData     = (pPacket->pBuffer+sizeof(PacketHeader)+sizeof(PacketHeaderSplit));
                  pPacket->ulDataLength = pPacket->ulBufferLength - (sizeof(PacketHeader)+sizeof(PacketHeaderSplit));

				  /*
				  SYSTEMTIME sysTime; 
				  GetLocalTime(&sysTime);
				  FILE *pft = NULL;
				  fopen_s(&pft,me->m_strLogFileName,"a+");
				  if(pft)
				  {
					  fprintf(pft,"R->%04d-%02d-%02d %02dh%02d.%02d (%d.%d.%d.%d) : No=%d, Nbr=%d, Max=%d, L=%d\n",
						  sysTime.wYear, sysTime.wMonth,sysTime.wDay,sysTime.wHour, sysTime.wMinute,sysTime.wSecond,
						  pPacket->sockAddrO.sin_addr.S_un.S_un_b.s_b1,pPacket->sockAddrO.sin_addr.S_un.S_un_b.s_b2,
						  pPacket->sockAddrO.sin_addr.S_un.S_un_b.s_b3,pPacket->sockAddrO.sin_addr.S_un.S_un_b.s_b4,
						  pPacket->pHeaderSplit->uchSplitNO,
						  pPacket->pHeaderSplit->uchPartNbr,
						  pPacket->pHeaderSplit->uchMaxPart,
						  pPacket->pHeaderSplit->ushDataSize
						  ) ;
					  fclose(pft);
				  }
				  */



                  if(pPacket->pHeader->ushCompress)
                  {
                     pPacket->pHeaderComp   = (PacketHeaderComp*)(pPacket->pBuffer+sizeof(PacketHeader)+sizeof(PacketHeaderSplit));
                     pPacket->puchData     += sizeof(PacketHeaderComp);
                     pPacket->ulDataLength -= sizeof(PacketHeaderComp);
                  }
                  lpConnection->AddPacketFragment(pPacket);
                  me->PacketFree(pPacket);//RECV Le Packet est un fragment on ajoute au fragment, traiter et detruit.
               }
               else
               {
                  if(pPacket->pHeader->ushCompress)
                  {
                     pPacket->pHeaderComp  = (PacketHeaderComp*)(pPacket->pBuffer+sizeof(PacketHeader));
                     pPacket->puchData     = (pPacket->pBuffer+sizeof(PacketHeader)+sizeof(PacketHeaderComp));
                     pPacket->ulDataLength = pPacket->ulBufferLength - (sizeof(PacketHeader)+sizeof(PacketHeaderComp));
                  }
                  else
                  {
                     pPacket->puchData     = (pPacket->pBuffer+sizeof(PacketHeader));
                     pPacket->ulDataLength = pPacket->ulBufferLength - sizeof(PacketHeader);
                  }
                  //On a un packet complet compresser ou non, on envoie dans la liste des packet a traiter
			         me->PostReceivePacket(pPacket);
               }
            }
            else
            {
               //Impossible de recuperer la connection...
               //on flush et oublie ce packet simplement...
               me->PacketFree(pPacket); //RECV Le Packet , Impossible avoirt une connection on flush..., traiter et detruit.
            }
         }
         catch (...)
         {
            _LOG_DEBUG
               LOG_DEBUG_LVL1,
               "*********** TRY CATCH on ReceiveUDPCallback():Connection Part **********"
               LOG_
         }
         LeaveCriticalSection(&me->m_crConnectionLock);
      }
   }
   catch (...)
   {
      _LOG_DEBUG
         LOG_DEBUG_LVL1,
         "*********** TRY CATCH on ReceiveUDPCallback():Global **********"
         LOG_
   }

   return 0;
}


UINT NMPacketManager::UDPProcessPacketFct(LPVOID lpData)
{
   NMPacketManager *me = (NMPacketManager *)lpData;

    CAutoThreadMonitor tmMonitor("NMPacketManager::UDPReceivePacketThread");
    _LOG_DEBUG
       LOG_DEBUG_LVL1,
       "UDPProcessPacketFct Thread Id=%u",
       GetCurrentThreadId()
       LOG_
     START_DEADLOCK_DETECTION( me->m_hUDPProcessPacketThread, "NMPacketManager::UDPReceivePacketThread" );
     ENTER_TIMEOUT

   while( me->m_bUDPProcessPacketThread )
   {
      
      DWORD dwFoo = 0;
      DWORD dwPacketAddr = 0;
      LPOVERLAPPED lpOverlapped = NULL;
      
      ENTER_TIMEOUT
         
      UDPPacket* pPacket=NULL;
      
      BOOL bRet = GetQueuedCompletionStatus( me->m_hUDPReceivePacketIO, &dwFoo, &dwPacketAddr, &lpOverlapped, WAIT_QUEUE_MAX ) ;

      if(bRet)
      {
         LEAVE_TIMEOUT;
         
         pPacket = reinterpret_cast< UDPPacket* >( dwPacketAddr );

			EnterCriticalSection(&me->m_crInterLockCnt);
			me->m_uiNbrInterpQueueCnt--;
  	      LeaveCriticalSection(&me->m_crInterLockCnt);

         try
         {
            me->AnalyzePacket( pPacket );
         }
         catch(...)
         {
            //un probleme est survenu lors de l<analyse du packet...
            _LOG_DEBUG
               LOG_DEBUG_LVL1,
               "*********** TRY CATCH on UDPProcessPacketFct():AnalyzePacket **********"
               LOG_
         }
         me->PacketFree(pPacket);  //UDPProcessPacketFct:Packet a ete recu, traiter et detruit.
         Sleep(0);
      }
   }
   
   STOP_DEADLOCK_DETECTION

   return 0;
}

UINT NMPacketManager::UDPSendPacketFct(LPVOID lpData)
{
   NMPacketManager *me = (NMPacketManager *)lpData;

    CAutoThreadMonitor tmMonitor("NMPacketManager::UDPSendPacketFct");
    _LOG_DEBUG
       LOG_DEBUG_LVL1,
       "UDPSendPacketFct Id=%u",
       GetCurrentThreadId()
       LOG_
    START_DEADLOCK_DETECTION( me->m_hUDPSendPacketThread, "NMPacketManager::UDPSendPacketFct" );
    ENTER_TIMEOUT
      
   
   UDPPacket *pPacket = NULL;
   while( me->m_bUDPSendPacketThread )
   {
      
      DWORD dwFoo               = 0;
      DWORD dwPacketAddr        = 0;
      LPOVERLAPPED lpOverlapped = NULL;
      
      ENTER_TIMEOUT
      
      BOOL bRet = GetQueuedCompletionStatus( me->m_hUDPSendPacketIO, &dwFoo, &dwPacketAddr, &lpOverlapped, WAIT_QUEUE_MAX ) ;
      if(bRet)
      {
         LEAVE_TIMEOUT

         pPacket = reinterpret_cast< UDPPacket* >( dwPacketAddr );
         if(pPacket && !pPacket->boDelete)
         {
            bool bSend = true;
            EnterCriticalSection(&me->m_crConnectionLock);
            try
            {
               CUDPConnection* lpConnection = me->GetConnection( pPacket->sockAddrO ,false);
               if(lpConnection )
               {
                  if(pPacket->ulNbrAck >0)
                  {
                     pPacket->uldwTimeout = timeGetTime() + pPacket->ulAckDelay;
                     if(pPacket->boAddPending)
                     {
                        //a besoin d'un ACK et a forcement un retry...
                        if(!lpConnection->AddPending(pPacket))
                        {
                           //impossible ajouter le pending, on set comme si cetais un pack sans RETRY...
                           pPacket->ulAckDelay           = 0;
                           pPacket->ulNbrAck             = 0;
                           pPacket->pHeader->ushNeedAck  = 0;
                        }
                     }
                  }

                  //Calcule le checksum...
                  if(pPacket->ulBufferLength >= sizeof(PacketHeader))
                     pPacket->pHeader->uchCRC8    = me->CalcChecksumComp2(pPacket->pBuffer,pPacket->ulBufferLength);

                  me->m_pUDP->SendTo(pPacket->sockAddrO,pPacket->sockAddrI,(char*)pPacket->pBuffer,pPacket->ulBufferLength);
               }
               else
                  bSend = false; // no connection exist

               //Si pas de acknoledge, on delete le packet apres l'envoie sinon
               //on le reste vivant le temps de recevoir l'acknoledge...
               if(pPacket->ulNbrAck == 0 || bSend == false)
                  me->PacketFree(pPacket); //SEND Le packet na plus de ACK, on peu le detruire.

            }
            catch (...)
            {
               _LOG_DEBUG
                  LOG_DEBUG_LVL1,
                  "*********** TRY CATCH on UDPSendPacketFct():Send packet and maybe add to pending...**********"
                  LOG_
            }

            LeaveCriticalSection(&me->m_crConnectionLock);
         }
         else if(pPacket)
         {
            if(pPacket->ulBufferLength == 1 && pPacket->pBuffer[0] == 0xF1)
            {
               //AddToLogFile(FALSE,"+++ send WatchDOG\n");
               unsigned short ushNbrUser = CPlayerManager::GetUserCount();
               DWORD dwServerTime = timeGetTime() - pPacket->uldwTimeout;

               pPacket->pBuffer[ 1 ]   = static_cast< BYTE >( dwServerTime >> 24 );
               pPacket->pBuffer[ 2 ]   = static_cast< BYTE >( dwServerTime >> 16 );
               pPacket->pBuffer[ 3 ]   = static_cast< BYTE >( dwServerTime >> 8  );
               pPacket->pBuffer[ 4 ]   = static_cast< BYTE >( dwServerTime );
               pPacket->pBuffer[ 5 ]   = static_cast< BYTE >( ushNbrUser >> 8  );
               pPacket->pBuffer[ 6 ]   = static_cast< BYTE >( ushNbrUser );
               pPacket->ulBufferLength = 7;

               me->m_pUDP->SendTo(pPacket->sockAddrO,pPacket->sockAddrI,(char*)pPacket->pBuffer,pPacket->ulBufferLength);
            }

            me->PacketFree(pPacket); //SEND Le Packet est marquer a DELETE on le detruit sans l envoyer.
         }

         KEEP_ALIVE

         Sleep(0);
      }
   }

   STOP_DEADLOCK_DETECTION
   return 0;
}

UINT NMPacketManager::UDPMaintenancePacket(LPVOID lpData)
{
   NMPacketManager *me = (NMPacketManager *)lpData;

   CAutoThreadMonitor tmMonitor("NMPacketManager::UDPMaintenancePacket");
   _LOG_DEBUG
	   LOG_DEBUG_LVL1,
	   "UDPMaintenancePacket Id=%u",
	   GetCurrentThreadId()
	LOG_

	START_DEADLOCK_DETECTION( me->m_hUDPMaintenancePacket, "NMPacketManager::UDPMaintenancePacket" );
   
   while( me->m_bUDPMaintenancePacket )
   {
      KEEP_ALIVE
      Sleep( 100 );
      
      EnterCriticalSection(&me->m_crConnectionLock);
      try
      {
         KEEP_ALIVE
         
         CUDPConnection * pConnection = NULL;
         vector< sConnectionList >::iterator f;
         for(int i=0; i<me->m_UDPConnectionsList.size();i++)
         {
            if(me->m_UDPConnectionsList[i].pConn)
            {
               // Verify and retransmit safe packets that timedout
               me->m_UDPConnectionsList[i].pConn->VerifyTimedoutPending();
               // Verify and destroy fragmented packets that timedout before getting completed
               me->m_UDPConnectionsList[i].pConn->VerifyTimedoutFragments();
               //Verify if Mega Pack Timeout and need to send...
               me->m_UDPConnectionsList[i].pConn->VerifyMegaPack();

               if (me->m_UDPConnectionsList[i].pConn->ConnectionHasTimedout() == true) 
               {
                  // Send this connection to lost connectio List...
                  // And remove it from the list of active connections
                  me->PostLostConnection(me->m_UDPConnectionsList[i].pConn);

                  f = me->m_UDPConnectionsList.begin();
                  int iCnt = 0;
                  for(int iCnt=0;iCnt<i;iCnt++)
                     f++; 
                  me->m_UDPConnectionsList.erase(f);
                  i--; //on refais le nouvel element
               } 
            }
         }
      }
      catch (...)
      {
         _LOG_DEBUG
            LOG_DEBUG_LVL1,
            "*********** TRY CATCH on UDPMaintenancePacket():Maintenance des packet... **********"
            LOG_
      }
      LeaveCriticalSection(&me->m_crConnectionLock);
   }
   return 0;
}

UINT NMPacketManager::UDPLostConnectionThread(LPVOID lpData)
{
   NMPacketManager *me = (NMPacketManager *)lpData;

   CAutoThreadMonitor tmMonitor("NMPacketManager::UDPLostConnectionThread");
   _LOG_DEBUG
      LOG_DEBUG_LVL1,
      "UDPLostConnectionThread Id=%u",
      GetCurrentThreadId()
      LOG_

   START_DEADLOCK_DETECTION( me->m_hLostConnThread, "NMPacketManager::UDPLostConnectionThread" );

   while( me->m_bLostConnThread )
   {
      KEEP_ALIVE

      DWORD dwFoo = 0;
      DWORD dwPacketAddr = 0;
      LPOVERLAPPED lpOverlapped = NULL;

      CUDPConnection* pConnection = NULL;
      BOOL bRet = FALSE;
      bRet = GetQueuedCompletionStatus( me->m_hLostConnIO, &dwFoo, &dwPacketAddr, &lpOverlapped, WAIT_QUEUE_MAX );
      if(bRet)
      {
         pConnection = reinterpret_cast< CUDPConnection* >( dwPacketAddr );
         if(pConnection)
         {
            delete pConnection;
            pConnection = NULL;
         }
         Sleep(100);
      }
   }

   return 0;
}

UINT NMPacketManager::OneShotEnableCommThread(LPVOID lpData)
{
   NMPacketManager *me = (NMPacketManager *)lpData;

   while(me->m_bCanStartComm == FALSE)
   {
		Sleep(1000);
   }
   
   me->m_bServerStarted  = TRUE;

   return 0;
}



void NMPacketManager::SendUDPPacket( sockaddr_in sockAddrO,sockaddr_in sockAddrI, unsigned char* pBuffer, int nBufferSize, 
                                DWORD dwAckDelay, DWORD dwMaxAck, bool bCompress,bool bAutoCompress,bool bSendDirect)
{
 

   //on check si le packet doit etre compresser en premier...
   //si oui on le compresse car une fois compresser il est possible
   //quil entre dans un fragment

   bool  bCompressed = false;
   if(bAutoCompress)
   {
      if(nBufferSize > 2000) 
         bCompressed = true;
   }
   else
   {
      bCompressed = bCompress;
   }

   if(theApp.dwEnableCOMMCompression == 0)
      bCompressed = false; //le systeme de compression ets desactiver on compresse rien...

   BYTE *pCompressData = NULL;

   BYTE *pFinalData;
   DWORD dwFinalLength;

   PacketHeaderComp CompHeader;

   if(bCompressed)
   {
      ULONG dwTaille  = nBufferSize;
      ULONG dwTailleC = MEM_COMPRESS_NEED(nBufferSize);

      pCompressData = new BYTE[dwTailleC];
      if( compress(pCompressData, &dwTailleC, pBuffer, dwTaille) != Z_OK )
      {
         //ADD Log :Impossible de compresser on sort de ici le pack ne dois pas etre bon...
         if(pCompressData)
            delete []pCompressData;
         pCompressData = NULL;
         return;
      }

      //on valide que le buffer compresser est bien plus petit que le buffer original, si
      //ce n'est pas le cas, on enverra le buffer NON compresser
      if(dwTailleC < dwTaille)
      {
         //on utilise les data compressed...
         CompHeader.ulPackData   = dwTailleC;
         CompHeader.ulUnpackData = dwTaille;

         pFinalData    = pCompressData;
         dwFinalLength = dwTailleC;

      }
      else
      {
         bCompressed = false;
         pFinalData    = pBuffer;
         dwFinalLength = nBufferSize;
      }
   }
   else
   {
      pFinalData    = pBuffer;
      dwFinalLength = nBufferSize;
   }

   EnterCriticalSection(&m_crConnectionLock);
   try
   {
      CUDPConnection* lpConnection = GetConnection( sockAddrO ,false);
      if(lpConnection)
      {
         if(dwFinalLength <= PACKET_DATA_MAX) //None splitted packet...
         {
            unsigned short ushPackID = 0;
            ushPackID = lpConnection->GetPacketID();


            int iPacketLength = dwFinalLength+sizeof(PacketHeader);
            if(bCompressed)
               iPacketLength += sizeof(PacketHeaderComp);


            UDPPacket* pPacket = PacketAlloc(iPacketLength);

            pPacket->boAddPending = TRUE; // if ya des ack on enverra le pending...

            pPacket->puchData  = pPacket->pBuffer + sizeof(PacketHeader);
            if(bCompressed)
            {
               pPacket->pHeaderComp = (PacketHeaderComp*)(pPacket->pBuffer+sizeof(PacketHeader));
               pPacket->puchData += sizeof(PacketHeaderComp);
            }
            pPacket->ulBufferLength = iPacketLength;
            pPacket->ulAckDelay     = dwAckDelay;
            pPacket->ulNbrAck       = dwMaxAck;

            //fill header struct
            pPacket->sockAddrI              = sockAddrI; 
            pPacket->sockAddrO              = sockAddrO; 
            pPacket->pHeader->ushID         = ushPackID;
            pPacket->pHeader->ushNeedAck    = 0;
            pPacket->pHeader->ushCompress   = 0;
            pPacket->pHeader->ushSplit      = 0;
            pPacket->pHeader->uchPartNumber = 0;

            if(pPacket->ulNbrAck >0)
            {
               pPacket->pHeader->ushNeedAck  = 1;
               

               if(nBufferSize >4 && !bCompressed)
               {
                  USHORT ushOFFSET = Reverse16(*(short *)&pBuffer[4]);
                  if(ushOFFSET < 0xFFFF)
                     m_pdwNbrPacketSendbyPacket[ushOFFSET]++;
               } 
            }

            if(bCompressed)
            {
               // fill compressed struct
               pPacket->pHeader->ushCompress = 1;
               pPacket->pHeaderComp->ulPackData   = CompHeader.ulPackData;
               pPacket->pHeaderComp->ulUnpackData = CompHeader.ulUnpackData;
            }
       
            pPacket->ulDataLength = dwFinalLength;
            memcpy( pPacket->puchData, pFinalData, dwFinalLength );
           
            //Send the packet to Sending queue
            m_dwNbrPacketSend++;
            //si un send direct ou que pack > 300 bytes ou que le system de megapack est a OFF, on shoot direct
            if(bSendDirect || pPacket->ulBufferLength >300 || theApp.dwEnableCOMMMegaPack == 0) //we not pack packet over 300 bytes...
            {
               PostSendPacket(pPacket); //Send direct...
            }
            else
            {
               lpConnection->AddPacketToMegaPack(pPacket);
               PacketFree(pPacket); //on delete ce packet anyway il est soit copier 
            }
         }
         else
         {
            //Split packet...
            //On dois creer des parties de packet et les envoyer...
            int numberOfPieces = (dwFinalLength / PACKET_DATA_MAX) + 1;
            if (numberOfPieces <= PACKET_DATA_MAX_SPLIT_PART) 
            {
               unsigned short ushPackID[PACKET_DATA_MAX_SPLIT_PART];
               unsigned short packetSplitNO = 0;
               int i=0;
               for(i = 0; i < numberOfPieces; ++i )
                  ushPackID[i] = lpConnection->GetPacketID();
               packetSplitNO = lpConnection->GetPacketSplitID();

               // For all pieces...
               for(i = 0; i < numberOfPieces; ++i )
               {
                  int pieceSize = 0;
                  if( i == numberOfPieces - 1 )  // If this is the last piece
                  {
                     pieceSize = dwFinalLength % PACKET_DATA_MAX;
                  } 
                  else
                  {
                     pieceSize = PACKET_DATA_MAX;
                  }

                  int iPacketLength = pieceSize + sizeof(PacketHeader) + sizeof(PacketHeaderSplit);
                  if(bCompressed)
                     iPacketLength += sizeof(PacketHeaderComp);

                  UDPPacket* pPacket = PacketAlloc(iPacketLength);

                  pPacket->boAddPending = TRUE; // if ya des ack on enverra le pending...
                  
                  pPacket->pHeaderSplit = (PacketHeaderSplit*)(pPacket->pBuffer+sizeof(PacketHeader));
                  pPacket->puchData  = pPacket->pBuffer + sizeof(PacketHeader) + sizeof(PacketHeaderSplit);
                  if(bCompressed)
                  {
                     pPacket->pHeaderComp = (PacketHeaderComp*)(pPacket->pBuffer+sizeof(PacketHeader)+sizeof(PacketHeaderSplit));
                     pPacket->puchData += sizeof(PacketHeaderComp);
                  }

                  pPacket->ulBufferLength = iPacketLength;

                  memcpy( pPacket->puchData, pFinalData + (i*PACKET_DATA_MAX), pieceSize);
                  pPacket->ulDataLength = pieceSize;
                  pPacket->ulAckDelay     = dwAckDelay;
                  pPacket->ulNbrAck       = dwMaxAck;

                  //fill header struct
                  pPacket->sockAddrI              = sockAddrI; 
                  pPacket->sockAddrO              = sockAddrO; 
                  pPacket->pHeader->ushID         = ushPackID[i];
                  pPacket->pHeader->ushNeedAck    = 0;
                  pPacket->pHeader->ushCompress   = 0;
                  pPacket->pHeader->ushSplit      = 1;
                  pPacket->pHeader->uchPartNumber = i;
                  
                  if(pPacket->ulNbrAck >0)
                     pPacket->pHeader->ushNeedAck  = 1;

                  //Fill the split structure
                  pPacket->pHeaderSplit->uchSplitNO  = packetSplitNO;
                  pPacket->pHeaderSplit->uchPartNbr  = i;
                  pPacket->pHeaderSplit->uchMaxPart  = numberOfPieces;
                  pPacket->pHeaderSplit->ushDataSize = (unsigned short)dwFinalLength;
                  
                  if(bCompressed)
                  {
                     // fill compressed struct
                     pPacket->pHeader->ushCompress = 1;
                     pPacket->pHeaderComp->ulPackData   = CompHeader.ulPackData;
                     pPacket->pHeaderComp->ulUnpackData = CompHeader.ulUnpackData;
                  }
                  
                  //Send the packet to Sending queue
                  m_dwNbrPacketSend++;
                  PostSendPacket(pPacket); //packet fragmenter on send direct...
               }
            }
         }
      }
   }
   catch (...)
   {
      _LOG_DEBUG
         LOG_DEBUG_LVL1,
         "*********** TRY CATCH on SendUDPPacket():Send packet all king... **********"
         LOG_
   }
   LeaveCriticalSection(&m_crConnectionLock);
   if(pCompressData)
      delete []pCompressData;
   pCompressData = NULL;
}


void NMPacketManager::SendAckPacket(UDPPacket* pPacket) 
{
   UDPPacket* pPacketAck = PacketAlloc(sizeof(PacketHeader));

   pPacketAck->ulBufferLength  = sizeof(PacketHeader);
      
   // The ack's ID is the ID of the packet it acknowledges.
   // And no flags :)
   pPacketAck->pHeader->ushID         = pPacket->pHeader->ushID;
   pPacketAck->pHeader->ushNeedAck    = 0;
   pPacketAck->pHeader->ushCompress   = 0;
   pPacketAck->pHeader->ushSplit      = 0;
   pPacketAck->pHeader->uchCRC8       = 0x00;
   pPacketAck->pHeader->uchPartNumber = pPacket->pHeader->uchPartNumber;
   // Send packet back to its destination
   pPacketAck->sockAddrI = pPacket->sockAddrI;   //copie le socket de celui qui a transmit
   pPacketAck->sockAddrO = pPacket->sockAddrO;   //copie le socket de celui qui a transmit
   PostSendPacket(pPacketAck); //send direct les ACK...
}



void NMPacketManager::AnalyzePacket(UDPPacket *pPacket)
{
   //On valide si ce packet a deja ete processer ou pas, car on ne process pas 2 fois le meme packet...
   bool bAlreadyProcessed = false;
   EnterCriticalSection(&m_crConnectionLock);
   try
   {
      CUDPConnection* lpConnection = GetConnection( pPacket->sockAddrO ,false);
      if(lpConnection && lpConnection->AlreadyReceivedPacket(pPacket->pHeader->ushID) == false )
      {
         lpConnection->RegisterReceivedPacketID(pPacket->pHeader->ushID);
         m_dwNbrPacketRecv++;
      }
      else
      {
         m_dwNbrPacketRecvAlreadyRegistred++;
         bAlreadyProcessed = true;
      }
   }
   catch (...)
   {
      //une erreur est survenu dans la validation, on rejete ce packet...
      bAlreadyProcessed = true;
      _LOG_DEBUG
         LOG_DEBUG_LVL1,
         "*********** TRY CATCH on AnalyzePacket():valid if packet already exist...**********"
         LOG_
   }
   LeaveCriticalSection(&m_crConnectionLock);
   if(bAlreadyProcessed)
      return;



   //rendu ici on a un packet complet compresser ou non...
   //on peu donc appeler le callback avec les donnee de ce 
   //packet pour terminer par supprimer ce packet...

   if(pPacket->pHeader->ushCompress)
   {
      if(pPacket->pHeaderComp->ulUnpackData < 0x00800000) //8 megs
      {
         //on alloue un buffer temporaire pour decompresset le ZIP...
         BYTE *pDataUnpack = new BYTE[pPacket->pHeaderComp->ulUnpackData];
      

         if(uncompress((BYTE*)pDataUnpack,&pPacket->pHeaderComp->ulUnpackData,(BYTE*)pPacket->puchData,pPacket->pHeaderComp->ulPackData) == 0)
         {
            //AddToLogFile(TRUE,"-> Analyse COmpressed Data: PACK = %d, UNPACK = %d   ==GAIN = %d bytes\n",pPacket->pHeaderComp->ulPackData,pPacket->pHeaderComp->ulUnpackData,pPacket->pHeaderComp->ulUnpackData-pPacket->pHeaderComp->ulPackData);
            //AddToLogFile(TRUE,"-> Analyse Data: ID = %d\n",pPacket->pHeader->ushID);
            if(m_pfProcessCallback)
               m_pfProcessCallback( m_pParent,pPacket->sockAddrO,pPacket->sockAddrI, pDataUnpack, pPacket->pHeaderComp->ulUnpackData,0);
         }
         else
         {
            //AddToLogFile(TRUE,"-> Unable to undompress packet\n");
         }

         if(pDataUnpack)
            delete []pDataUnpack;
         pDataUnpack = NULL;
      }
      else
      {
         //ADD LOG:  taille compresser invalide...
      }

   }
   else
   {
      try
      {
		  if(pPacket->ulDataLength >4)
        {
           USHORT ushOFFSET = Reverse16(*(short *)&pPacket->puchData[4]);
           if(ushOFFSET < 0xFFFF)
              m_pdwNbrPacketRecvbyPacket[ushOFFSET]++;
        }
        //le serveur ne devrait jamais recevoir de mega pack...
        //ce truc est uniquement Du SERVEUR -->  Client...
        if(m_pfProcessCallback)
           m_pfProcessCallback( m_pParent,pPacket->sockAddrO,pPacket->sockAddrI, pPacket->puchData, pPacket->ulDataLength,0);
      }
      catch(...)
      {
         _LOG_DEBUG
            LOG_DEBUG_LVL1,
            "*********** TRY CATCH on AnalyzePacket():Callbackdata**********"
            LOG_
      }
   }
}


UDPPacket *NMPacketManager::PacketAlloc(int size)
{
   int iErr = 1;
   UDPPacket *packet = new UDPPacket;
   if (packet) 
   {
      packet->uldwTimeoutMegaT= 0x00000000;
      packet->uldwTimeoutMegaE= 0x00000000;
      packet->uldwTimeout     = 0xFFFFFFFF;
      packet->pBuffer = new unsigned char[size];
      if (packet->pBuffer) 
      {

         packet->boDelete       = FALSE;
         packet->boAckReceived  = FALSE;
         packet->boAddPending   = FALSE;
         packet->ulBufferLength = 0;
         packet->pHeader        = (PacketHeader*)packet->pBuffer;
         packet->pHeaderSplit   = NULL;
         packet->pHeaderComp    = NULL;
         packet->puchData       = NULL;
         packet->ulDataLength   = 0;
         iErr = 0;
      }
   }

   if(iErr)
   {
      PacketFree(packet); //ALLOC  Ce packet c'est mal alouer donc detruit.
      packet = NULL;
   }
   return(packet);
}

void NMPacketManager::PacketFree(UDPPacket *packet)
{
   if ( packet ) 
   {
      if ( packet->pBuffer )
      {
         delete []packet->pBuffer;
         packet->pBuffer = NULL;
      }
      delete packet; 
      packet = NULL;
   }
}

void NMPacketManager::PacketAllocAndCopy( UDPPacket *pNPacket,UDPPacket *packet)
{
   pNPacket->sockAddrO = packet->sockAddrO;
   pNPacket->sockAddrI = packet->sockAddrI;
   memcpy(pNPacket->pBuffer,packet->pBuffer,packet->ulBufferLength);
   pNPacket->ulBufferLength    = packet->ulBufferLength;
   pNPacket->ulDataLength      = packet->ulDataLength;
   pNPacket->boDelete          = packet->boDelete;
   pNPacket->boAckReceived     = packet->boAckReceived;
   pNPacket->uldwTimeout       = packet->uldwTimeout;
   pNPacket->uldwTimeoutMegaE  = packet->uldwTimeout;
   pNPacket->uldwTimeoutMegaT  = packet->uldwTimeout;
   pNPacket->ulAckDelay        = packet->ulAckDelay;
   pNPacket->ulNbrAck          = packet->ulNbrAck;
   pNPacket->boAddPending      = packet->boAddPending;

   //Copie le header...
   pNPacket->pHeader->ushID         = packet->pHeader->ushID;
   pNPacket->pHeader->ushNeedAck    = packet->pHeader->ushNeedAck;
   pNPacket->pHeader->ushCompress   = packet->pHeader->ushCompress;
   pNPacket->pHeader->ushSplit      = packet->pHeader->ushSplit;
   pNPacket->pHeader->uchCRC8       = packet->pHeader->uchCRC8;
   pNPacket->pHeader->uchPartNumber = packet->pHeader->uchPartNumber;
   pNPacket->puchData     = (pNPacket->pBuffer+sizeof(PacketHeader));

   if(pNPacket->pHeader->ushSplit)
   {
      pNPacket->pHeaderSplit = (PacketHeaderSplit*)(pNPacket->pBuffer+sizeof(PacketHeader));
      pNPacket->pHeaderSplit->uchSplitNO  = packet->pHeaderSplit->uchSplitNO;
      pNPacket->pHeaderSplit->uchPartNbr  = packet->pHeaderSplit->uchPartNbr;
      pNPacket->pHeaderSplit->uchMaxPart  = packet->pHeaderSplit->uchMaxPart;
      pNPacket->pHeaderSplit->ushDataSize = packet->pHeaderSplit->ushDataSize;
      pNPacket->puchData     = (pNPacket->pBuffer+sizeof(PacketHeader)+sizeof(PacketHeaderSplit));
   }
   if(pNPacket->pHeader->ushCompress)
   {
      if(pNPacket->pHeader->ushSplit)
      {
         pNPacket->pHeaderComp = (PacketHeaderComp*)(pNPacket->pBuffer+sizeof(PacketHeader)+sizeof(PacketHeaderSplit));
         pNPacket->puchData     = (pNPacket->pBuffer+sizeof(PacketHeader)+sizeof(PacketHeaderSplit)+sizeof(PacketHeaderComp));
      }
      else
      {
         pNPacket->pHeaderComp = (PacketHeaderComp*)(pNPacket->pBuffer+sizeof(PacketHeader));
         pNPacket->puchData     = (pNPacket->pBuffer+sizeof(PacketHeader)+sizeof(PacketHeaderComp));
      }

      pNPacket->pHeaderComp->ulUnpackData = packet->pHeaderComp->ulUnpackData;
      pNPacket->pHeaderComp->ulPackData   = packet->pHeaderComp->ulPackData;
   }


}

unsigned char  NMPacketManager::CalcChecksumComp2(unsigned char	*pData, unsigned long dwNbrData)
{
   if(!pData || dwNbrData <=0)
      return 0x00; // pas de données a calculer
   
   unsigned char chkSum;
   unsigned char sumByte = 0xAA;
   // on additionne tous les BYTES
   for(unsigned long i=0;i<dwNbrData;i++)
   {
      if(i != 2) //Skip le checksum bytes...
         sumByte = sumByte+ *(unsigned char *)&pData[i];
   }
   
   chkSum = 0x100 - (sumByte);
   
   return chkSum;
}

bool NMPacketManager::IsChksumOK(unsigned char *pData, unsigned long dwNbrData,unsigned char uchCheckSum)
{
   if(!pData || dwNbrData <=0 || dwNbrData > PACKET_MAX_SIZE)
      return false; // pas de données a calculer

   unsigned char sumByte = 0xAA;
   // on additionne tous les BYTES
   for(unsigned long i=0;i<dwNbrData;i++)
   {
      if(i != 2) //Skip le checksum bytes...
         sumByte = sumByte+ *(unsigned char *)&pData[i];
   }
   unsigned char uchVal = sumByte+uchCheckSum;
   if(uchVal)
      return false;

   return true;
}



// Put the packet on the queue for receiving
inline void NMPacketManager::PostReceivePacket (UDPPacket* pPacket)
{
   EnterCriticalSection(&m_crInterLockCnt);
   m_uiNbrInterpQueueCnt++;
   LeaveCriticalSection(&m_crInterLockCnt);

   PostQueuedCompletionStatus( m_hUDPReceivePacketIO, 0, reinterpret_cast< DWORD >( pPacket ), NULL );
}

inline void NMPacketManager::PostSendPacket (UDPPacket* pPacket) 
{
   PostQueuedCompletionStatus( m_hUDPSendPacketIO, 0, reinterpret_cast< DWORD >( pPacket ), NULL );
}

inline void NMPacketManager::PostSendPacketMega (UDPPacket* pPacket) 
{
   //Allocate new packet here and stay megapack clean...
   UDPPacket *pMegaPack = PacketAlloc(PACKET_MAX_SIZE);
   PacketAllocAndCopy(pMegaPack,pPacket);
   PostQueuedCompletionStatus( m_hUDPSendPacketIO, 0, reinterpret_cast< DWORD >( pMegaPack ), NULL );
}

inline void NMPacketManager::PostLostConnection  (CUDPConnection* pConnection)
{
   PostQueuedCompletionStatus( m_hLostConnIO, 0, reinterpret_cast< DWORD >( pConnection ), NULL );
}

int NMPacketManager::GetIntrQueueSize()
{
   return m_uiNbrInterpQueueCnt;
}

vector< sockaddr_in > *NMPacketManager::GetLostConnections( void )
{
   if(m_pUDP)
   {
	   return m_pUDP->GetLostConnections();
   }

   return NULL;
}
void NMPacketManager::FreeLostConnections(BOOL boFlushList)
{
   if(m_pUDP)
   {
	   m_pUDP->FreeLostConnections(boFlushList);
   }
}

//---------- START   OF CUDPConnection METHODS ------------------
CUDPConnection::CUDPConnection()
{
    m_pNMPacketManager = NULL;
    m_ushCurrentID      = 0;
    m_ushCurrentSplitID = 0;
    m_ushOffsetID       = 0;
    
    for(int i=0;i<NBR_RECEIVED_REGISTRED_ID;i++)
       m_lReceivedPacketID[i] = -1;
    
    m_dwConnectionTimeout = timeGetTime() + PACKET_BACKLOG_TIMEOUT;

    m_pMegaPack = NULL;
 
    InitializeCriticalSection(&m_crPendingLock);
    InitializeCriticalSection(&m_crSplitLock);
    InitializeCriticalSection(&m_crMegaPackLock);
}

CUDPConnection::~CUDPConnection( void )
{

    DestroyAllPending();

    if(m_pNMPacketManager)
      m_pNMPacketManager->PacketFree(m_pMegaPack);
 
    DeleteCriticalSection(&m_crPendingLock);
    DeleteCriticalSection(&m_crSplitLock);
    DeleteCriticalSection(&m_crMegaPackLock);

}

int CUDPConnection::InitializeComm(const sockaddr_in &sAddr, NMPacketManager *pNMPacketManager)
{
   m_sockAddr         = sAddr;
   m_pNMPacketManager = pNMPacketManager;
  
   return 0;
}

void CUDPConnection::ResetTimeout()
{
   m_dwConnectionTimeout = timeGetTime() + PACKET_BACKLOG_TIMEOUT;
}

inline WORD CUDPConnection::GetPacketID() 
{
   // Get Packet ID
   ++m_ushCurrentID;
   if ( m_ushCurrentID  >= PACKET_ID_MAX) 
   {
      m_ushCurrentID = 0;
   }
   return m_ushCurrentID;
}

inline WORD CUDPConnection::GetPacketSplitID() 
{
   // Get Packet ID
   ++m_ushCurrentSplitID;
   if ( m_ushCurrentSplitID  >= NBR_PACKET_SPLIT_MAX) 
   {
      m_ushCurrentSplitID = 0;
   }
   return m_ushCurrentSplitID;
}


inline void CUDPConnection::RegisterReceivedPacketID(unsigned short ushPacketID) 
{
   // Register the packet id
   m_lReceivedPacketID[m_ushOffsetID] = ushPacketID;

   m_ushOffsetID++;
   if (m_ushOffsetID >= NBR_RECEIVED_REGISTRED_ID) 
       m_ushOffsetID = 0; // We reached the end. Restart.
}

inline bool CUDPConnection::AlreadyReceivedPacket(unsigned short ushPacketID) 
{
   int iEnd = m_ushOffsetID;
   int iCnt = m_ushOffsetID;
   do
   {
      iCnt--;
      if (iCnt < 0) 
         iCnt = NBR_RECEIVED_REGISTRED_ID - 1;

      if (m_lReceivedPacketID[iCnt] == ushPacketID) 
         return true;

   }while (iCnt != iEnd);
   return false;
}

inline bool CUDPConnection::AddPending(UDPPacket* pPending) 
{
   bool bAdd = true;
   EnterCriticalSection(&m_crPendingLock);
   if (m_pendingPackets.size() < NBR_PACKET_PENDING_MAX) 
   {
      bAdd = true;
      m_pendingPackets.push_back(pPending);
   } 
   else 
   {
      bAdd = false;
   }
   LeaveCriticalSection(&m_crPendingLock);

   return bAdd;
}

bool CUDPConnection::DestroyPending (WORD packetID) 
{
   if(!m_pNMPacketManager)
      return false;

   EnterCriticalSection(&m_crPendingLock);
   vector<UDPPacket*>::iterator i = m_pendingPackets.begin();
   while (i != m_pendingPackets.end()) 
   {
      if ((*i)->pHeader->ushID == packetID) 
      {
         (*i)->boAckReceived = TRUE;
         LeaveCriticalSection(&m_crPendingLock);
         return true;
      }
      ++i;
   }
   LeaveCriticalSection(&m_crPendingLock);
   return false;
}

void CUDPConnection::DestroyAllPending() 
{
   if(!m_pNMPacketManager)
      return ;

   vector<UDPPacket*>::iterator iP = m_pendingPackets.begin();
   while ( iP != m_pendingPackets.end() ) 
   {
      UDPPacket* pPacket = (*iP);
      m_pNMPacketManager->PacketFree(pPacket);
      ++iP;
   }
   m_pendingPackets.clear();

   //destroy all fragment
   for (int iFrag=0; iFrag < m_FragmentsPackets.size();iFrag++) 
      m_pNMPacketManager->PacketFree(m_FragmentsPackets[iFrag].pPak);
   m_FragmentsPackets.clear();
}

inline bool CUDPConnection::ConnectionHasTimedout() 
{
   if (m_dwConnectionTimeout <= timeGetTime()) 
   {
      if ( m_pendingPackets.empty()  && m_FragmentsPackets.empty()) 
      {
         return true;
      }
   }
   return false;
}

inline bool CUDPConnection::AddPacketFragment(UDPPacket* pFragment)
{
   if(!m_pNMPacketManager)
      return false;
   
   bool bAdd = true;
   EnterCriticalSection(&m_crSplitLock);
   if (m_FragmentsPackets.size() < NBR_PACKET_SPLIT_MAX) 
   {
      // on valide que le packet n'est pas deja la...
      vector<UDPPacketSplit>::iterator itF = m_FragmentsPackets.begin();
      for(UINT i=0;i<m_FragmentsPackets.size();i++,itF++)
      {
         if(m_FragmentsPackets[i].pPak->pHeaderSplit->uchSplitNO == pFragment->pHeaderSplit->uchSplitNO)
         {
            int iPos = pFragment->pHeaderSplit->uchPartNbr*PACKET_DATA_MAX;
            if(iPos+pFragment->ulDataLength <= m_FragmentsPackets[i].pPak->ulDataLength)
            {
               m_FragmentsPackets[i].pPak->pHeaderSplit->uchPartNbr++;
               memcpy(m_FragmentsPackets[i].pPak->puchData+iPos,pFragment->puchData,pFragment->ulDataLength);
               m_FragmentsPackets[i].lPartReceive.push_back(pFragment->pHeaderSplit->uchPartNbr);

               //on valide si on a recu tous les fragment d<un packet...
               if(m_FragmentsPackets[i].pPak->pHeaderSplit->uchPartNbr == m_FragmentsPackets[i].pPak->pHeaderSplit->uchMaxPart)
               {
                  //on a recu tous les fragment de ce packet...on peu deleter de la liste ce packet
                  //et envoyer le PACK au receive packet complet...
                  UDPPacket *pNewComplete = m_FragmentsPackets[i].pPak;
                  itF = m_FragmentsPackets.erase(itF);

                  m_pNMPacketManager->PostReceivePacket(pNewComplete);
               }
            }
            
            LeaveCriticalSection(&m_crSplitLock);
            return bAdd;
         }
      }

      //le packet existe pas, c<est le premier fragment que l'on recoit...
      //on cree un packet temporaire qui contiendra toutes les donnee deu packet...
      int iTotalSize = pFragment->pHeaderSplit->ushDataSize+sizeof(PacketHeader)+sizeof(PacketHeaderSplit);

      if(pFragment->pHeader->ushCompress)
         iTotalSize += sizeof(PacketHeaderComp);

      UDPPacketSplit NewSplit;
      NewSplit.pPak = m_pNMPacketManager->PacketAlloc(iTotalSize);
      NewSplit.pPak->ulBufferLength = iTotalSize;
      NewSplit.pPak->puchData       = NewSplit.pPak->pBuffer + sizeof(PacketHeader) + sizeof(PacketHeaderSplit);
      NewSplit.pPak->ulDataLength   = pFragment->pHeaderSplit->ushDataSize;  

      NewSplit.pPak->pHeaderSplit = (PacketHeaderSplit*)(NewSplit.pPak->pBuffer+sizeof(PacketHeader));

      NewSplit.pPak->pHeader->ushCompress = pFragment->pHeader->ushCompress;
      NewSplit.pPak->pHeader->ushID       = pFragment->pHeader->ushID;
      NewSplit.pPak->pHeader->ushNeedAck  = 0;

      if(pFragment->pHeader->ushCompress)
      {
         NewSplit.pPak->pHeaderComp = (PacketHeaderComp*)(NewSplit.pPak->pBuffer+sizeof(PacketHeader)+sizeof(PacketHeaderSplit));
         NewSplit.pPak->puchData += sizeof(PacketHeaderComp);

         NewSplit.pPak->pHeaderComp->ulPackData   = pFragment->pHeaderComp->ulPackData;
         NewSplit.pPak->pHeaderComp->ulUnpackData = pFragment->pHeaderComp->ulUnpackData;
      }

      NewSplit.pPak->pHeaderSplit->uchSplitNO  = pFragment->pHeaderSplit->uchSplitNO;
      NewSplit.pPak->pHeaderSplit->uchPartNbr  = 1;
      NewSplit.pPak->pHeaderSplit->uchMaxPart  = pFragment->pHeaderSplit->uchMaxPart;
      NewSplit.pPak->pHeaderSplit->ushDataSize = pFragment->pHeaderSplit->ushDataSize;

      //copie la partie de data dans le gros bufer now...
      int iPos = pFragment->pHeaderSplit->uchPartNbr*PACKET_DATA_MAX;
      memcpy(NewSplit.pPak->puchData+iPos,pFragment->puchData,pFragment->ulDataLength);

      NewSplit.pPak->sockAddrO = pFragment->sockAddrO;
      NewSplit.pPak->sockAddrI = pFragment->sockAddrI;
      NewSplit.pPak->uldwTimeout = timeGetTime() + PACKET_SPLIT_TIMEOUT;
      NewSplit.lPartReceive.push_back(pFragment->pHeaderSplit->uchPartNbr);
      m_FragmentsPackets.push_back(NewSplit);
   } 
   else 
   {
      bAdd = false;
      //ADD Log : le pending ets plein... etrange, mais on en mets plus dans la liste...
   }
   LeaveCriticalSection(&m_crSplitLock);
   
   return bAdd;
}

inline void CUDPConnection::VerifyTimedoutPending()
{
   if(!m_pNMPacketManager)
      return;

   UDPPacket* pResentPending = NULL;
   DWORD dwCurrentTime = timeGetTime();
   
      
   EnterCriticalSection(&m_crPendingLock);


   vector<UDPPacket*>::iterator i = m_pendingPackets.begin();
   while ( i != m_pendingPackets.end() ) 
   {
      if ( (*i)->uldwTimeout <= dwCurrentTime ) 
      {
         pResentPending = *i;
         if(pResentPending)
         {
            // It timed out, lets get it ready for retransmission!
            pResentPending->uldwTimeout = 0xFFFFFFFF;
            pResentPending->ulNbrAck--;
            pResentPending->boAddPending = FALSE; //les retry on ajoute jamnais au pending...
            if(pResentPending->ulNbrAck == 0 || pResentPending->boDelete || pResentPending->boAckReceived)
            { 
               if(!pResentPending->boDelete && !pResentPending->boAckReceived)
               {
                  m_pNMPacketManager->m_dwGlobalNbrLost++;

               }

               pResentPending->ulAckDelay  = 0;
               pResentPending->boDelete = TRUE;
               pResentPending->pHeader->ushNeedAck = 0; // plus d'ack...

               //on delete ce pack du pending et on set le delete a TRUS donc sera detruit et pas envoyer
               vector<UDPPacket*>::iterator iDelete(i);
               i = m_pendingPackets.erase(iDelete);

               //on delete de la liste de pending, mais on ne delete pas le packet le SEND s<en chargera,...
               //car on set le packet avec PAS de RETRY...
            } 
            else
            {
               m_pNMPacketManager->m_dwGlobalNbrRetry++;
               pResentPending->ulAckDelay += (pResentPending->ulAckDelay/4); //add 25% of delay to timeout if connection are slow...
               ++i;
            }
            
            
            //resent this packet ID... and keep it in pending list
            m_pNMPacketManager->PostSendPacket(pResentPending);//resend direct le spacket retry...
         }
         else
         {
            ++i;
         }
      } 
      else 
      {
         ++i;
      }
   }
   LeaveCriticalSection(&m_crPendingLock);
}

inline void CUDPConnection::VerifyTimedoutFragments()
{
   if(!m_pNMPacketManager)
      return;
   
   DWORD dwCurrentTime = timeGetTime();
   
   EnterCriticalSection(&m_crSplitLock);
   
   
   vector<UDPPacketSplit>::iterator itF = m_FragmentsPackets.begin();
   while ( itF != m_FragmentsPackets.end() ) 
   {
      if ( (*itF).pPak->uldwTimeout <= dwCurrentTime ) 
      {
         UDPPacket *pNewComplete = (*itF).pPak;
         m_pNMPacketManager->PacketFree(pNewComplete);
         itF = m_FragmentsPackets.erase(itF);
      } 
      else 
      {
         ++itF;
      }
   }
   LeaveCriticalSection(&m_crSplitLock);
}



inline bool CUDPConnection::AddPacketToMegaPack(UDPPacket* pPack)
{
   if(!m_pNMPacketManager)
      return false;

   EnterCriticalSection(&m_crMegaPackLock);


   //on regarde si on peu ajouter ou pas...
   //si le pack est full on send le pack courant et on vas en creer un nouveau...
   if(m_pMegaPack && m_pMegaPack->ulBufferLength + pPack->ulDataLength+2 >= PACKET_DATA_MAX)
   {
      //le packet courant est plein...
      //AddToLogFile(FALSE,"Send megaPack SIZE");

      //CV14: Add to megapack send and clear the megapack here...
      m_pNMPacketManager->PostSendPacket(m_pMegaPack); 
      //m_pNMPacketManager->PostSendPacketMega(m_pMegaPack); 
      //m_pNMPacketManager->PacketFree(m_pMegaPack);

      m_pMegaPack = NULL;//on delete pas, le pointeur dois rester en vie on fait que resetter le ptr local...
   }
 
   if(m_pMegaPack == NULL)
   {
      //On dois creer le packet
      m_pMegaPack = m_pNMPacketManager->PacketAlloc(PACKET_MAX_SIZE);
      m_pMegaPack->puchData  = m_pMegaPack->pBuffer + sizeof(PacketHeader);

      m_pMegaPack->ulBufferLength = sizeof(PacketHeader)+4; //header + megapack headerID +nbrPart
      m_pMegaPack->ulDataLength   = 4;
      m_pMegaPack->ulAckDelay     = 1000;
      m_pMegaPack->ulNbrAck       = 2;
      m_pMegaPack->boAddPending   = TRUE;

      //fill header struct
      m_pMegaPack->sockAddrO              = pPack->sockAddrO; 
      m_pMegaPack->sockAddrI              = pPack->sockAddrI;
      m_pMegaPack->pHeader->ushID         = pPack->pHeader->ushID;
      m_pMegaPack->pHeader->ushNeedAck    = 1;
      m_pMegaPack->pHeader->ushCompress   = 0;
      m_pMegaPack->pHeader->ushSplit      = 0;
      m_pMegaPack->pHeader->uchPartNumber = 0;

      unsigned short ushNbrPack = 0;
      unsigned short ushMegaID  = 0xFFFF;
      *(unsigned short  *)&m_pMegaPack->puchData[ 0]  = Reverse16(ushMegaID);
      *(unsigned short  *)&m_pMegaPack->puchData[ 2]  = Reverse16(ushNbrPack);

      m_pMegaPack->uldwTimeoutMegaT = timeGetTime()+250; //1 second maximum for mega pack construction...
   }

   //ici on est sure de pouvoir ajouter
   //recupere le nbr de partie
   unsigned short  ushNbrPack = Reverse16(*(short *)&m_pMegaPack->puchData[2]);
   ushNbrPack++;
   //reecris le bon nombre de part...
   *(unsigned short  *)&m_pMegaPack->puchData[ 2]  = Reverse16(ushNbrPack);

   //maintenant ecrit la part dans le mega pack...
   unsigned char *pPosData = m_pMegaPack->puchData+m_pMegaPack->ulDataLength;
   *(unsigned short  *)&pPosData[0]  = Reverse16(pPack->ulDataLength); //write the datalen...
   pPosData+=2;

   memcpy(pPosData,pPack->puchData,pPack->ulDataLength);
   m_pMegaPack->ulBufferLength = m_pMegaPack->ulBufferLength + pPack->ulDataLength+2;
   m_pMegaPack->ulDataLength   = m_pMegaPack->ulDataLength   + pPack->ulDataLength+2;

   //reset the timeout...
   m_pMegaPack->uldwTimeoutMegaE = timeGetTime()+50; //1 second maximum for mega pack construction...

   LeaveCriticalSection(&m_crMegaPackLock);
   return true;
}

inline void CUDPConnection::VerifyMegaPack()
{
   if(!m_pNMPacketManager)
      return;

   EnterCriticalSection(&m_crMegaPackLock);
   if(m_pMegaPack)
   {
      if(m_pMegaPack->uldwTimeoutMegaT < timeGetTime() || m_pMegaPack->uldwTimeoutMegaE < timeGetTime())
      {
         //exoirere on send le packet
         //AddToLogFile(FALSE,"Send megaPack Timeout");

         //CV14: Add to megapack send and clear the megapack here...
         m_pNMPacketManager->PostSendPacket(m_pMegaPack); 
         //m_pNMPacketManager->PostSendPacketMega(m_pMegaPack); 
         //m_pNMPacketManager->PacketFree(m_pMegaPack);
         m_pMegaPack = NULL;
      }
   }
   LeaveCriticalSection(&m_crMegaPackLock);
}


