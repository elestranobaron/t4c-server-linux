/******************************************************************************
Modify for vs2008 (30/04/2009)
Add tous les nouveau packet... Guild, Ahm,  Profession, system mort, etc etc etc by Nightmare (29/06/2009)
Add NM Firewall Chech by Nightmare (29/06/2009)
/******************************************************************************/
#include "stdafx.h"
#ifdef _WIN32
#include <direct.h>
#endif
#include <cstdint>
#include "TFC Server.h"
#include "UDP/NMPacketManager.h"
#include "PacketManager.h"
#include "TFCMessagesHandler.h"
#include "PlayerManager.h"

#ifndef _WIN32
/* ------------------------------------------------------------------------- *
 * Diagnostic de stall du thread de reception (Linux).
 * Quand le detecteur de deadlock soupconne un blocage de
 * 'NMPacketManager::UDPReceivePacketThread', on imprime l'opcode en cours de
 * traitement + la pile d'appels exacte du thread bloque (via SIGUSR1).
 * Permet de localiser precisement un handler synchrone qui bloque > 30 s.
 * ------------------------------------------------------------------------- */
#include <csignal>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <execinfo.h>
#include <pthread.h>

namespace {
volatile std::sig_atomic_t g_dbgRxOpcode = -1;   // opcode en cours de dispatch
volatile std::sig_atomic_t g_dbgRxActive = 0;    // 1 = dans DispatchPacket
volatile std::time_t       g_dbgRxStartTime = 0; // debut du dispatch courant
pthread_t                  g_dbgRxThread;
volatile std::sig_atomic_t g_dbgRxThreadSet = 0;

void DbgStallBacktraceHandler( int /*sig*/ )
{
   void *frames[64];
   const int n = backtrace( frames, 64 );
   backtrace_symbols_fd( frames, n, 2 /* stderr */ );
}
}  // namespace

/* Appele par le thread de reception a son demarrage. */
void DbgRegisterReceiveThreadForStallDump()
{
   g_dbgRxThread = pthread_self();
   g_dbgRxThreadSet = 1;

   struct sigaction sa;
   std::memset( &sa, 0, sizeof( sa ) );
   sa.sa_handler = DbgStallBacktraceHandler;
   sigemptyset( &sa.sa_mask );
   sa.sa_flags = SA_RESTART;
   sigaction( SIGUSR1, &sa, NULL );
}

/* Appele par le detecteur de deadlock quand le thread de reception stalle. */
void DbgDumpReceiveStall()
{
   const long elapsed =
      g_dbgRxStartTime != 0 ? static_cast<long>( std::time( NULL ) - g_dbgRxStartTime ) : -1;
   std::fprintf( stderr,
                 "[DEADLOCK] receive thread bloque sur opcode=%d (active=%d, ~%lds) — backtrace:\n",
                 static_cast<int>( g_dbgRxOpcode ),
                 static_cast<int>( g_dbgRxActive ),
                 elapsed );
   std::fflush( stderr );
   if( g_dbgRxThreadSet )
      pthread_kill( g_dbgRxThread, SIGUSR1 );
}

void DbgSetReceiveOpcode( int opcode, int active )
{
   g_dbgRxOpcode = opcode;
   g_dbgRxActive = active;
   if( active )
      g_dbgRxStartTime = std::time( NULL );
}
#endif

#ifdef _WIN32

class CMyHttpClient : public CHttpClient
{
public:
   virtual BOOL AllowThisConnection();

   DECLARE_DYNCREATE(CMyHttpClient)
};

IMPLEMENT_DYNCREATE(CMyHttpClient, CHttpClient)

BOOL CMyHttpClient::AllowThisConnection()
{
   //Create a string representation of the HTTP verb
   CString sVerb;
   switch (m_Request.m_Verb)
   {
   case CHttpRequest::HTTP_VERB_GET:     sVerb = _T("GET"); break;
   case CHttpRequest::HTTP_VERB_POST:    sVerb = _T("POST"); break;
   case CHttpRequest::HTTP_VERB_HEAD:    sVerb = _T("HEAD"); break;
   case CHttpRequest::HTTP_VERB_PUT:     sVerb = _T("PUT"); break;
   case CHttpRequest::HTTP_VERB_LINK:    sVerb = _T("LINK"); break;
   case CHttpRequest::HTTP_VERB_DELETE:  sVerb = _T("DELETE"); break;
   case CHttpRequest::HTTP_VERB_UNLINK:  sVerb = _T("UNLINK"); break;
   case CHttpRequest::HTTP_VERB_UNKNOWN: sVerb = _T("UNKNOWN"); break;
   }

   //Get the current date and time
   SYSTEMTIME st;
   GetLocalTime(&st);
   TCHAR sDate[64];
   GetDateFormat(NULL, LOCALE_NOUSEROVERRIDE, &st, NULL, sDate, 64);
   TCHAR sTime[64];
   GetTimeFormat(NULL, LOCALE_NOUSEROVERRIDE, &st, NULL, sTime, 64);

   //Display the connections to the console window
   CString sUser(m_Request.m_sUsername);
   if (sUser.IsEmpty())
      sUser = _T("Anonymous");

   _LOG_DEBUG
      LOG_ALWAYS,
      "%s %s, %s, %d.%d.%d.%d, %d",sVerb, m_Request.m_sURL, sUser,
      m_Request.m_ClientAddress.sin_addr.S_un.S_un_b.s_b1,
      m_Request.m_ClientAddress.sin_addr.S_un.S_un_b.s_b2, 
      m_Request.m_ClientAddress.sin_addr.S_un.S_un_b.s_b3, 
      m_Request.m_ClientAddress.sin_addr.S_un.S_un_b.s_b4, 
      ntohs(m_Request.m_ClientAddress.sin_port)
      LOG_
/*
   _tprintf(_T("%s, %s, %s %s, %s, %d.%d.%d.%d, %d\n"), sDate,
      sTime, sVerb, m_Request.m_sURL, sUser,
      m_Request.m_ClientAddress.sin_addr.S_un.S_un_b.s_b1,
      m_Request.m_ClientAddress.sin_addr.S_un.S_un_b.s_b2, 
      m_Request.m_ClientAddress.sin_addr.S_un.S_un_b.s_b3, 
      m_Request.m_ClientAddress.sin_addr.S_un.S_un_b.s_b4, 
      ntohs(m_Request.m_ClientAddress.sin_port));
      */

   //Let the parent do its thing
   return CHttpClient::AllowThisConnection();
}

#endif /* _WIN32 */

/******************************************************************************/
extern CTFCServerApp theApp;
NMPacketManager *CPacketManager::lpComm = NULL;
#ifdef _WIN32
CHttpServer CPacketManager::m_WebServer;
#endif
CLock destroyCommLock;

char g_strLogFileHACK[1024];

static std::uint16_t NM_Bswap16(std::uint16_t v) {
    return static_cast<std::uint16_t>((v >> 8) | (v << 8));
}
static std::uint32_t NM_Bswap32(std::uint32_t v) {
    return ((v & 0x000000FFu) << 24) | ((v & 0x0000FF00u) << 8) |
           ((v & 0x00FF0000u) >> 8) | ((v & 0xFF000000u) >> 24);
}
static std::uint64_t NM_Bswap64(std::uint64_t v) {
    return (static_cast<std::uint64_t>(NM_Bswap32(static_cast<std::uint32_t>(v & 0xFFFFFFFFu))) << 32) |
           NM_Bswap32(static_cast<std::uint32_t>(v >> 32));
}

__int64 Reverse (__int64 n)
{
   return static_cast<__int64>(NM_Bswap64(static_cast<std::uint64_t>(n)));
}

unsigned int Reverse (unsigned int n)
{
   return NM_Bswap32(n);
}

float Reverse (float n)
{
   unsigned int uiVal;
   float fVal;
   memcpy((char*)&uiVal,(char*)&n,4);
   uiVal = Reverse(uiVal);
   memcpy((char*)&fVal,(char*)&uiVal,4);
   return fVal;
}

unsigned short Reverse (unsigned short n)
{
   return NM_Bswap16(n);
}

short Reverse (short n)
{
   return static_cast<short>(NM_Bswap16(static_cast<std::uint16_t>(n)));
}

/******************************************************************************/
#define SETACK( __ackdelay, __maxack ) dwAckDelay = __ackdelay; dwMaxAck = __maxack; break;

/******************************************************************************/
void CPacketManager::Create( void )
/******************************************************************************/
{
    lpComm = new NMPacketManager();

    _LOG_DEBUG
        LOG_DEBUG_LVL4,
        "Initializing CPacketManager"
    LOG_

    const char *szRecvIP1 = theApp.sNetwork.csRecvIP1;
    WORD wRecvPort1       = theApp.sNetwork.wRecvPort1;

    const char *szRecvIP2 = theApp.sNetwork.csRecvIP2;
    WORD wRecvPort2       = theApp.sNetwork.wRecvPort2;

	if( !lpComm->Init(PacketInterpret,NULL,true,wRecvPort1,szRecvIP1,wRecvPort2,szRecvIP2))
	{
#ifndef _WIN32
		fprintf(stderr, "Could not initialize server's communication module\n");
#else
		AfxMessageBox(_T("Could not initialize server's communication module"));
#endif
		exit( CANNOT_BIND_TO_PORT );
	}

#ifdef _WIN32
	char pszBuffer[MAX_PATH*2];
	int loop = GetModuleFileName( GetModuleHandle( NULL ), pszBuffer, _MAX_PATH * 2 );		
	do
	{
		loop--;
	} while( pszBuffer[ loop ] != '\\' && loop >= 0 );
	// End string after backslash.
	pszBuffer[ loop + 1 ] = 0;
	int iSvrID = 0;
	CString strIDF;
	strIDF.Format("%ssvr.ID",pszBuffer);
	FILE *pfID = NULL;
	fopen_s(&pfID,strIDF.GetBuffer(0),"rt");
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
	sprintf_s(g_strLogFileHACK,1024,"C:\\!!LOG_UDP_ALL");
	_mkdir(g_strLogFileHACK);
	sprintf_s(g_strLogFileHACK,1024,"C:\\!!LOG_UDP_ALL\\LogHCK%d_%04d-%02d-%02d %02dh%02d_%02d.txt",iSvrID,
		sysTime.wYear, sysTime.wMonth,sysTime.wDay,sysTime.wHour, sysTime.wMinute,sysTime.wSecond);
#endif
	
}

#ifdef _WIN32
void CPacketManager::StartWebServer()
{
   _LOG_DEBUG
      LOG_ALWAYS,
      "START Web Server"
      LOG_


   CString strPath,strFile;
   strPath = TFCMAIN::GetHomeDir();
   strPath +="WebServer\\";
   CreateDirectory(strPath,NULL);
   strFile = "WebServer.html";
   CHttpDirectory VirtualDir;
   VirtualDir.SetAlias("\\");
   VirtualDir.SetDirectory(strPath);
   VirtualDir.SetDefaultFile(strFile);
   CHttpServerSettings settings;
   settings.m_Directories.SetAt(0, VirtualDir);
   settings.m_nPort = theApp.dwWebServerPort;
   //Setup the runtime client class
   settings.m_pRuntimeClientClass = RUNTIME_CLASS(CMyHttpClient);
   if(!m_WebServer.Start(settings))
   {
      _LOG_DEBUG
         LOG_ALWAYS,
         "ERROR ! unable to start web server"
         LOG_
   }
}
void CPacketManager::StopWebServer()
{
   _LOG_DEBUG
      LOG_ALWAYS,
      "STOP Web Server"
      LOG_

   m_WebServer.Stop();
}
#endif /* _WIN32 */

void CPacketManager::StopComm( void )
/******************************************************************************/
{
   if (lpComm) 
   {
      lpComm->ForceStopServer();
   }
}

/******************************************************************************/
void CPacketManager::Destroy( void )
/******************************************************************************/
{
   CAutoLock autoLock( &destroyCommLock );

	if (lpComm != NULL) 
	{
		delete lpComm;
		lpComm = NULL;
	}
}
/******************************************************************************/
// This function does a preminilary inspection of the packet to lock global resources.
void CPacketManager::PacketInterpret(COMM_INTR_PROTOTYPE) // The packet data information.
/******************************************************************************/
{
   if(!ValidPacket( lpbBuffer, nBufferSize,sockAddrO))
   {
#ifndef _WIN32
      if( nBufferSize >= 6 )
      {
         const short shID = Reverse( *(short *)( lpbBuffer + 4 ) );
         std::fprintf( stderr,
                       "[PKT] ValidPacket FAIL opcode=%d size=%d from %s:%u\n",
                       static_cast<int>( shID ), nBufferSize,
                       inet_ntoa( sockAddrO.sin_addr ),
                       static_cast<unsigned>( ntohs( sockAddrO.sin_port ) ) );
      }
#endif
      //Log we receive a invalid packet...
      return;
   }


   TFCPacket pRecv;
   // Set the packet to the received buffer.
   if( !pRecv.SetBuffer( lpbBuffer, nBufferSize ) )
   {
      return;
   }

	RQ_SIZE rqPacketID = 0;
    BOOL boTerminate = FALSE;

    try
	{
        // Fetch the packet type.
        pRecv.Get( (RQ_SIZE *)&rqPacketID );
	}
	catch( TFCPacketException *e )
	{
		TRACE( "\r\nPacket ID not found!!" );
        _LOG_DEBUG
            LOG_DEBUG_LVL1,
            "Sent undefined packet ID to client."
        LOG_
        
        if (e != NULL) 
		{
			delete e;
			e = NULL;
		}
        boTerminate = TRUE;
    }

    if( boTerminate )
	{
        return;
    }

#ifndef _WIN32
   DbgSetReceiveOpcode( static_cast<int>( rqPacketID ), 1 );
   struct DbgRxOpcodeScope
   {
      ~DbgRxOpcodeScope() { DbgSetReceiveOpcode( -1, 0 ); }
   } dbgRxOpcodeScope;
#endif

	//TRACE( "\r\nReceiving RQ=%u.", rqPacketID );

   //NMNMNM OutputDebug
   //FILE *pft = NULL;
   //fopen_s(&pft,"c:\\__svrrecv.txt","a+");
   //fprintf(pft,"R-> %d port %d\n",rqPacketID,sockAddr.sin_port);
   //fclose(pft);
   bool bValidRegister = true;
   if( rqPacketID == RQ_AuthenticateServerVersion || rqPacketID == RQ_ExitGame
       || rqPacketID == RQ_GetPersonnalPClist || rqPacketID == RQ_GetPersonnalPClistEquitSkin
       || rqPacketID == RQ_DeletePlayer || rqPacketID == RQ_CreatePlayer || rqPacketID == RQ_Reroll
       || rqPacketID == RQ_PutPlayerInGame
       || rqPacketID == RQ_FromPreInGameToInGame
       || rqPacketID == RQ_GetNearItems
       || rqPacketID == RQ_GetSkillList
       || ( rqPacketID >= RQ_MoveNorth && rqPacketID <= RQ_MoveNorthWest ) )
      bValidRegister = false;
    // Determines which packets need which resources.
   switch( rqPacketID )
	{
      case RQ_AuthenticateServerVersion:
      case RQ_ExitGame:
         bValidRegister = false;
		case RQ_MoveNorth:
		case RQ_MoveNorthEast:
		case RQ_MoveEast:
		case RQ_MoveSouthEast:
		case RQ_MoveSouth:
		case RQ_MoveSouthWest:
		case RQ_MoveWest:
		case RQ_MoveNorthWest:
      case RQ_MoveNorthKB:
      case RQ_MoveNorthEastKB:
      case RQ_MoveEastKB:
      case RQ_MoveSouthEastKB:
      case RQ_MoveSouthKB:
      case RQ_MoveSouthWestKB:
      case RQ_MoveWestKB:
      case RQ_MoveNorthWestKB:
		case RQ_GetPlayerPos:
		case RQ_Ack:
		case RQ_GetObject:
		case RQ_DepositObject:
		case RQ_PutPlayerInGame:
		case RQ_DeletePlayer:
		case RQ_GodCreateObject:
		case RQ_ViewBackpack2:
      case RQ_ViewInv:
		case RQ_ViewEquiped:
		case RQ_EquipObject:
		case RQ_UnequipObject:
		case RQ_UseObject:
		case RQ_UseObject2:
		case RQ_Attack:
		case RQ_CreatePlayer:
		case RQ_GetPersonnalPClist:
		case RQ_GetPersonnalPClistEquitSkin:
		case RQ_IndirectTalk:
		case RQ_Shout:
		case RQ_Page:
		case RQ_DirectedTalk:
      case RQ_DirectedTalkNoFeed:
		case RQ_Reroll:
		case RQ_CastSpell:
		case RQ_HPchanged:
		case RQ_BroadcastTextChange:
		case RQ_GetUnitName:
      case RQ_GetUnitName2:
		case RQ_BreakConversation:
		case RQ_LevelUp:
		case RQ_ReturnToMenu:
		case RQ_GetSkillList:
		case RQ_SendTrainSkillList:
		case RQ_SendBuyItemList:
      case RQ_SendPointsItemList:
		case RQ_UseSkill:
		case RQ_GetStatus:
      case RQ_GetStatus2:
      case RQ_ChestNormal:
      case RQ_ChestListe:
		case RQ_XPchanged:
		case RQ_FromPreInGameToInGame:
		//case RQ_YouDied:
		case RQ_EnterChatterChannel:
		case RQ_SendChatterMessage:
      case RQ_GetChatterUserList2:
		//case RQ_GetOnlinePlayerList:
		case RQ_GetSkillStatPoints:
		case RQ_GoldChange:
		case RQ_ViewGroundItemIndentContent:
		case RQ_SendTeachSkillList:
		case RQ_SendSellItemList:
		case RQ_TeleportPlayer:
		case RQ_SendStatTrain:
		case RQ_QueryItemName:
		case RQ_GetNearItems:
		case RQ_PlayerFastMode:
		case RQ_SendSpellList:
		case RQ_ServerMessage:
		case RQ_InfoMessage:
		case RQ_SpellEffect:
		case RQ_ManaChanged:
		case RQ_PuppetInformation:
		case RQ_QueryUnitExistence:
		case RQ_UseItemByAppearance:
      case RQ_UpdateSmile :
		case RQ_RemoveFromChatterChannel:
		case RQ_GetChatterChannelList:
		case RQ_ToggleChatterListening:
		case RQ_JunkItems:
		case RQ_GroupInvite:
		case RQ_GroupJoin:
		case RQ_GroupLeave:
		case RQ_GroupKick:
		case RQ_GroupToggleAutoSplit:
		case RQ_TogglePage:
		case RQ_Rob:
		case RQ_ChestAddItemFromBackpack:
		case RQ_ChestRemoveItemToBackpack:
		case RQ_TradeInvite:
		case RQ_TradeCancel:
		case RQ_TradeSetStatus:
		case RQ_TradeAddItemFromBackpack:
		case RQ_TradeRemoveItemToBackpack:
		case RQ_TradeClear:
		case RQ_TradeStarted:
		case RQ_QueryItemInfo:
		case RQ_UpdateBackpackAfterUse:  //123 — RQ_SafePlug logout antiplug

      case RQ_WeatherMsg:
	   case RQ_NM_SendTeachFormuleList:
	   case RQ_NameChange:

      case RQ_GMMSG_Post:
      case RQ_GMMSG_Get:
      case RQ_GMMSG_Close:

      case RQ_GetAllArealinkForWorld:
      case RQ_GetAllPlayerPos:
      case RQ_SvrOptions:
      case RQ_SvrNPC:
      case RQ_SvrSpellList:
      case RQ_SvrMonsterList:
      case RQ_SvrItemsList:
      case RQ_SvrMonsterSkin:
      case RQ_SvrQuestFlagList:
      case RQ_AttackMode:
      case RQ_AskCompagnonName:
      case RQ_GetEventsList:


      case RQ_NM_GetGuildList:
      case RQ_NM_GuildInvite:
      case RQ_NM_GuildJoin:
      case RQ_NM_GuildLeave:
      case RQ_NM_GuildKick:
      case RQ_NM_GuildChangeSetting:
      case RQ_NM_GuildChangeNote:
      case RQ_NM_GuildGetLogs:
      case RQ_NM_GUILDChestAddItem:
      case RQ_NM_GUILDChestRemoveItem:

      case RQ_NM_GetAHList:
      case RQ_NM_AddAHItems:
      case RQ_NM_BuyAHItems:
      case RQ_NM_CancelAHItems:
      case RQ_NM_InfoAHItems:

      case RQ_RPStatus:
      case RQ_RP_BroadCastRP:
      case RQ_RP_BroadCastPVP:
      case RQ_RP_BroadCastPVPStat:
      case RQ_RP_CreerRP:
      case RQ_RP_TerminerRP:
      case RQ_RP_RejoindreRP:
      case RQ_RP_RejoindreAnswerRP:
      case RQ_RP_ExpulserRP:
      case RQ_RP_InviteRP:
      case RQ_RP_InviteAnswerRP:
	   case RQ_RP_SignalerRP:

      case RQ_QB_GetQuestList:
      case RQ_QB_GetQuestMsg:
      case RQ_QB_GetActiveQuest:
      case RQ_QB_GetQuestListComplete:
      case RQ_QB_StopQuest:

      case RQ_NM_NMSGOLD_AchatOpt1:
      case RQ_NM_NMSGOLD_AchatOpt2:
      case RQ_NM_NMSGOLD_AchatOpt3:
      case RQ_NM_NMSGOLD_AchatOpt4:
      case RQ_NM_NMSGOLD_Acheter:
      case RQ_NM_NMSGOLD_ListPanier:
      case RQ_NM_NMSGOLD_UtiliserPanier:
      case RQ_NM_NMSGOLD_CrediterPanier:
      case RQ_NM_NMSGOLD_Sanction:

      case RQ_NM_DeathStatus:
      case RQ_NM_DeathProgress:
      case RQ_NM_DeathResurect:
      case RQ_NM_GetProfession:
      case RQ_NM_SendMakeFormule:
      case RQ_NM_XPScrollProgress:
      case RQ_NM_ORScrollProgress:

      case RQ_ARENA1_Join:
      case RQ_ARENA1_Leave:
      case RQ_ARENA1_GetWaitPlayerList:
      case RQ_ARENA1_UpdatePlayStat:
      case RQ_UpdateFactionID:
         {
            // Fetch player resource.
            Players *lpPlayer;
            
            // Fetch player resource corresponding to the packet's address
            try
            {
               const bool bIsMoveOpcode =
                  ( rqPacketID >= RQ_MoveNorth && rqPacketID <= RQ_MoveNorthWest )
                  || ( rqPacketID >= RQ_MoveNorthKB && rqPacketID <= RQ_MoveNorthWestKB );
               const bool bIsSessionOpcode =
                  rqPacketID == RQ_FromPreInGameToInGame
                  || rqPacketID == RQ_GetNearItems
                  || rqPacketID == RQ_GetSkillList
                  || rqPacketID == RQ_ExitGame
                  || rqPacketID == RQ_UpdateBackpackAfterUse
                  || bIsMoveOpcode;
               if( bIsSessionOpcode )
                  lpPlayer = CPlayerManager::GetPlayerResourceFctForSession( sockAddrO );
               else
                  lpPlayer = CPlayerManager::GetPlayerResourceFct( sockAddrO );
               if( lpPlayer != NULL && ((bValidRegister && lpPlayer->registred) || !bValidRegister)) // If a player resource could be fetched
               {
                  // Reset Idle time on player.
                  lpPlayer->ResetIdle();
                  lpPlayer->m_dwXPCurrentTick  = GetTickCount();
                  lpPlayer->m_dwDPSCurrentTick = GetTickCount();
                  // Send packet for interpretation
                  TFCMessagesHandler::DispatchPacket( &pRecv, lpPlayer, rqPacketID, sockAddrO, sockAddrI);
                  //TRACE( "\r\nFreeing player resource" );
                  // Frees the player resource after it has been analyzed.
                  CPlayerManager::FreePlayerResourceFct( lpPlayer );
               }
               else
               {
                  if(lpPlayer)
                  {
                     CPlayerManager::FreePlayerResourceFct( lpPlayer );
#ifndef _WIN32
                     if( rqPacketID == RQ_GetPersonnalPClist || rqPacketID == RQ_GetPersonnalPClistEquitSkin
                         || rqPacketID == RQ_CreatePlayer || rqPacketID == RQ_DeletePlayer
                         || rqPacketID == RQ_FromPreInGameToInGame
                         || rqPacketID == RQ_GetNearItems || rqPacketID == RQ_GetSkillList
                         || ( rqPacketID >= RQ_MoveNorth && rqPacketID <= RQ_MoveNorthWest )
                         || ( rqPacketID >= RQ_MoveNorthKB && rqPacketID <= RQ_MoveNorthWestKB ) )
                     {
                        std::fprintf( stderr,
                                      "[PKT] paquet %u ignore — registred=0 compte='%s' IP %s:%u\n",
                                      static_cast<unsigned>( rqPacketID ),
                                      (LPCTSTR)lpPlayer->GetAccount(),
                                      inet_ntoa( sockAddrO.sin_addr ),
                                      static_cast<unsigned>( ntohs( sockAddrO.sin_port ) ) );
                     }
#endif
                     _LOG_DEBUG LOG_DEBUG_LVL1, 
                        "Received an unregistered packet ID %u from user %s "
                        "playing character %s, IP %s.", 
                        rqPacketID,
                        (LPCTSTR)lpPlayer->GetFullAccountName(),
                        (LPCTSTR)lpPlayer->self->GetTrueName(),
                        (LPCTSTR)lpPlayer->GetIP()
                        LOG_
                  }
                  else
                  {
#ifndef _WIN32
                     if( rqPacketID == RQ_AuthenticateServerVersion || rqPacketID == RQ_GetPersonnalPClist
                         || rqPacketID == RQ_CreatePlayer || rqPacketID == RQ_DeletePlayer
                         || rqPacketID == RQ_ExitGame || rqPacketID == RQ_RegisterAccount
                         || rqPacketID == RQ_PutPlayerInGame
                         || rqPacketID == RQ_FromPreInGameToInGame
                         || ( rqPacketID >= RQ_MoveNorth && rqPacketID <= RQ_MoveNorthWest )
                         || ( rqPacketID >= RQ_MoveNorthKB && rqPacketID <= RQ_MoveNorthWestKB ) )
                     {
                        std::fprintf( stderr,
                                      "[PKT] paquet %u ignore — joueur NULL (IP %s:%u)\n",
                                      static_cast<unsigned>( rqPacketID ),
                                      inet_ntoa( sockAddrO.sin_addr ),
                                      static_cast<unsigned>( ntohs( sockAddrO.sin_port ) ) );
                     }
#endif
                     //on log pa speu etre assez frequent en cas de flood ou autre request long
                     /*
                     _LOG_DEBUG LOG_DEBUG_LVL1, 
                        "Received an unregistered packet ID %u from NULL user IP: %s",
                        rqPacketID,
                        inet_ntoa(sockAddrO.sin_addr)
                        LOG_
                     */
                  }
               }
            }
            catch(...)
            {
               CPlayerManager::FreePlayerResourceFct( lpPlayer );
               throw;
            }
         }	   
		break;

      //Valid pas les NULL ici
      case RQ_QueryServerVersion:
      case RQ_GetTime:
      case RQ_QueryNameExistence:
      case RQ_MessageOfDay:
      case RQ_QueryPatchServerInfo:
      case RQ_QueryPatchServerInfo2:
      case RQ_RegisterAccount:
			// Send message directly to the TFCMessagesHandler.
			TFCMessagesHandler::DispatchPacket( &pRecv, NULL, rqPacketID, sockAddrO,sockAddrI );
      break;
	}
}
/******************************************************************************/
// Sends a packet to the remote client.
void CPacketManager::SendPacket(
                                TFCPacket     &pPacket,              // The packet.
                                sockaddr_in    sockAddrO,             // Adress of target client.
                                sockaddr_in    sockAddrI,             // Adress IP Local.
                                int            nBroadcastRange,      // Range to broadcast message.
                                WorldPos       wlCenter,             // Center from which to broadcast.
                                BOOL           boBroadcastSend,          // TRUE if packet should be broadcast.
                                SendPacketVisitor *sendPacketVisitor,// An object to visitor before sending
                                // the packet to a player.
                                bool           inGame,                // Send only to ingame units.
                                bool bUseLevelRange,
                                int iLevelMin,
                                int iLevelMax
                                )
/******************************************************************************/
{
   static DWORD dwLastReportTime = 0;
   const int  ReportTime = 300000;// Five minutes per report.

   RQ_SIZE rqPacketID = pPacket.GetPacketID();

   BOOL  boSend = TRUE;
   bool  bSendDirect = false;

   // Unsafe packet by default.
   DWORD dwAckDelay = 1000;
   DWORD dwMaxAck   = 1;      //5

	// Determine the packet's security level.
	switch( rqPacketID )
	{
      case RQ_MoveNorth:		
      case RQ_MoveNorthEast:	
      case RQ_MoveEast:       
      case RQ_MoveSouthEast:  
      case RQ_MoveSouth:      
      case RQ_MoveSouthWest:  
      case RQ_MoveWest:        
      case RQ_MoveNorthWest: 
      case RQ_MoveNorthKB:
      case RQ_MoveNorthEastKB:
      case RQ_MoveEastKB:
      case RQ_MoveSouthEastKB:
      case RQ_MoveSouthKB:
      case RQ_MoveSouthWestKB:
      case RQ_MoveWestKB:
      case RQ_MoveNorthWestKB:
      case RQ_GetPlayerPos:   
      case RQ_Ack:     
      case RQ_SpellEffect: 
      case RQ_1stApril:
      case RQ_HPchanged:
      case RQ_XPchanged:   
      case RQ_ManaChanged:            				
         bSendDirect = true;
         SETACK( 0, 0 );
      break;
      
              		
      case RQ_GoldChange:             		
      case RQ_ViewGroundItemIndentContent:
      case RQ_GetTime:          
      case __EVENT_MISS:
      case RQ_DamageUnit:                           
      case RQ_HealingUnit:    
      case RQ_AskCompagnonName:
         SETACK( 0, 0 );
      break;

      case RQ_QueryItemName:  
         SETACK( 750,5 ); 
      break;

      case RQ_QueryNameExistence:
      case RQ_RegisterAccount:
      case RQ_QueryServerVersion:
      case RQ_QueryPatchServerInfo:
      case RQ_QueryPatchServerInfo2:
      case RQ_MessageOfDay:
      case RQ_ExitGame:               				
      case RQ_ExitGameAntiPlug:	
         bSendDirect = true;
         SETACK( 2000, 2 ); //2000 1
      break;
      case RQ_PutPlayerInGame:
         bSendDirect = true;
         SETACK( 2000, 3 ); //2000 2
      break;
      case RQ_SendPeriphericObjects:
         if(!inGame) 
            bSendDirect = true;
         SETACK( 750, 3 ); //2000 2
         break;
      case RQ_TeleportPlayer: 
      case RQ_SendChatterMessage:
      case RQ_IndirectTalk:           				
      case RQ_DirectedTalk:      
      case RQ_DirectedTalkNoFeed:
         bSendDirect = true;
         SETACK(750,5); //1000   1
      break;

      case RQ_Attack:
      case RQ_CastSpell: 
      case RQ_GetObject: 
      case RQ_DepositObject:
      case RQ_UseObject:
	   case RQ_UseObject2:
      case RQ_UseSkill:   
      case RQ_BreakConversation:      				
         bSendDirect = true;
         SETACK(750,3); //1000   2
      break;

      case RQ_ViewBackpack2: 
      case RQ_ChestContents:	
      case RQ_ChestNewContents:	
      case RQ_NM_GuildChestContents:                
      case RQ_GuildChestNewContents:                
         bSendDirect = true;
         SETACK(2000,2); //
      break;
            
      case RQ_GetUnitName:  
      case RQ_GetUnitName2:
      case RQ_DeletePlayer:           				
      case RQ_GodCreateObject:        				
      
      case RQ_ViewInv:
      case RQ_ViewEquiped:            				
      case RQ_EquipObject:            				
      case RQ_UnequipObject:          				
                    				
      case RQ_CreatePlayer:           				
      case RQ_GetPersonnalPClist:     				
	   case RQ_GetPersonnalPClistEquitSkin:
      case RQ_Page:                   				
      case RQ_Reroll:                 				
      case RQ_BroadcastTextChange:    				
      case RQ_LevelUp:                				
      case RQ_ReturnToMenu:                     
      case RQ_GetSkillList:           				
      case RQ_SendTrainSkillList:     				
      case RQ_SendBuyItemList:  
      case RQ_SendPointsItemList:
      case RQ_GetStatus:              				
      case RQ_GetStatus2:
      case RQ_ChestNormal:
      case RQ_ChestListe:
      case RQ_YouDied:                				
      case RQ_EnterChatterChannel:    				
      case RQ_GetChatterUserList2:   
      //case RQ_GetOnlinePlayerList:    				
      case RQ_GetSkillStatPoints:     				
      case RQ_SendTeachSkillList:     				
      case RQ_SendSellItemList:       				
      case RQ_SendStatTrain:          				
      case RQ_GetNearItems:          				
      case RQ_PlayerFastMode:         				
      case RQ_SendSpellList:          				
      case RQ_ServerMessage:          				
      case RQ_InfoMessage:            				
      case RQ_PuppetInformation:      			
      case RQ_UnitUpdate:             			
      case RQ_MissingUnit:            			
      case RQ_QueryUnitExistence:     			
      case RQ_UseItemByAppearance:  
      case RQ_UpdateSmile :
      case RQ_CannotFindItemByAppearance: 	
      case RQ_RemoveFromChatterChannel:   	
      case RQ_GetChatterChannelList:      	
      case RQ_CreateEffectStatus:         	
      case RQ_DispellEffectStatus:        	
      case RQ_UpdateGroupMembers:         	
      case RQ_UpdateGroupInviteList:      	
      case RQ_GroupInvite:                	
      case RQ_GroupJoin:                  	
      case RQ_GroupLeave:                 	
      case RQ_GroupKick:                  	
      case RQ_NotifyGroupDisband:         	
      case RQ_ToggleChatterListening:     	
      case RQ_UpdateGroupMemberHp:        	
      case RQ_GroupToggleAutoSplit:       	
      case RQ_UpdateWeight:               	
      case RQ_Rob:                        	
      case RQ_DispellRob:                 	
      case RQ_ArrowMiss:                  	
      case RQ_ArrowHit:                   	
      case RQ_GodFlagUpdate:               	
      case RQ_SeraphArrival:          			
      case RQ_AuthenticateServerVersion: 		
      case RQ_Remort:								
      case __EVENT_SOMETHING_DIED:    			
      case __EVENT_ATTACK:							
      case __EVENT_SKILL_USED:							   
      case 10004:											      
      case RQ_MaxCharactersPerAccountInfo:				
      case RQ_WeatherMsg:					 		   		
      case RQ_OpenURL:								       	
      case RQ_ChestAddItemFromBackpack:					
      case RQ_ChestRemoveItemToBackpack:					
      case RQ_ShowChest:			   						
      case RQ_ShowChestNew:			   						
      case RQ_HideChest:				   					
      case RQ_HideChestNew:				   					
      case RQ_TradeInvite:					   			   
      case RQ_TradeCancel:						      		
      case RQ_TradeSetStatus:							   	
      case RQ_TradeAddItemFromBackpack:					
      case RQ_TradeRemoveItemToBackpack:					
      case RQ_TradeClear:  									
      case RQ_TradeStarted:   								
      case RQ_TradeContents:	   							
      case RQ_TradeFinish:			      					
      case RQ_QueryItemInfo:				   				
      case RQ_AttackMode:                           
      case RQ_NameChange:                           
      case RQ_NM_SendTeachFormuleList:              
      case RQ_NM_GuildInvite:                       
      case RQ_NM_GuildJoin:                         
      case RQ_NM_GuildLeave:                        
      case RQ_NM_GuildKick:                         
      case RQ_NM_GuildChangeSetting:                
      case RQ_NM_GuildChangeNote:                   
      case RQ_NM_GuildGetLogs:                      
      case RQ_NM_GetGuildList:                      
      case RQ_NM_GuildChestShow:                    
      case RQ_NM_GuildChestHide:                    
      case RQ_GuildChestNewShow:                    
      case RQ_GuildChestNewHide:                    
      case RQ_NM_GUILDChestAddItem:                 
      case RQ_NM_GUILDChestRemoveItem:              
      case RQ_NM_GetAHList:                         
      case RQ_NM_AddAHItems:                        
      case RQ_NM_BuyAHItems:                        
      case RQ_NM_CancelAHItems:                     
      case RQ_NM_InfoAHItems:
      case RQ_UpdateBackpackAfterUse:               
      case RQ_NM_DeathStatus:                       
      case RQ_NM_DeathProgress:                     
      case RQ_NM_DeathResurect:                     
      case RQ_NM_GetProfession:                     
      case RQ_NM_SendMakeFormule:                   
      case RQ_NM_XPScrollProgress:                  
      case RQ_NM_ORScrollProgress:  
      case RQ_NM_NMSGOLD_AchatOpt1:
      case RQ_NM_NMSGOLD_AchatOpt2:
      case RQ_NM_NMSGOLD_AchatOpt3:
      case RQ_NM_NMSGOLD_AchatOpt4:
      case RQ_NM_NMSGOLD_Acheter:
      case RQ_NM_NMSGOLD_ListPanier:
      case RQ_NM_NMSGOLD_UtiliserPanier:
      case RQ_NM_NMSGOLD_CrediterPanier:
      case RQ_NM_NMSGOLD_Sanction:
      case RQ_RPStatus:
      case RQ_ARENA_SendTakeStatus:
      case RQ_ARENA1_Join:
      case RQ_ARENA1_Leave:
      case RQ_ARENA1_GetWaitPlayerList:
      case RQ_ARENA1_UpdateTimeBS:
      case RQ_ARENA1_UpdatePlayStat:
         //bSendDirect = true;
         SETACK(750,4); //1000   3
      break;
      case RQ_GMMSG_Post:
      case RQ_GMMSG_Get:
      case RQ_GMMSG_Close:
      case RQ_GetAllArealinkForWorld:
      case RQ_GetAllPlayerPos:
      case RQ_SvrOptions:                           
      case RQ_SvrNPC:                               
      case RQ_SvrSpellList:                         
      case RQ_SvrMonsterList:                       
      case RQ_SvrItemsList:                         
      case RQ_SvrMonsterSkin:                       
      case RQ_SvrQuestFlagList: 
      case RQ_GetEventsList:

      case RQ_RP_CreerRP:
      case RQ_RP_BroadCastRP:
      case RQ_RP_BroadCastPVP:
      case RQ_RP_BroadCastPVPStat:
      case RQ_RP_TerminerRP:
      case RQ_RP_RejoindreRP:
      case RQ_RP_RejoindreAnswerRP:
      case RQ_RP_ExpulserRP:
      case RQ_RP_InviteRP:
      case RQ_RP_InviteAnswerRP:
	   case RQ_RP_SignalerRP:

      case RQ_QB_GetQuestList:
      case RQ_QB_GetQuestMsg:
      case RQ_QB_GetActiveQuest:
      case RQ_QB_GetQuestListComplete:
      case RQ_QB_StopQuest:
      case RQ_UpdateFactionID:
         
         bSendDirect = true;
         SETACK( 1000, 2 ); // 1000 3
      break;
      case RQ_FromPreInGameToInGame:  					
         bSendDirect = true;
         SETACK( 3000, 3 ); //3000 2
      break;

    // If packet isn't a valid request
      default:
        boSend = FALSE;
      break;
    }

    if( boSend )
	{
        // Retreive the packet buffer.
        LPBYTE lpbBuffer = NULL;
        int    nBufferSize = 0;

        pPacket.GetBuffer( lpbBuffer, nBufferSize );

        // If packet should be broadcasted.
        if( boBroadcastSend )
        {
          
           vector < sSockBooth > vAddress;
           
           if( wlCenter.X == -1 || wlCenter.Y == -1 || wlCenter.world == -1 || nBroadcastRange == -1 )
           {
              // If center or range isn't valid, packet is globally broadcasted.
              if(bUseLevelRange)
                 CPlayerManager::GetGlobalBroadcastAddressLevelRange( vAddress, sendPacketVisitor, inGame ,iLevelMin, iLevelMax);
              else
                 CPlayerManager::GetGlobalBroadcastAddress( vAddress, sendPacketVisitor, inGame );
           }
           else
           {
              // Otherwise packet is only broadcasted locally            
              CPlayerManager::GetLocalBroadcastAddress ( vAddress, wlCenter, nBroadcastRange, sendPacketVisitor );
           }

           // Send message to all recipients.
           vector< sSockBooth >::iterator i;
           for( i = vAddress.begin(); i != vAddress.end(); i++ )
           {
/*
//               FILE *pft = NULL;
//               fopen_s(&pft,"c:\\__svrSend.txt","a+");
//               fprintf(pft,"BS-> %d port %d\n",rqPacketID,(*i).sin_port);
//               fclose(pft);
*/
              //TRACE( "\r\nBSending %u.", rqPacketID );                               

              // Send packet.
              lpComm->SendUDPPacket((*i).skO,(*i).skI,lpbBuffer,nBufferSize,dwAckDelay,dwMaxAck,false,true,bSendDirect);
            }
        }
        else
        {
/*
//            FILE *pft = NULL;
//            fopen_s(&pft,"c:\\__svrSend.txt","a+");
//            fprintf(pft,"BS-> %d port %d\n",rqPacketID,sockAddr.sin_port);
//            fclose(pft);
*/

           TRACE( "\r\nSending %u.", rqPacketID );

           // Send packet directly to client.
           lpComm->SendUDPPacket( sockAddrO,sockAddrI,lpbBuffer,nBufferSize,dwAckDelay,dwMaxAck,false,true,bSendDirect);
        }
    }
}
/******************************************************************************/
// Returns the const CommCenter object.
NMPacketManager *CPacketManager::GetCommCenter( void )
/******************************************************************************/
{
    CAutoLock autoLock( &destroyCommLock );
    return lpComm;
}



bool CPacketManager::ValidPacket(LPBYTE lpbBuffer, int nBufferSize,sockaddr_in sockAddr)
{
   if(theApp.dwUDPFilterEnable == 0)
      return true; //not usint validation packet...


   int iDataSize = nBufferSize-=4;
   BYTE *pByte   = lpbBuffer+=4;

   //must contain 2 bytes... the packet ID...
   if(iDataSize <2)
      return false;



   short shID = Reverse(*(short *)&pByte[0]); pByte+=2;
   iDataSize -=2; //Remove the ID...


   bool bValid = false;
   switch(shID)
   {
      //no data packet...
      case RQ_MoveNorth :            // 1
      case RQ_MoveNorthEast :        // 2
      case RQ_MoveEast :             // 3
      case RQ_MoveSouthEast :        // 4
      case RQ_MoveSouth :            // 5
      case RQ_MoveSouthWest :        // 6
      case RQ_MoveWest :             // 7
      case RQ_MoveNorthWest :        // 8
      case RQ_MoveNorthKB:           // 30001
      case RQ_MoveNorthEastKB:       // 30002
      case RQ_MoveEastKB:            // 30003
      case RQ_MoveSouthEastKB:       // 30004
      case RQ_MoveSouthKB:           // 30005
      case RQ_MoveSouthWestKB:       // 30006
      case RQ_MoveWestKB:            // 30007
      case RQ_MoveNorthWestKB:       // 30008
      case RQ_GetPlayerPos:          // 9
      case RQ_Ack :                  //10
      case RQ_ViewEquiped :          //19
      case RQ_ExitGame:              //20
      case RQ_GetPersonnalPClist :   //26
      case RQ_Reroll:                //31
      case RQ_HPchanged:             //33
      case RQ_LevelUp:               //37      ///eeeeeee
      case RQ_ReturnToMenu:          //38
      case RQ_GetSkillList:          //39
      case RQ_GetStatus:             //43
      case RQ_XPchanged:             //44
      case RQ_GetTime:               //45
      case RQ_FromPreInGameToInGame: //46
      //case RQ_GetOnlinePlayerList:   //51
      case RQ_GetSkillStatPoints:    //52
      case RQ_GoldChange:            //53
      case RQ_GetNearItems:          //60
      case RQ_QueryServerVersion:    //65
      case RQ_MessageOfDay :         //66
      case RQ_GetChatterChannelList: //75
      case RQ_GroupJoin:             //79
      case RQ_GroupLeave:            //80
      case RQ_QueryPatchServerInfo : //91
      case RQ_QueryPatchServerInfo2: //18
      case RQ_ChestContents:         //106
      case RQ_TradeCancel:				 //118
      case RQ_TradeClear:				 //119
      case RQ_NM_GetGuildList:       //126
      case RQ_NM_GuildJoin:          //128
      case RQ_NM_GuildLeave:         //129
      case RQ_NM_GuildGetLogs:       //133
      case RQ_NM_GuildChestContents: //136
      case RQ_NM_GetAHList:          //140
      case RQ_ChestNormal:           //148
      case RQ_ChestListe:            //149
      case RQ_WeatherMsg:            //150
      case RQ_GetStatus2:            //151
	   case RQ_GetPersonnalPClistEquitSkin://153
      case RQ_ViewBackpack2:         //154
      case RQ_ViewInv:               //156
      case RQ_NM_NMSGOLD_AchatOpt1:  //160
      case RQ_NM_NMSGOLD_AchatOpt2:  //161
      case RQ_NM_NMSGOLD_AchatOpt3:  //162
      case RQ_NM_NMSGOLD_AchatOpt4:  //163
      case RQ_NM_NMSGOLD_ListPanier: //165
      case RQ_NM_NMSGOLD_Sanction:   //168
      case RQ_RP_BroadCastRP:        //170
      case RQ_RP_TerminerRP:         //172
      case RQ_RP_BroadCastPVP:       //180
      case RQ_RP_BroadCastPVPStat:   //181
      case RQ_QB_GetQuestList:       //182
      case RQ_QB_GetActiveQuest:     //183
      case RQ_QB_GetQuestListComplete: //185
      case RQ_GMMSG_Get:             //187
      case RQ_GetAllArealinkForWorld://189
      case RQ_GetAllPlayerPos:       //190
      case RQ_SvrOptions:            //191
      case RQ_SvrNPC:                //192
      case RQ_SvrSpellList:          //193
      case RQ_SvrMonsterList:        //194
      case RQ_SvrItemsList:          //195
      case RQ_SvrMonsterSkin:        //196
      case RQ_SvrQuestFlagList:      //197
      case RQ_NM_DeathResurect:      //202
      case RQ_NM_GetProfession:      //203
      case RQ_ChestNewContents:      //220
      case RQ_GuildChestNewContents: //225
      case RQ_GetEventsList:         //233
      case RQ_UpdateFactionID:       //234
      if(iDataSize == 0)
            bValid = true;
      break;


      // 1 byte packet data
      case RQ_UnequipObject:        //22
      case RQ_SendSpellList:        //62
      case RQ_UpdateBackpackAfterUse://123 — SafePlug logout antiplug (client 1.68+)
      case RQ_GroupToggleAutoSplit: //88
      case RQ_TogglePage:           //89
      case RQ_RP_RejoindreAnswerRP: //174
      case RQ_RP_InviteAnswerRP:    //177
         if(iDataSize == 1)
            bValid = true;
      break;

      // 2 bytes packet data
      case RQ_UseItemByAppearance:  //72    //appearance ID
      case RQ_TradeSetStatus :      //117   //2 Status
      case RQ_UpdateSmile :         //159
      case RQ_QB_GetQuestMsg:       //183
      case RQ_NM_SendMakeFormule:   //205   // ID de formule
      case RQ_QB_StopQuest:         //208
         if(iDataSize == 2)
            bValid = true;
      break;




      // 4 byte packet data
      case RQ_EquipObject :     //21
      case RQ_GroupKick :       //81    //4 ID
      case RQ_Rob:              //93    //4 ID
      case RQ_QueryItemInfo :   //122
      case RQ_NM_GuildKick:     //130
      case RQ_NM_InfoAHItems:   //144
      case RQ_NM_NMSGOLD_UtiliserPanier://166
      case RQ_NM_NMSGOLD_CrediterPanier://167
      case RQ_RP_RejoindreRP:   //173
      case RQ_RP_ExpulserRP:    //175
      case RQ_RP_SignalerRP:    //178
      case RQ_GMMSG_Close:      //188
      case RQ_AttackMode:       //198
         if(iDataSize == 4)
            bValid = true;
      break;


      // 5 byte packet data
      case RQ_SendPeriphericObjects : //16   //1 Dir, 2X , 2Y
      case RQ_QueryItemName :         //59
         if(iDataSize == 5)
            bValid = true;
      break;

      case RQ_BroadcastTextChange:   //34
         if(iDataSize == 6)
            bValid = true;
      break;


      // 8 byte packet data
      case RQ_GetObject :                 //11
      case RQ_UseObject:                  //23
      case RQ_Attack:                     //24
      case RQ_GetUnitName:                //35   //4 ID, 2 X, 2Y
      case RQ_BreakConversation:          //36   //4 ID, 2 X, 2Y
      case RQ_PuppetInformation :         //68   //4 ID, 2 X, 2Y
      case RQ_QueryUnitExistence:         //71   //4 ID, 2 X, 2Y
      case RQ_GroupInvite:                //78   //4 ID, 2 X, 2Y
      case RQ_JunkItems:                  //85   //4 ID, 4 Qty
      case RQ_ChestAddItemFromBackpack:   //107  //4 ID, 4 Qty
      case RQ_ChestRemoveItemToBackpack:	//108  //4 ID, 4 Qty
      case RQ_TradeAddItemFromBackpack:   //112  //4 ID, 4 Qty
      case RQ_TradeRemoveItemToBackpack:  //113  //4 ID, 4 Qty
      case RQ_TradeInvite:                //116  //4 ID, 2 X, 2Y
      case RQ_NM_GuildInvite:             //127  //4 ID, 2 X, 2Y
      case RQ_NM_GUILDChestAddItem:       //137  //4 ID, 4 Qty
      case RQ_NM_GUILDChestRemoveItem:    //138  //4 ID, 4 Qty
      case RQ_NM_CancelAHItems:           //143  //4 Timestamp.
      case RQ_NM_NMSGOLD_Acheter:         //164
      case RQ_RP_InviteRP:                //176
      case RQ_ARENA1_Join:                //227
      case RQ_ARENA1_Leave:               //228
      case RQ_ARENA1_GetWaitPlayerList:   //231
      case RQ_ARENA1_UpdatePlayStat:      //232
      
         if(iDataSize == 8)
            bValid = true;
      break;

      case RQ_GetUnitName2:               //152  //4 ID, 2 X, 2Y, 2Show
         if(iDataSize == 10)
            bValid = true;
      break;
      case RQ_UseObject2:				  //155
        if(iDataSize == 14)
           bValid = true;
      break;


      case RQ_PutPlayerInGame:  //13
      {
         //1 bytes taille nom ,nom
         int iNomS = pByte[0];
         iDataSize-=(iNomS+1);
         iDataSize-=4; //validation key
         if(iDataSize == 0)
            bValid = true;
      }

      case RQ_DeletePlayer:     //15
      case RQ_AskCompagnonName: //157
      {
         //1 bytes taille nom ,nom
         int iNomS = pByte[0];
         iDataSize-=(iNomS+1);
         if(iDataSize == 0)
            bValid = true;
      }
      break;

      case RQ_NM_BuyAHItems:  //142
      {
         //4 bytes Index, 1 Byte Is buy or Bid, 4 Bytes Prix,4 Bytes PrixNMS,4 IDS
         iDataSize-=17;
         if(iDataSize == 0)
            bValid = true;
      }


      case RQ_NM_GuildChangeNote:    //132
      case RQ_RP_CreerRP:            //171
      case RQ_GMMSG_Post:            //186
      {
         //2 bytes taille nom ,nom
         short shS = Reverse(*(short *)&pByte[0]); pByte+=2;
         iDataSize-=(shS+2);
         if(iDataSize == 0)
            bValid = true;
      }
      break;

      

      case RQ_DepositObject :      // 12  //2 bytes X, 2 Bytes Y, 4 Bytes ID, 4 byte qty
      case RQ_NM_GuildChangeSetting :   //131  //4 bytes USerID, 4 bytes UTitre, 4 bytes uPerm
      {

         iDataSize-=12;
         if(iDataSize == 0)
            bValid = true;
      }
      break;


      //Custom packet...
      case RQ_RegisterAccount:       //14
      {
         //1 bytes Taille Nom compte, Nom compte, 1Byte taille Pass, PAss, 4 byte version
         int iNomS = pByte[0];
         iDataSize-=(iNomS+1);
         if(iDataSize >0)
         {
            pByte += (iNomS+1);
            int iPassS = pByte[0];
            iDataSize -= (iPassS+5);
            if(iDataSize == 0)
               bValid = true;
         }
      }
      break;

      case RQ_CreatePlayer:  //25
      {
         //6 bytes question reponse, 1 bytes nom size, nom
         pByte+=6;
         iDataSize-=6;
         int iNomS = pByte[0];
         iDataSize-=(iNomS+1);
         if(iDataSize == 0)
            bValid = true;
      }
      break;

      case RQ_IndirectTalk:  //27
      {
         //4 byte ID, 1 byte direction, 4 bytes txt color, 4 bytes key,2 byte txt size, txt
         pByte+=13;
         iDataSize-=13;
         if(iDataSize >1)
         {
            short shS = Reverse(*(short *)&pByte[0]); pByte+=2;
            iDataSize-=(shS+2);
            if(iDataSize == 0)
               bValid = true;
         }
      }
      break;


      case RQ_Shout:  //28
      {
         //2 bytes size nom CC, CC, 4 bytes color, 2 bytes txt size,txt
         short shS = Reverse(*(short *)&pByte[0]); pByte+=2;
         iDataSize-=(shS+2+4);
         if(iDataSize >1)
         {
            pByte+=(shS+4);
            shS = Reverse(*(short *)&pByte[0]); pByte+=2;
            iDataSize-=(shS+2);
            if(iDataSize == 0)
               bValid = true;
         }
      }
      break;

      case RQ_Page:  //29
      {
         //2 bytes size nom player, nom player, 2 bytes txt size,txt
         short shS = Reverse(*(short *)&pByte[0]); pByte+=2;
         iDataSize-=(shS+2);
         if(iDataSize >1)
         {
            pByte+=(shS);
            shS = Reverse(*(short *)&pByte[0]); pByte+=2;
            iDataSize-=(shS+2);
            if(iDataSize == 0)
               bValid = true;
         }
      }
      break;

      case RQ_DirectedTalk : //30
      case RQ_DirectedTalkNoFeed: //50
      {
         //2 bytes XPos, 2Byte YPos, 4 Bytes ID, 1 bytes Direction, 4 Bytes Color , 4 bytes key, 2 Byte Message size, Message
         pByte+=17;
         iDataSize-=17;
         if(iDataSize >1)
         {
            short shS = Reverse(*(short *)&pByte[0]); pByte+=2;
            iDataSize-=(shS+2);
            if(iDataSize == 0)
               bValid = true;
         }
      }
      break;


      case RQ_CastSpell : //32
      {
         //2 Bytes ID, 2 Bytes XPos, 2 Bytes YPos, 4 Bytes ID
         pByte+=10;
         iDataSize-=10;
         if(iDataSize == 0)
            bValid = true;
      }
      break;

      case RQ_SendTrainSkillList : //40
      case RQ_SendBuyItemList :    //41
      case RQ_SendPointsItemList:  //209
      {
         // 2 X, 2Y, 4ID,2 NbItem (nbrfois 2 ItemID, 2 Item Qty)
         pByte+=8;
         iDataSize-=8;
         if(iDataSize >1)
         {
            unsigned short shS = Reverse(*(unsigned short *)&pByte[0]); pByte+=2;
            iDataSize-=2;
            for(int i=0;i<shS;i++)
               iDataSize-=4;

            if(iDataSize == 0)
               bValid = true;
         }
      }
      break;

      case RQ_UseSkill : //42
      {
         //2 Bytes Skill, 2 Bytes XPos, 2 Bytes YPos, 4 Bytes ID
         pByte+=10;
         iDataSize-=10;
         if(iDataSize == 0)
            bValid = true;
      }
      break;


      case RQ_EnterChatterChannel :  //48
      {
         //1 byte is ClearCCList,1 byte is UpdateList, 2 bytes size nom CC, CC, 2 byres pass size,Password
         pByte++;
         iDataSize-=1; // clear CC
         pByte++;
         iDataSize-=1; // UpdateList
         short shS = Reverse(*(short *)&pByte[0]); pByte+=2;
         iDataSize-=(shS+2);
         if(iDataSize >1)
         {
            pByte+=shS;
            shS = Reverse(*(short *)&pByte[0]); pByte+=2;
            iDataSize-=(shS+2);
            if(iDataSize == 0)
               bValid = true;
         }
      }
      break;

      case RQ_SendChatterMessage :  //49 
      {
         //2 bytes size str1 CC, str1, 2 byres str2 size,str2
         short shS = Reverse(*(short *)&pByte[0]); pByte+=2;
         iDataSize-=(shS+2);
         if(iDataSize >1)
         {
            pByte+=shS;
            shS = Reverse(*(short *)&pByte[0]); pByte+=2;
            iDataSize-=(shS+2);
            if(iDataSize == 0)
               bValid = true;
         }
      }
      break;




      case RQ_GetChatterUserList2:
      {

         //2byrtes CC Size, CC
         short shS = Reverse(*(short *)&pByte[0]); pByte+=2;
         iDataSize-=(shS+2);
         if(iDataSize == 0)
            bValid = true;
      }
      break;


      case RQ_ViewGroundItemIndentContent: //54
         //2 X, 2Y, 4 ID, 2 Ident
         if(iDataSize == 10)
            bValid = true;
      break;

      case RQ_SendTeachSkillList: //55
      case RQ_NM_SendTeachFormuleList: // 204
      {

         //2X, 2Y, 4 ID
         pByte+=8;
         iDataSize-=8;
         if(iDataSize >1)
         {
            unsigned short shS = Reverse(*(unsigned short *)&pByte[0]); pByte+=2;
            iDataSize-=2;
            for(int i=0;i<shS;i++)
               iDataSize-=2;

            if(iDataSize == 0)
               bValid = true;
         }
      }
      break;
      case RQ_SendSellItemList: //56
      {
         //2X, 2Y, 4 ID, 
         pByte+=8;
         iDataSize-=8;
         if(iDataSize >1)
         {
            unsigned short shS = Reverse(*(unsigned short *)&pByte[0]); pByte+=2; //2 QTY
            iDataSize-=2;
            for(int i=0;i<shS;i++)
               iDataSize-=8;

            if(iDataSize == 0)
               bValid = true;
         }
      }
      break;


      case RQ_SendStatTrain: //58
         //2byte by train stat...  STR,End,agi,Wil,Wis,int,Lck
         if(iDataSize == 14)
            bValid = true;

      break;

      case RQ_RemoveFromChatterChannel: //74
      {

         //2byrtes sty Size, str
         short shS = Reverse(*(short *)&pByte[0]); pByte+=2;
         iDataSize-=(shS+2);
         if(iDataSize == 0)
            bValid = true;
      }
      break;

      case RQ_ToggleChatterListening: //86
      {

         //2 brtes str Size, str, 1byte listen state
         short shS = Reverse(*(short *)&pByte[0]); pByte+=2;
         iDataSize-=(shS+2);
         if(iDataSize == 1)
            bValid = true;
      }
      break;






      case RQ_QueryNameExistence : //90
      {
         //2 bytes size nom, nom
         short shS = Reverse(*(short *)&pByte[0]); pByte+=2;
         iDataSize-=(shS+2);
         if(iDataSize == 0)
            bValid = true;
      }
      break;

      //one LONG data packet
      case RQ_AuthenticateServerVersion : //99
         //Only 4 LONG data...
         if(iDataSize == 4)
            bValid = true;
      break;

      case RQ_NM_AddAHItems:              //141  //4 ID, 4 Qty  , 4 BuyitPrice,4 BuyitPriceNMS, 4 BidMin, 4 maxTime
         if(iDataSize == 24)
            bValid = true;
      break;

      //ses packet sont envoyer par le serveur uniquement jamais 
      //par le client donc si recu sont invalide,...
      /*
      /////#define RQ_GodCreateObject				   17 
      /////#define RQ_YouDied						      47
      /////#define RQ_TeleportPlayer				   57
      /////#define RQ_PlayerFastMode				   61
      /////#define RQ_ServerMessage				      63
      /////#define RQ_SpellEffect                  64
      /////#define RQ_ManaChanged                  67
      /////#define RQ_UnitUpdate                   69
      /////#define RQ_MissingUnit                  70
      /////#define RQ_CannotFindItemByAppearance   73
      /////#define RQ_UpdateGroupMembers           76
      /////#define RQ_UpdateGroupInviteList        77
      /////#define RQ_NotifyGroupDisband           82
      /////#define RQ_CreateEffectStatus           83
      /////#define RQ_DispellEffectStatus          84
      /////#define RQ_UpdateGroupMemberHp          87
      /////#define RQ_UpdateWeight                 92
      /////#define RQ_DispellRob                   94
      /////#define RQ_ArrowMiss                    95
      /////#define RQ_ArrowHit                     96
      /////#define RQ_GodFlagUpdate                97
      /////#define RQ_SeraphArrival                98
      /////#define RQ_Remort                       100
      /////#define RQ_ExitGameAntiPlug				   101 
      /////#define RQ_InfoMessage					   102
      /////#define RQ_MaxCharactersPerAccountInfo  103
      /////#define RQ_OpenURL					      	105
      /////#define RQ_ShowChest				      	109
      /////#define RQ_HideChest			 	      	110
      /////#define RQ_TradeContents			       	111
      /////#define RQ_TradeStarted			    		120
      /////#define RQ_TradeFinish			    		121
      /////#define RQ_UpdateBackpackAfterUse       123
      /////#define RQ_DamageUnit                   124
      /////#define RQ_HealingUnit                  125
      /////#define RQ_NM_GuildChestShow            134
      /////#define RQ_NM_GuildChestHide            135
      /////#define RQ_WeatherMsg					      150
      /////#define RQ_RPStatus                     169
      /////#define RQ_NameChange				      	199
      /////#define RQ_NM_DeathStatus     		      200
      /////#define RQ_NM_DeathProgress   		      201
      /////#define RQ_NM_XPScrollProgress 		   206
      /////#define RQ_NM_ORScrollProgress          207
      /////#define RQ_ShowChestNew  		      	221
      /////#define RQ_HideChestNew		 	      	222
      /////#define RQ_GuildChestNewShow            223
      /////#define RQ_GuildChestNewHide            224
      /////#define RQ_ARENA_SendTakeStatus         226
      /////#define RQ_ARENA1_UpdateTimeBS          229
      */

   }


   if(!bValid)
   {
      _LOG_DEBUG
         LOG_DEBUG_LVL1,
         "---===HACK PACKET==-- Invalid packet.... ID[%d]   socket  %d.%d.%d.%d",
            shID,
            NM_IPV4_B1(sockAddr.sin_addr),
            NM_IPV4_B2(sockAddr.sin_addr),
            NM_IPV4_B3(sockAddr.sin_addr),
            NM_IPV4_B4(sockAddr.sin_addr)
         LOG_

         
         //Log Packet Invalide...
		 /*
         FILE *pf1 = NULL;
         fopen_s(&pf1,g_strLogFileHACK,"a+");
         if(pf1)
         {
            fprintf(pf1 ,"---===HACK PACKET==-- Invalid packet.... ID[%d]   socket  %d.%d.%d.%d\n",shID,
                        sockAddr.sin_addr.S_un.S_un_b.s_b1,
                        sockAddr.sin_addr.S_un.S_un_b.s_b2,
                        sockAddr.sin_addr.S_un.S_un_b.s_b3,
                        sockAddr.sin_addr.S_un.S_un_b.s_b4);
            fclose(pf1);
         }
		 */
         
   }


   return bValid;
}



