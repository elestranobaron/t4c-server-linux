/******************************************************************************
Modify for vs2008 (26/04/2009)
Add Guild,AH,tous les cleg de loading if ON/OF, magic World by Nightmare (29/06/2009)
******************************************************************************/
#include "stdafx.h"
#ifdef _WIN32
#include <conio.h>
#else
#include "LinuxConio.h"
#include <unistd.h>
#include <filesystem>
#endif
#include "TFC Server.h"
#include "TFCInit.h"
#include "TFC_MAIN.h"
#ifdef _WIN32
#include "C_Servic.h"
#endif
#include "RegKeyHandler.h"
#include "T4CLog.h"
#include "IntlText.h"
#include "AsyncFuncQueue.h"
#include "PacketManager.h"
#include "PlayerManager.h"
#include "TFCMessagesHandler.h"
#include "DeadlockDetector.h"
#include "AutoConfig.h"
#include "Game_Rules.h"
#include "Scheduler.h"
#include "Format.h"
#include "DynObjManager.h"
#include "Clans.h"
#include "SpellEffectManager.h"
#include "NPC Thread.h"
#include "MainConsole.h"
#include "SysopCmd.h"
#include <signal.h>
#include "version.h"
#include "ThreadMonitor.h"
#ifdef _WIN32
#include "UDP/NMPacketManager.h"
#endif
#include "CustomBuild.h"


#include "System.h"
#include "ExitCode.h"
#include "random.h"
#include "BoostFormula.h"
#include "GuildMaster.h"
#include "RPMaster.h"
#include "Arena1Master.h"
#include "Arena2Master.h"
#include "NMS5YearsEvents.h"
#include "EventsMaster.h"
#include "AuctionMaster.h"
#include "GMMsgMaster.h"
#include "Tokenizer.h"
#include "QuestFlagsListing.h"
#include "DynObjListing.h"

//#include "DevTools/bsccLogger.h"

#ifdef _WIN32
#include "LimitSingleInstance.H"
// The one and only CLimitSingleInstance object.
CLimitSingleInstance g_SingleInstanceObj;
#endif



extern CTFCServerApp theApp;

typedef struct _FlagListSaved
{
   DWORD flagID;
   DWORD flagValue;
}FlagListSaved;






/******************************************************************************/
#pragma data_seg ("SHARED_INSTANCE")
	BOOL boServerRunning = FALSE;
#pragma data_seg ()

#ifdef _DEBUG
	#define DEADLOCK	5000000
#else
	#define DEADLOCK	5000
#endif

#define VERSION_STRING  "v%u"

#ifdef _DEBUG
	#define new DEBUG_NEW
#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

#ifdef __ENABLE_LOG
	DEBUG_LOG __LOG(200, 100); // Basic debug level of 100 entries and 25 entries
#endif

/******************************************************************************/
#define _EXITLOG {\
		CString fileName( theApp.sPaths.csLogPath );\
		fileName += "exit.txt";\
		FILE *__f;\
		fopen_s(&__f, fileName, "ab" );\
		if(__f)\
		{\
			fprintf( __f, 

#define EXITLOG_ );\
			fclose(__f);\
		}\
	};

#define KEY2TXT( __str, __key, __default )\
	__str = regKey.GetProfileString( __key, __default );\
	__str.TrimRight();\
	__str.TrimLeft();

/******************************************************************************/
void __cdecl EntryFunction(void *);
void __cdecl StopFunction(void *);
void TerminateServer( bool boExit );
void TFCInitMaps(void);

/******************************************************************************/
extern TFC_MAIN *TFCServer;
extern Clans    *CreatureClans;
static CString   ServerPath;

/******************************************************************************/
// This functions makes sure a trailing backlash exists at the end of a string.
inline void AddTrailingBackslash(CString &csText)
/******************************************************************************/
{
	// Remove trailing white spaces
	csText.TrimLeft();
	// If string doesn't have a trailing backslash, add it.
	if( csText.GetAt( csText.GetLength() - 1 ) != '\\' ){
		csText += '\\';
	}
}
/******************************************************************************/
CTFCServerApp::CTFCServerApp() : szServerAcctID( "T4CSVRID" )
/******************************************************************************/
{
	current_check = 1;
	last_PlayerMainDeadlockFlag = 0xFFFFFFFF;
	last_TFCMainDeadlockFlag = 0xFFFFFFFF;
	last_PlayerMainTime = 0;
	last_TFCMainTime = 0;
	hCrtReportFile = INVALID_HANDLE_VALUE;

   m_hhGuildRequestListIO     = NULL;
   m_hhAHRequestListIO        = NULL;
   
	serverStarted = false;


  //Create Guild and AH and PVP async request QUEUE
   m_hhGuildRequestListIO     = CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL, 0, 1 );
   m_hhAHRequestListIO        = CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL, 0, 1 );

   
   for(int i=0;i<100;i++)
   {
      m_dwArenaSystem1[i] = 0;
      m_dwArenaSystem2[i] = 0;
   }
   m_dwNMS5YearItemnPod    = 0;

   m_dwRPSystem            = 1;
   m_dwGMMsgSystem         = 1;
   m_dwModeRPorHRP         = 1;
   m_dwXPstat              = 1;
   m_dwDUELSyetemActif     = 1;
   m_dwCCShortcut          = 1;
   m_dwPseudoname          = 1;
   m_dwPVPSyetem2Actif     = 1;
   m_dwManagePrisonExit    = 1;
   dwEnableCOMMMegaPack    = 1;
   dwEnableCOMMCompression = 1;
   m_dwFriendlyBlockPJAttack = 0;

   m_bProcessTakeALLprogress = false;
   m_pTakeAllPlayer          = NULL;

 
   int iCntStep = 0;
   for(int i=0;i<100;i++)
   {
      iCntStep = (i+1)*100;
      g_dwRPXPTableToLevel[i] = iCntStep;
      if(i == 0)
         g_dwRPXPTable[i] = iCntStep;
      else
         g_dwRPXPTable[i] = g_dwRPXPTable[i-1] + iCntStep;
   }
   

   FILE *pfE = NULL;
   fopen_s(&pfE,"EventSpecificItems.txt","rt");
   if(pfE)
   {
      char *pstrRead;
      char strLine[1024];
      do
      {
         pstrRead = fgets(strLine,1024,pfE);
         if(pstrRead)
         {
            if(pstrRead[strlen(pstrRead)-1] == '\n')
               pstrRead[strlen(pstrRead)-1] = NULL; //delete \n
                        
            //ex of line :00,00,blabla bla
            sEventItems nE;
            
            nE.iEvent = atoi(pstrRead);pstrRead+=3;
            nE.iType  = atoi(pstrRead);pstrRead+=3;
            nE.strID.Format("%s",pstrRead);
            
            m_aEventItems.Add(nE);
         }
      }while(pstrRead);
      fclose(pfE);
   }


   FILE *pfSP = NULL;
   fopen_s(&pfSP,"WorldPosSpell.txt","rt"); 
   if(pfSP)
   {
      char *pstrRead;
      char strLine[1024];
      do
      {
         pstrRead = fgets(strLine,1024,pfSP);
         if(pstrRead)
         {
            if(pstrRead[strlen(pstrRead)-1] == '\n')
               pstrRead[strlen(pstrRead)-1] = NULL; //delete \n

            //ex of line :X,Y,W=SpellID
            sPosWSpell newS;
            int dwNbr = sscanf_s(pstrRead,"%d,%d,%d=%d",&newS.X,&newS.Y,&newS.W,&newS.SpellID);
            if(dwNbr == 4)
            {
               m_aPosWSpell.Add(newS);
            }
         }
      }while(pstrRead);
      fclose(pfSP);
   }

}
/******************************************************************************/
CTFCServerApp::~CTFCServerApp()
/******************************************************************************/
{
	if( hCrtReportFile != INVALID_HANDLE_VALUE )
	{
		CloseHandle( hCrtReportFile );
	}
}

BOOL CTFCServerApp::IsFreeTime()
{
   if(m_bHavePaidTime == FALSE)
      return TRUE; //Server Always FREE...


   SYSTEMTIME sysTime; 
   GetLocalTime(&sysTime);

   if(m_abPaidTime[sysTime.wDayOfWeek][sysTime.wHour ] == 0)
      return TRUE;

   return FALSE;
}

/******************************************************************************/ 
//manage for PVP System...
BOOL CTFCServerApp::IsTargetOnList(UINT uiIDF,UINT uiIDS)
{
   //_LOG_DEBUG  LOG_DEBUG_LVL4,
   //   "========================== -> Recherche inside PVP Exception list %d  %d ",uiIDF,uiIDS
   //LOG_
   CAutoLock autoPVPListLock( &g_ALockPVPList );

   for(int i=0;i< m_aPVPListEx.GetSize() ;i++)
   {
      if(m_aPVPListEx[i].uiIDF == uiIDF && m_aPVPListEx[i].uiIDS == uiIDS)
      {
         m_aPVPListEx[i].iWDCnt = 0;
         return TRUE;
      }
   }
   return FALSE;
}

void CTFCServerApp::AddTargetOnList(UINT uiIDF,UINT uiIDS)
{
   CAutoLock autoPVPListLock( &g_ALockPVPList );

   for(int i=0;i< m_aPVPListEx.GetSize() ;i++)
   {
      if(m_aPVPListEx[i].uiIDF == uiIDF && m_aPVPListEx[i].uiIDS == uiIDS)
      {
         //existe de ja, on fait que resetter...
         m_aPVPListEx[i].iWDCnt = 0;
         return ;
      }
   }

   sHLLPVPEx sNew;

   sNew.uiIDF   = uiIDF;
   sNew.uiIDS   = uiIDS;
   sNew.iWDCnt  = 0;
   m_aPVPListEx.Add(sNew);

   return ;
}

void CTFCServerApp::ManageThreadFctOnList() 
{
   CAutoLock autoPVPListLock( &g_ALockPVPList );
   for(int i=0;i< m_aPVPListEx.GetSize() ;i++)
   {
      m_aPVPListEx[i].iWDCnt++;
      if(m_aPVPListEx[i].iWDCnt >6)  //6 *5sec == 30 sec sans combat... on delete de la liste
      {
         m_aPVPListEx.RemoveAt(i);
         i--;
      }
   }
}


BOOL CTFCServerApp::IsIDOnSpecificEvent(CString strID, int &iEvent, int &iType)
{
   for(int i=0;i<m_aEventItems.GetSize();i++)
   {
      if(m_aEventItems[i].strID == strID)
      {
         iEvent = m_aEventItems[i].iEvent;
         iType  = m_aEventItems[i].iType ;
         return TRUE;
      }
   }
   return FALSE;
}

/******************************************************************************/ 

////////////////////////////////////////////////////////////////////////////
//
//  GUILD ALL FUNCTION...
//
//
//
////////////////////////////////////////////////////////////////////////////
void CTFCServerApp::GuildRequestAnalyseProcess()
{
   DWORD dwFoo = 0;
   DWORD dwPacketAddr = 0;
   LPOVERLAPPED lpOverlapped = NULL;


   sGuildRequest* pRequest=NULL;

   BOOL bRet = FALSE;
   DWORD dwNbrProcess = 0;
   if(dwNbrProcess < 5)
   {
      dwNbrProcess++;
      bRet = GetQueuedCompletionStatus( m_hhGuildRequestListIO, &dwFoo, &dwPacketAddr, &lpOverlapped, 0 ) ;
      if(bRet)
      {
         CAutoLock autoGuildRequestLock( &g_ALockGuildRequest ); //NMNMNM a valider

         pRequest = reinterpret_cast< sGuildRequest* >( dwPacketAddr );
         switch(pRequest->dwRequestID)
         {
            case GUILD_REQ_GET_USER_LIST :
               GR_GetUserListID(pRequest->dwParam1,(char)pRequest->dwParam2);
            break;
            case GUILD_REQ_ADD_USER:
               GR_AddUserID(pRequest->dwParam1);
            break;
            case GUILD_REQ_REMOVE_USER:
            {
               bool bKick = false;
               if(pRequest->dwParam3 == 1)
                  bKick = true;
               GR_RemoveUserID(pRequest->dwParam1,pRequest->dwParam2,bKick);
            }
            break;
            case GUILD_REQ_REMOVE_OFFLINE_USER :
               GR_RemoveOfflineUserID(pRequest->dwParam1,pRequest->dwParam2);
            break;
            case GUILD_CHANGE_USER_SETTINGS :
               GR_ChangeUserStetingsID(pRequest->dwParam1,pRequest->dwParam2,pRequest->dwParam3,pRequest->dwParam4);
            break;
            case GUILD_CHANGE_NOTES:
               GR_ChangeNoteID(pRequest->dwParam1,pRequest->strParam1);
            break;
            case GUILD_REQUEST_LOGS:
               GR_RequestLogsID(pRequest->dwParam1);
            break;
            case GUILD_ADD_LOGS:
               GR_AddLogsID(pRequest->dwParam1,pRequest->strParam1);
            break;
            case GUILD_UPDATE_OFFLINE_NAME:
               GR_UpdateOfflineNameID(pRequest->pUser,pRequest->dwParam1,pRequest->strParam1);
            break;
            case GUILD_LOAD_CHEST:
               GR_LoadChestID(pRequest->strParam1);
            break;
            case GUILD_CREATE:
               GR_CreateGuildID(pRequest->dwParam1,pRequest->dwParam2,pRequest->strParam1);
            break;
            case GUILD_DELETE:
               GR_DeleteGuildID(pRequest->dwParam1,pRequest->strParam1);
            break;
            case GUILD_RENAME:
               GR_RenameGuildID(pRequest->dwParam1,pRequest->strParam1,pRequest->strParam2);
            break;
            case GUILD_MODIFY:
               GR_ModifyGuildID(pRequest->dwParam1,pRequest->dwParam2,pRequest->strParam1);
            break;
         }
         if(pRequest)
            delete pRequest;
         pRequest = NULL;

         Sleep(0); //libere le CPU pour ne pas traiter les requete en while1....
      }
   }
}


void CTFCServerApp::AddGuildRequest(Character *pUser,Character *pTarget,Players *pPlayerP,
                                    DWORD dwRequestID, DWORD dwParam1,DWORD dwParam2,DWORD dwParam3,DWORD dwParam4,CString strParam1, CString strParam2)
{
   CAutoLock autoGuildRequestLock( &g_ALockGuildRequest );

   sGuildRequest* pNewRequest = new sGuildRequest;
   
   pNewRequest->pUser       = pUser;
   pNewRequest->pTarget     = pTarget;
   pNewRequest->pPlayerP    = pPlayerP;
   pNewRequest->dwRequestID = dwRequestID;
   pNewRequest->dwParam1    = dwParam1;
   pNewRequest->dwParam2    = dwParam2;
   pNewRequest->dwParam3    = dwParam3;
   pNewRequest->dwParam4    = dwParam4;
   pNewRequest->strParam1   = strParam1;
   pNewRequest->strParam2   = strParam2;

   PostQueuedCompletionStatus( m_hhGuildRequestListIO, 0, reinterpret_cast< DWORD >( pNewRequest ), NULL );
}


void CTFCServerApp::GR_GetUserListID(DWORD dwUserID, char chShow)
{
   Players *pUser     = CPlayerManager::GetCharacterRessourceByID(dwUserID);//PM
   if(pUser)
   {
      //now on peu recup les membre de cette guild...
      Character *lpCharacter = static_cast< Character * >( pUser->self );
      if(lpCharacter)
         GuildMaster::SendGuildList(lpCharacter,chShow);
      CPlayerManager::FreePlayerResource(pUser);
   }
   else
   {
      _LOG_DEBUG
         LOG_DEBUG_LVL1,
         "************** Unable to lock resource in GR_GetUserListID"
         LOG_
   }
} 

void CTFCServerApp::GR_AddUserID(DWORD dwUserID)
{
   Players *pUser     = CPlayerManager::GetCharacterRessourceByID(dwUserID);//PM
   if(pUser)
   {
      Character *lpCharacter = static_cast< Character * >( pUser->self );
      if(lpCharacter)
      {
         //Ajoute maintenant a la liste des user de cette guild...
         GuildMaster::AddUserGuild(lpCharacter,lpCharacter->GetGuildNameInvited().GetBuffer(0));

         // Notify player of succesfull addition to group.
         TFormat format;

         pUser->self->SendInfoMessage(format( _STR( 15052 , lpCharacter->GetLang() ), lpCharacter->GetGuildName()),0x0080FF);
      }
      
      CPlayerManager::FreePlayerResource(pUser);
   }
   else
   {
      _LOG_DEBUG
         LOG_DEBUG_LVL1,
         "************** Unable to lock resource in GR_AddUserID"
      LOG_
   }
}



void CTFCServerApp::GR_RemoveUserID(DWORD dwUserID,DWORD dwTargetID,bool bKick)
{
   if(bKick == false)
   {
      Players *pUser     = CPlayerManager::GetCharacterRessourceByID(dwUserID);//PM
      if(pUser)
      {
         Character *lpCharacter = static_cast< Character * >( pUser->self );
         if(lpCharacter)
         {
            GuildMaster::RemoveUserGuild(lpCharacter,bKick,"");
            pUser->self->SendInfoMessage( _STR( 15058, pUser->self->GetLang() ),0x0080FF);
         }
         CPlayerManager::FreePlayerResource(pUser);
      }
      else
      {
         _LOG_DEBUG
            LOG_DEBUG_LVL1,
            "************** Unable to lock resource in GR_RemoveUserID (bKick=false)"
            LOG_
      }
   }
   else 
   {
      Players *pUser     = CPlayerManager::GetCharacterRessourceByID(dwUserID);//PM
      Players *pTarget   = CPlayerManager::GetCharacterRessourceByID(dwTargetID);//PM
      if(pUser && pTarget)
      {
         Character *lpCharacter = static_cast< Character * >( pTarget->self );
         if(lpCharacter)
         {
            //on sort le memebre de la guild...
            //delete de la liste des guilduser ce membre
            if(GuildMaster::RemoveUserGuild(lpCharacter,bKick,pUser->self->GetTrueName()) == 0)
            {
               pTarget->self->SendInfoMessage( _STR( 15058, pTarget->self->GetLang() ),0x0080FF);
               pUser->self->SendInfoMessage( _STR( 15068, pUser->self->GetLang() ),0x0080FF);
            }
            else
            {
               pUser->self->SendInfoMessage( _STR( 15067, pUser->self->GetLang() ),0x0080FF);
            }
         }
      }
      else
      {
         _LOG_DEBUG
            LOG_DEBUG_LVL1,
            "************** Unable to lock resource in GR_RemoveUserID (bKick=true)"
            LOG_
      }
      if(pUser)
         CPlayerManager::FreePlayerResource(pUser);
      if(pTarget)
         CPlayerManager::FreePlayerResource(pTarget);
   }
}


void CTFCServerApp::GR_RemoveOfflineUserID (DWORD dwUserID,DWORD dwRemUserID)
{
   Players *pUser     = CPlayerManager::GetCharacterRessourceByID(dwUserID);//PM
   if(pUser)
   {
      if(GuildMaster::RemoveUserGuildOffline(dwRemUserID,pUser->self->GetTrueName()) == 0)
      {
         if(pUser)
            pUser->self->SendInfoMessage( _STR( 15068, pUser->self->GetLang() ),0x0080FF);
      }
      else
      {
         if(pUser)
            pUser->self->SendInfoMessage( _STR( 15067, pUser->self->GetLang() ),0x0080FF);
      }
      CPlayerManager::FreePlayerResource(pUser);
   }
   else
   {
      _LOG_DEBUG
         LOG_DEBUG_LVL1,
         "************** Unable to lock resource in GR_RemoveOfflineUserID"
         LOG_
   }
}

void CTFCServerApp::GR_ChangeUserStetingsID (DWORD dwUserID,DWORD dwGuildUserID, DWORD dwTitre, DWORD dwPerm)
{
   Players *pUser     = CPlayerManager::GetCharacterRessourceByID(dwUserID);//PM
   if(pUser)
   {
      if(GuildMaster::ChangeUserSettings(pUser->self,dwGuildUserID,dwTitre,dwPerm,pUser->self->GetTrueName())==0)
      {
         if(pUser)
            pUser->self->SendInfoMessage( _STR( 15078, pUser->self->GetLang() ),0x0080FF);
      }
      else
      {
         if(pUser)
            pUser->self->SendInfoMessage( _STR( 15076, pUser->self->GetLang() ),0x0080FF);
      }
      CPlayerManager::FreePlayerResource(pUser);
   }
   else
   {
      _LOG_DEBUG
         LOG_DEBUG_LVL1,
         "************** Unable to lock resource in GR_ChangeUserStetingsID"
         LOG_
   }
}  


void CTFCServerApp::GR_ChangeNoteID(DWORD dwUserID,CString strNote)
{
   Players *pUser     = CPlayerManager::GetCharacterRessourceByID(dwUserID);//PM
   if(pUser)
   {
      GuildMaster::ChangeNoteSettings(pUser->self->GetGuildName(),strNote.GetBuffer(0),pUser->self->GetTrueName());
      CPlayerManager::FreePlayerResource(pUser);
   }
   else
   {
      _LOG_DEBUG
         LOG_DEBUG_LVL1,
         "************** Unable to lock resource in GR_ChangeNoteID"
         LOG_
   }
}

void CTFCServerApp::GR_RequestLogsID(DWORD dwUserID)
{
   Players *pUser     = CPlayerManager::GetCharacterRessourceByID(dwUserID);//PM
   if(pUser)
   {
      if(pUser->self)
         GuildMaster::SendLogsList(pUser->self);
      CPlayerManager::FreePlayerResource(pUser);
   }
   else
   {
      _LOG_DEBUG
         LOG_DEBUG_LVL1,
         "************** Unable to lock resource in GR_RequestLogsID"
         LOG_
   }
}

void CTFCServerApp::GR_AddLogsID(DWORD dwUserID, CString strItemName)
{
   Players *pUser     = CPlayerManager::GetCharacterRessourceByID(dwUserID);//PM
   if(pUser)
   {
      CString strTmp = strItemName;
      strTmp.Replace("%","%%");
      GuildMaster::AddGuildLog(pUser->self->GetGuildName(),strTmp.GetBuffer(0));
      CPlayerManager::FreePlayerResource(pUser);
   }
   else
   {
      _LOG_DEBUG
         LOG_DEBUG_LVL1,
         "************** Unable to lock resource in GR_AddLogsID"
         LOG_
   }
}

void CTFCServerApp::GR_UpdateOfflineNameID(Character *pUser,DWORD dwUserID,CString strName)
{
   if(pUser)
   {
      GuildMaster::RenameLastUserName(pUser,strName);
   }
   else
   {
      Players *pUserLck     = CPlayerManager::GetCharacterRessourceByID(dwUserID);//PM
      if(pUserLck)
      {
         GuildMaster::RenameLastUserName(pUserLck->self,strName);
         CPlayerManager::FreePlayerResource(pUserLck);
      }
      else
      {
         _LOG_DEBUG
            LOG_DEBUG_LVL1,
            "************** Unable to lock resource in GR_UpdateOfflineNameID"
            LOG_
      }
   }
}

void CTFCServerApp::GR_LoadChestID(CString strGuildName)
{
   GuildMaster::LoadGuildChest(strGuildName);
}

void CTFCServerApp::GR_CreateGuildID(DWORD dwUserID,DWORD dwFondateurID,CString strName)
{
   Players *pUser     = CPlayerManager::GetCharacterRessourceByID(dwUserID);//PM
   Players *pFondator = CPlayerManager::GetCharacterRessourceByID(dwFondateurID);//PM
   if(pUser && pFondator)
   {
   
      int iCreate = GuildMaster::CreateNewGuild(strName,pFondator->self->GetID(),pFondator);
      if(iCreate == 0)
      {
         CString strMessage;
         strMessage.Format("Guild %s Created...", strName );
         if(pUser)
            pUser->self->SendInfoMessage(strMessage,0x0080FF);
      }
      else if(iCreate == -1)
      {
         if(pUser)
            pUser->self->SendInfoMessage("This guild already exist...",0x0080FF);
      }
   }
   else if(pUser)
   {
      pUser->self->SendInfoMessage("Unable to get target resource",0x0080FF);
   }
   
   if(pUser)
      CPlayerManager::FreePlayerResource(pUser);
   if(pFondator)
      CPlayerManager::FreePlayerResource(pFondator);
   
}

void CTFCServerApp::GR_DeleteGuildID(DWORD dwUserID,CString strName)
{
   CString strMessage;

   int iResult = GuildMaster::DeleteGuild(strName);

   if(iResult == 0)
      strMessage.Format("Guild %s was successfully deleted", strName );
   else if(iResult == -1)
      strMessage.Format("Unable to find Guild %s", strName );

   Players *pUser     = CPlayerManager::GetCharacterRessourceByID(dwUserID);//PM
   if(pUser)
   {
      pUser->self->SendInfoMessage(strMessage,0x0080FF);
      CPlayerManager::FreePlayerResource(pUser);
   }
}

void CTFCServerApp::GR_RenameGuildID(DWORD dwUserID,CString strName,CString strNewName)
{
   int iResult = GuildMaster::RenameGuild(strName,strNewName);

   CString strMessage;

   if(iResult == 0)
      strMessage.Format("Guild %s was successfully renamed", strName );
   else if(iResult == -1)
      strMessage.Format("Unable to find Guild %s", strName );
   else if(iResult == -2)
      strMessage.Format("Guild %s already exist...", strNewName );

   Players *pUser     = CPlayerManager::GetCharacterRessourceByID(dwUserID);//PM
   if(pUser)
   {
      pUser->self->SendInfoMessage(strMessage,0x0080FF);
      CPlayerManager::FreePlayerResource(pUser);
   }
}

void CTFCServerApp::GR_ModifyGuildID(DWORD dwUserID,DWORD dwFondateurID,CString strName)
{
   Players *pUser     = CPlayerManager::GetCharacterRessourceByID(dwUserID);//PM
   Players *pFondator = CPlayerManager::GetCharacterRessourceByID(dwFondateurID);//PM
   if(pUser && pFondator)
   {
      //change the fondator of the guild...
      int iCreate = GuildMaster::ModifyGuild(strName,pFondator->self->GetID(),pFondator);
      if(iCreate == 0)
      {
         CString strMessage;
         strMessage.Format("%s is now the  founder of the guild %s ...", pFondator->self->GetTrueName(),strName );
         if(pUser)
            pUser->self->SendInfoMessage(strMessage,0x0080FF);
      }
      else if(iCreate == -1)
      {
         if(pUser)
         {
            CString strMessage;
            strMessage.Format("Unable to find Guild %s", strName );
            pUser->self->SendInfoMessage(strMessage,0x0080FF);
         }
      }
   }
   else if(pUser)
   {
      pUser->self->SendInfoMessage("Unable to get target resource",0x0080FF);
   }

   if(pUser)
      CPlayerManager::FreePlayerResource(pUser);
   if(pFondator)
      CPlayerManager::FreePlayerResource(pFondator);
}




////////////////////////////////////////////////////////////////////////////
//
//  MAISOn DES VENTES ALL FUNCTION...
//
//
//
////////////////////////////////////////////////////////////////////////////
void CTFCServerApp::AHRequestAnalyseProcess()
{
   DWORD dwFoo = 0;
   DWORD dwPacketAddr = 0;
   LPOVERLAPPED lpOverlapped = NULL;


   sAHRequest* pRequest=NULL;

   BOOL bRet = FALSE;
   DWORD dwNbrProcess = 0;
   if(dwNbrProcess < 5)
   {
      dwNbrProcess++;
      bRet = GetQueuedCompletionStatus( m_hhAHRequestListIO, &dwFoo, &dwPacketAddr, &lpOverlapped, 0 ) ;
      if(bRet)
      {
         CAutoLock autoGuildRequestLock( &g_ALockGuildRequest );
         pRequest = reinterpret_cast< sAHRequest* >( dwPacketAddr );
         switch(pRequest->dwRequestID)
         {
            case AH_REQ_GET_LIST :
               AHR_GetListID(pRequest->dwParam1,pRequest->dwParam2);
            break;
            case AH_ADD_ITEM:
               AHR_AddItemID(pRequest->dwParam1, pRequest->strParam1, pRequest->strParam2, pRequest->dwParam2, pRequest->dwParam3,
               pRequest->strParam3,pRequest->dwParam4,pRequest->dwParam8, pRequest->dwParam5, pRequest->dwParam6, pRequest->lParam7);
            break;
            case AH_BUY_ITEM:
               AHR_BuyItemID(pRequest->dwParam1,pRequest->dwParam2,pRequest->dwParam3,pRequest->dwParam4,pRequest->dwParam5,pRequest->dwParam6);
            break;
            case AH_CANCEL_ITEM:
               AHR_CancelItemID(pRequest->dwParam1,pRequest->dwParam2,pRequest->dwParam3);
            break;
            case AH_REQ_FORCE_EXPIRE:
               AHR_ForceExpireID(pRequest->dwParam1);
            break;
            case AH_INFO_ITEM:
               AHR_InfoItemID(pRequest->dwParam1,pRequest->dwParam2); //UserID, ItemID
            break;

            case AH_BANK_INTERET:
               AHR_AddBankInteretID(pRequest->dwParam1,pRequest->dwParam2);
            break;
         }
         if(pRequest)
            delete pRequest;
         pRequest = NULL;
         Sleep(0); //libere le CPU pour ne pas traiter les requete en while1....
      }
   }
}


void CTFCServerApp::AddAHRequest(Character *pUser,Character *pTarget,Players *pPlayerP,
                                 DWORD dwRequestID, DWORD dwParam1,DWORD dwParam2,DWORD dwParam3,DWORD dwParam4,DWORD dwParam5,DWORD dwParam6,long lParam7,
                                 CString strParam1, CString strParam2, CString strParam3,DWORD dwParam8)
{
   CAutoLock autoAHRequestLock( &g_ALockAHRequest );  
  
   
   sAHRequest* pNewRequest = new sAHRequest;
   
   pNewRequest->pUser       = pUser;
   pNewRequest->pTarget     = pTarget;
   pNewRequest->pPlayerP    = pPlayerP;
   pNewRequest->dwRequestID = dwRequestID;
   pNewRequest->dwParam1    = dwParam1;
   pNewRequest->dwParam2    = dwParam2;
   pNewRequest->dwParam3    = dwParam3;
   pNewRequest->dwParam4    = dwParam4;
   pNewRequest->dwParam5    = dwParam5;
   pNewRequest->dwParam6    = dwParam6;
   pNewRequest->lParam7     = lParam7;
   pNewRequest->strParam1   = strParam1;
   pNewRequest->strParam2   = strParam2;
   pNewRequest->strParam3   = strParam3;
   pNewRequest->dwParam8    = dwParam8;
   
   PostQueuedCompletionStatus( m_hhAHRequestListIO, 0, reinterpret_cast< DWORD >( pNewRequest ), NULL );
}



void CTFCServerApp::AHR_GetListID(DWORD dwUserID,DWORD dwIsShowDialog)
{
   Players *pUser     = CPlayerManager::GetCharacterRessourceByID(dwUserID); //PM
   if(pUser)
   {
      //now on peu recup les membre de cette guild...
      AuctionMaster::SendAHList(pUser->self,dwIsShowDialog);
      CPlayerManager::FreePlayerResource(pUser);
   }
}




void CTFCServerApp::AHR_AddItemID(DWORD dwUserID, CString strObjType, CString strObjName, DWORD dwQty, DWORD dwEquipPos,
                                  CString strMadeBy,DWORD dwBuyNow,DWORD dwBuyNowNMS, DWORD dwBid, DWORD dwTimeMax, long lCharge)
{
   Players *pUser     = CPlayerManager::GetCharacterRessourceByID(dwUserID);//PM
   if(pUser)
   {
      AuctionMaster::AddSoldItem(pUser,strObjType,strObjName,dwQty,dwEquipPos,strMadeBy,dwBuyNow,dwBuyNowNMS,dwBid,dwTimeMax,lCharge);
      CPlayerManager::FreePlayerResource(pUser);
   }
}


void CTFCServerApp::AHR_BuyItemID(DWORD dwUserID,DWORD dwIndex,DWORD dwBuy,DWORD dwPrix,DWORD dwPrixNMS, DWORD dwTS)
{
   Players *pUser     = CPlayerManager::GetCharacterRessourceByID(dwUserID);//PM
   if(pUser)
   {
      AuctionMaster::BuySoldItem(pUser->self,dwIndex,dwBuy,dwPrix,dwPrixNMS,dwTS);
      CPlayerManager::FreePlayerResource(pUser);
   }
}


void CTFCServerApp::AHR_CancelItemID(DWORD dwUserID,DWORD dwIndex, DWORD dwTS)
{
   Players *pUser     = CPlayerManager::GetCharacterRessourceByID(dwUserID); //PM
   if(pUser)
   {
      AuctionMaster::CancelSoldItem(pUser->self,dwIndex,dwTS);
      CPlayerManager::FreePlayerResource(pUser);
   }
}

void CTFCServerApp::AHR_InfoItemID(DWORD dwUserID,DWORD dwIndex)
{
   Players *pUser     = CPlayerManager::GetCharacterRessourceByID(dwUserID); //PM
   if(pUser)
   {
      AuctionMaster::InfoSoldItem(pUser->self,dwIndex);
      CPlayerManager::FreePlayerResource(pUser);
   }
}




void CTFCServerApp::AHR_ForceExpireID(DWORD dwUserID)
{
   Players *pUser     = CPlayerManager::GetCharacterRessourceByID(dwUserID); //PM
   if(pUser)
   {
      AuctionMaster::ForceExpireAllItems(pUser->self);
      CPlayerManager::FreePlayerResource(pUser);
   }
   
}

void CTFCServerApp::AHR_AddBankInteretID(DWORD dwUserID,DWORD dwGold)
{
   Players *pUser     = CPlayerManager::GetCharacterRessourceByID(dwUserID); //PM
   if(pUser)
   {
      AuctionMaster::AddMoneyGiveList(pUser->self,dwGold);
      CPlayerManager::FreePlayerResource(pUser);
   }
   
}













/***********************************************************************/
//manage de OLD Firewall
//NMNMNM_Firewall In Server
void CTFCServerApp::CloseAllUtilityThread()
{
   //Delete all thread for GUILS, AH and PVP
}


void CTFCServerApp::SendIndirectTalkMessage(Players *pPlayer,char dir,char *pStrMsg)
{
   CString strTmp;
   strTmp.Format("%s",pStrMsg);

   TFCPacket sending;
   sending << (RQ_SIZE)RQ_IndirectTalk;
   sending << (long)pPlayer->self->GetID();
   sending << (char)dir;
   sending << (long)0xBEBE0000;
   sending << (char)0; // not an NPC.
   sending << strTmp;
   sending << pPlayer->self->GetName( pPlayer->self->GetLang() );
   sending << (long)pPlayer->self->ViewFlag(__FLAG_UNIT_COLOR);

   Broadcast::BCast( pPlayer->self->GetWL(), _DEFAULT_RANGE, sending );
}

bool CTFCServerApp::ManageDuelSystem(Players *pPlayerV,Players *pPlayerA)
{
   if(!m_dwDUELSyetemActif || pPlayerV == NULL)
      return false;

   CString strMsgTmp;
   CAutoLock autoDuelLock( &g_ALockDuel );

   UINT dwIDV = pPlayerV->self->GetID();
   if(pPlayerA == NULL) //mort par un monstre ou un SLAY
   {
      // on regarde si la victime est en duel, on a un prob le gars a ete tuer par un monstre ou autres
      for(int i=0;i<m_asDuel.GetSize();i++)
      {
         if(m_asDuel[i].dwIDMaster == dwIDV || m_asDuel[i].dwIDSlave == dwIDV)
         {
            // la victime est en duel mais tuer par autre choses que le membre du duel...
            //master
            Players *lpPlayerM = CPlayerManager::GetCharacterRessourceByID(m_asDuel[i].dwIDMaster);//PM vien de DEATH function donc pas besoin de lock NORMALAMENT OK
            if(lpPlayerM)
            {
               if(m_asDuel[i].dwIDMaster == dwIDV)
                  strMsgTmp.Format(_STR(15219, lpPlayerM->self->GetLang()));
               else
                  strMsgTmp.Format(_STR(15220, lpPlayerM->self->GetLang()));
               lpPlayerM->self->SendInfoMessage(strMsgTmp.GetBuffer(0),0x0570D5);

               CPlayerManager::FreePlayerResource(lpPlayerM);
            }

            // slave
            Players *lpPlayerS = CPlayerManager::GetCharacterRessourceByID(m_asDuel[i].dwIDSlave);//PM vien de DEATH function donc pas besoin de lock NORMALAMENT OK
            if(lpPlayerS)
            {
               if(m_asDuel[i].dwIDSlave == dwIDV)
                  strMsgTmp.Format(_STR(15219, lpPlayerS->self->GetLang()));
               else
                  strMsgTmp.Format(_STR(15220, lpPlayerS->self->GetLang()));
               lpPlayerS->self->SendInfoMessage(strMsgTmp.GetBuffer(0),0x0570D5);
               CPlayerManager::FreePlayerResource(lpPlayerS);
            }

            m_asDuel.RemoveAt(i);
            return false; //return false mort et NON fin du duel...
         }
      }
      return false; //return false ce player n'est pas en duel
   }

   //Mort par un NPC joueur 
   UINT dwIDA = pPlayerA->self->GetID();
   for(int i=0;i<m_asDuel.GetSize();i++)
   {
      //find si la personne morte est dans un duel...
      if((m_asDuel[i].dwIDMaster == dwIDV && m_asDuel[i].dwIDSlave == dwIDA)|| 
         (m_asDuel[i].dwIDMaster == dwIDA && m_asDuel[i].dwIDSlave == dwIDV)   )
      {
         // un duel est completer

         // master
         Players *lpPlayerM = CPlayerManager::GetCharacterRessourceByID(m_asDuel[i].dwIDMaster);//PM vien de DEATH function donc pas besoin de lock NORMALAMENT OK
         if(lpPlayerM)
         {
            CString strMessage;
            if(m_asDuel[i].dwIDMaster == dwIDA)
            {
               lpPlayerM->self->SetFlag(__FLAG_DUEL_WIN,lpPlayerM->self->ViewFlag(__FLAG_DUEL_WIN)+1);
               strMsgTmp.Format(_STR(15221, lpPlayerM->self->GetLang()));
            }
            else
            {
               lpPlayerM->self->SetFlag(__FLAG_DUEL_LOSE,lpPlayerM->self->ViewFlag(__FLAG_DUEL_LOSE)+1);
               strMsgTmp.Format(_STR(15222, lpPlayerM->self->GetLang()));
            }

            lpPlayerM->self->SendInfoMessage(strMsgTmp.GetBuffer(0),0x0570D5);
            lpPlayerM->self->DealExhaust(10000,10000,0);
            CPlayerManager::FreePlayerResource(lpPlayerM);
         }

         // slave
         Players *lpPlayerS = CPlayerManager::GetCharacterRessourceByID(m_asDuel[i].dwIDSlave);//PM vien de DEATH function donc pas besoin de lock NORMALAMENT OK
         if(lpPlayerS)
         {
            CString strMessage;
            if(m_asDuel[i].dwIDSlave == dwIDA)
            {
               lpPlayerS->self->SetFlag(__FLAG_DUEL_WIN,lpPlayerS->self->ViewFlag(__FLAG_DUEL_WIN)+1);
               strMsgTmp.Format(_STR(15221, lpPlayerS->self->GetLang()));
            }
            else
            {
               lpPlayerS->self->SetFlag(__FLAG_DUEL_LOSE,lpPlayerS->self->ViewFlag(__FLAG_DUEL_LOSE)+1);
               strMsgTmp.Format(_STR(15222, lpPlayerS->self->GetLang()));
            }

            lpPlayerS->self->SendInfoMessage(strMsgTmp.GetBuffer(0),0x0570D5);
            lpPlayerS->self->DealExhaust(10000,10000,0);

            CPlayerManager::FreePlayerResource(lpPlayerS);
         }
         m_asDuel.RemoveAt(i);
         return true; //on est en duel.
      }
      else
      {
         //check si le duel a ete brisee par la mort externe de 1 ou de lautre des participant
         if(m_asDuel[i].dwIDMaster == dwIDV || m_asDuel[i].dwIDSlave == dwIDV)
         {
            // la victime est en duel mais tuer par autre choses que le membre du duel...
            //master
            Players *lpPlayerM = CPlayerManager::GetCharacterRessourceByID(m_asDuel[i].dwIDMaster);//PM vien de DEATH function donc pas besoin de lock NORMALAMENT OK
            if(lpPlayerM)
            {
               if(m_asDuel[i].dwIDMaster == dwIDV)
                  strMsgTmp.Format(_STR(15223, lpPlayerM->self->GetLang()));
               else
                  strMsgTmp.Format(_STR(15224, lpPlayerM->self->GetLang()));
               lpPlayerM->self->SendInfoMessage(strMsgTmp.GetBuffer(0),0x0570D5);

               CPlayerManager::FreePlayerResource(lpPlayerM);
            }
            

            // slave
            Players *lpPlayerS = CPlayerManager::GetCharacterRessourceByID(m_asDuel[i].dwIDSlave);//PM vien de DEATH function donc pas besoin de lock NORMALAMENT OK
            if(lpPlayerS)
            {
               if(m_asDuel[i].dwIDSlave == dwIDV)
                  strMsgTmp.Format(_STR(15223, lpPlayerS->self->GetLang()));
               else
                  strMsgTmp.Format(_STR(15224, lpPlayerS->self->GetLang()));

               lpPlayerS->self->SendInfoMessage(strMsgTmp.GetBuffer(0),0x0570D5);
               CPlayerManager::FreePlayerResource(lpPlayerS);
            }

            m_asDuel.RemoveAt(i);
            return false; //return false mort et NON fin du duel...
         }
      }
   }

   return false;
}


void CTFCServerApp::ManagePVPSystem(Players *pPlayerV,Players *pPlayerA,double &dPCDrop)
{
   CString strTmp;

   dPCDrop  = 0.00;

   if( theApp.dwPVPDropDisabled ) //on ne manage plus le systeme PVP qwuand le server est en PVP OFF
      return;

   if(pPlayerV == NULL || pPlayerA == NULL)
      return;
  
   if(m_dwPVPSyetem2Actif == 1) 
   {

      WorldPos wlPos = {0,0,0};
      int iCombatModeA = 0;
      int iCombatModeV = 0;

      if(pPlayerA && pPlayerA->self)
         iCombatModeA = pPlayerA->self->GetNMCombatMode();
      if(pPlayerV && pPlayerV->self)
         iCombatModeV = pPlayerV->self->GetNMCombatMode();


      //Les 2 joueur sont en mode combat c<est un combat consenti.. +1 HONOR
      int iCurTime = time(NULL);
      if(iCombatModeA > 0  && iCombatModeV > 0)
      {
         int iHonor = pPlayerA->self->GetHonor();
         if(iHonor <10)
         {
            iHonor++;
            pPlayerA->self->SetHonor(iHonor);
         }
         pPlayerA->self->SetFlag(__FLAG_NMS_LAST_HONOR_TIME,iCurTime);
      }

      //La victime ne voulait pas combatter combat NOn Consenti le gagnant se prend un point de crime
      else if(iCombatModeA > 0  && iCombatModeV ==0)
      {
         int iCrime = pPlayerA->self->GetCrime();
         if(iCrime <10)
         {
            iCrime++;
            pPlayerA->self->SetCrime(iCrime);
         }
         pPlayerA->self->SetFlag(__FLAG_NMS_LAST_CRIME_TIME,iCurTime);

      }

      int pdhA = pPlayerA->self->GetHonor();
      int pdhC = pPlayerV->self->GetHonor();
      int pdcC = pPlayerV->self->GetCrime();
      int lvlA = pPlayerA->self->GetLevel();
      int lvlC = pPlayerV->self->GetLevel();

      //Ajouter good formule
      if((lvlA - lvlC) >50)
         dPCDrop = (double)pdcC;
      else
         dPCDrop = 5.00+(((double)(pdhA*pdhC)/10.00) + ((double)(pdcC*(pdhA/10.00+1.00))));


      strTmp.Format(_STR(15443, pPlayerV->self->GetLang()),pPlayerA->self->GetTrueName(),dPCDrop);
      pPlayerV->self->SendInfoMessage(strTmp.GetBuffer(0),0x3c99e8);

      strTmp.Format(_STR(15444, pPlayerA->self->GetLang()),pPlayerV->self->GetTrueName(),dPCDrop);
      pPlayerA->self->SendInfoMessage(strTmp.GetBuffer(0),0x3c99e8);

   }
   return;
}


bool CTFCServerApp::CheckNMToolsCommand(Players *pPlayer,char Dir, std::string &Message)
{
   CString strMsgTmp;

   if (Message[0]=='.')
   {
      if (_stricmp(Message.c_str(), ".HELP") == 0)
      {
         strMsgTmp.Format(_STR(15176, pPlayer->self->GetLang()));
         pPlayer->self->SendSystemMessage(strMsgTmp.GetBuffer(0));

         //pPlayer->self->SendSystemMessage(".NOIR");
         pPlayer->self->SendSystemMessage(".LEVELUP");
         pPlayer->self->SendSystemMessage(".ROLL");
         pPlayer->self->SendSystemMessage(".DICE nbrface");
         if(m_dwModeRPorHRP)
         {
            pPlayer->self->SendSystemMessage(".HRP / .RP");
            pPlayer->self->SendSystemMessage(".HRP text / .RP text");
         }
         if(m_dwXPstat)
         {
            pPlayer->self->SendSystemMessage(".XPRESET / .XPSTAT");
         }
         if(m_dwDUELSyetemActif)
         {
            pPlayer->self->SendSystemMessage(".DUEL pj  / .ACCEPT / .REFUSE");
            pPlayer->self->SendSystemMessage(".DUELVIEW");
         }
         if(m_dwCCShortcut)
         {
            strMsgTmp.Format(_STR(15185, pPlayer->self->GetLang()));
            pPlayer->self->SendSystemMessage(strMsgTmp.GetBuffer(0));
         }
         if(m_dwPseudoname)
         {
            pPlayer->self->SendSystemMessage(".PSEUDO / .PSEUDO pname / .NOPSEUDO");
         }

         
         /*
         if(m_dwPVPSyetem2Actif == 1)
            pPlayer->self->SendSystemMessage(".PVPSYSTEM");
         */
         

         pPlayer->self->SendSystemMessage("<>");
         if(pPlayer->IsGod())
         {
            pPlayer->self->SendSystemMessage("\"GM Command\"");

            /*
            if(m_dwPVPSyetemOldActif == 1)
               pPlayer->self->SendSystemMessage(".HLL");
            */

            pPlayer->self->SendSystemMessage(".DPSRESET / .DPSSTAT");
            pPlayer->self->SendSystemMessage(".GO txt / .GET txt / .RETURN");
            pPlayer->self->SendSystemMessage(".GETPOS / .GETPOS name");
            pPlayer->self->SendSystemMessage(".GMSHOW / .GMHIDE");
            pPlayer->self->SendSystemMessage(".CCSHOW / .CCHIDE");
            pPlayer->self->SendSystemMessage(".MONSTER / .NOMONSTER");
            pPlayer->self->SendSystemMessage(".DEV / .NODEV");
            pPlayer->self->SendSystemMessage(".DIE / .NODIE");
            pPlayer->self->SendSystemMessage(".CLIP / .NOCLIP");
            pPlayer->self->SendSystemMessage(".ACCOUNT / .TITLE");
            pPlayer->self->SendSystemMessage(".NAME");
            pPlayer->self->SendSystemMessage(".PNAME");
            pPlayer->self->SendSystemMessage(".TITLE");
            pPlayer->self->SendSystemMessage(".TAKEALL");
            pPlayer->self->SendSystemMessage(".XP pj");
            pPlayer->self->SendSystemMessage(".SHOWID ON/OFF");
            pPlayer->self->SendSystemMessage(".SPOOF OFF/LAST/id");
            pPlayer->self->SendSystemMessage(".GETAREALINK world");
            pPlayer->self->SendSystemMessage("<>");
            pPlayer->self->SendSystemMessage(".GETCOMMSTAT //return some COMM global info");
            pPlayer->self->SendSystemMessage(".GETUNITMAPSIZE //return the size of unit_map");
            pPlayer->self->SendSystemMessage("<>");
         }
         return true;
      }

      
      else if (_stricmp(Message.c_str(), ".LEVELUP") == 0)
      {
         pPlayer->self->SetFlag(__FLAG_FORCE_LEVELUP_REROLL, 1);		
         return true;
      }

      else if (_stricmp(Message.c_str(), ".MINION Show") == 0)
      {
         //toggle le minions test
         pPlayer->self->Lock();
         pPlayer->self->ShowMinion(true);	
         pPlayer->self->Unlock();
         return true;
      }
      else if (_stricmp(Message.c_str(), ".MINION Hide") == 0)
      {
         //toggle le minions test
         pPlayer->self->Lock();
         pPlayer->self->ShowMinion(false);	
         pPlayer->self->Unlock();
         return true;
      }
      
      else if (_stricmp(Message.c_str(), ".HRP") == 0)
      {
         if(m_dwModeRPorHRP)
         {
            if(pPlayer->self->ViewFlag(__FLAG_RPHRP_BLOCK) == 1)
            {
               strMsgTmp.Format(_STR(15177, pPlayer->self->GetLang()));
               pPlayer->self->SendSystemMessage(strMsgTmp.GetBuffer(0));
               return true;
            }
            else if(pPlayer->self->ViewFlag(__FLAG_RPHRP_STATUS) == 0)
            {
               strMsgTmp.Format(_STR(15178, pPlayer->self->GetLang()));
               pPlayer->self->SendSystemMessage(strMsgTmp.GetBuffer(0));
               return true;
            }
            else if(pPlayer->self && pPlayer->self->GetNMCombatMode()) //a valider si faut mettre la liste...
            {
               strMsgTmp.Format(_STR(15179, pPlayer->self->GetLang()));
               pPlayer->self->SendSystemMessage(strMsgTmp.GetBuffer(0));
               return true;
            }

            time_t TimeMsTmp =  time(NULL);
            if(TimeMsTmp > pPlayer->self->ViewFlag(__FLAG_RPHRP_TIME)+NM_TIME_DELAY_HPHRP || pPlayer->IsGod())
            {
               pPlayer->self->SetFlag(__FLAG_RPHRP_STATUS,0);
               pPlayer->self->SetFlag(__FLAG_RPHRP_TIME,TimeMsTmp);
               strMsgTmp.Format(_STR(15180, pPlayer->self->GetLang()));
               pPlayer->self->SendSystemMessage(strMsgTmp.GetBuffer(0));
               pPlayer->self->BroadcastPopup( pPlayer->self->GetWL(), true );
            }
            else
            {
               int iReste = (pPlayer->self->ViewFlag(__FLAG_RPHRP_TIME)+NM_TIME_DELAY_HPHRP) -TimeMsTmp;
               int iResteH = iReste/3600;
               int iResteM = (iReste-(iResteH*3600))/60;
               int iResteS = (iReste-((iResteH*3600)+(iResteM*60)));
            }
            return true;
         }
      }
      else if (_strnicmp(Message.c_str(), ".HRP ", 5) == 0)
      {
         if(m_dwModeRPorHRP)
         {
            std::string strNewMsg = "[ " + Message .substr(5) + " ]";
            SendIndirectTalkMessage(pPlayer,0, (char*)strNewMsg.c_str());
            return true;
         }
      }
      else if (_stricmp(Message.c_str(), ".RP") == 0)
      {
         if(m_dwModeRPorHRP)
         {
            int dwCrime  = pPlayer->self->GetCrime();
            if(pPlayer->self->ViewFlag(__FLAG_RPHRP_BLOCK) == 1)
            {
               strMsgTmp.Format(_STR(15177, pPlayer->self->GetLang()));
               pPlayer->self->SendSystemMessage(strMsgTmp.GetBuffer(0));
               return true;
            }
            else if(pPlayer->self->ViewFlag(__FLAG_RPHRP_STATUS) == 1)
            {
               strMsgTmp.Format(_STR(15181, pPlayer->self->GetLang()));
               pPlayer->self->SendSystemMessage(strMsgTmp.GetBuffer(0));
               return true;
            }
            else if(pPlayer->self && pPlayer->self->GetNMCombatMode()) //a valider si faut mettre la liste...
            {
               
               strMsgTmp.Format(_STR(15179, pPlayer->self->GetLang()));
               pPlayer->self->SendSystemMessage(strMsgTmp.GetBuffer(0));
               return true;
            }

            time_t TimeMsTmp =  time(NULL);
            if(TimeMsTmp > pPlayer->self->ViewFlag(__FLAG_RPHRP_TIME)+NM_TIME_DELAY_HPHRP || pPlayer->IsGod())
            {
               pPlayer->self->SetFlag(__FLAG_RPHRP_STATUS,1);
               pPlayer->self->SetFlag(__FLAG_RPHRP_TIME,TimeMsTmp);
               
               strMsgTmp.Format(_STR(15182, pPlayer->self->GetLang()));
               pPlayer->self->SendSystemMessage(strMsgTmp.GetBuffer(0));
               pPlayer->self->BroadcastPopup( pPlayer->self->GetWL(), true );
            }
            else
            {
               int iReste = (pPlayer->self->ViewFlag(__FLAG_RPHRP_TIME)+NM_TIME_DELAY_HPHRP) -TimeMsTmp;
               int iResteH = iReste/3600;
               int iResteM = (iReste-(iResteH*3600))/60;
               int iResteS = (iReste-((iResteH*3600)+(iResteM*60)));
            }
            return true;
         }
      }
      else if (_strnicmp(Message.c_str(), ".RP ", 4) == 0)
      {
         if(m_dwModeRPorHRP)
         {
            SendIndirectTalkMessage(pPlayer,0, (char*)Message .substr(4).c_str());
            return true;
         }
      }
      else if (_stricmp(Message.c_str(), ".XPRESET") == 0)
      {
         if(m_dwXPstat)
         {
            pPlayer->m_dwDPSLastTickCounter = pPlayer->m_dwXPCurrentTick;
            pPlayer->m_XPCounter = pPlayer->self->GetXP();
            strMsgTmp.Format(_STR(15183, pPlayer->self->GetLang()));
            pPlayer->self->SendSystemMessage(strMsgTmp.GetBuffer(0));
            return true;
         }
      }
      else if (_stricmp(Message.c_str(), ".XPSTAT") == 0)
      {
         if(m_dwXPstat)
         {
            float XPtime = (pPlayer->m_dwXPCurrentTick - pPlayer->m_dwDPSLastTickCounter) / 1000.0;
            if (XPtime > 0)
            {
               CString UnitXPtime, UnitXPtotal, UnitXPheure;
               float XPtotal = ((__int64) pPlayer->self->GetXP() - (__int64) pPlayer->m_XPCounter) / 1.0;
               float XPheure = 3600.0 * XPtotal / XPtime;

               if (XPtime > 60)
                  UnitXPtime.Format("%9.0f min.", XPtime / 60.0 );
               else
                  UnitXPtime.Format("%9.0f sec.", XPtime / 1.0);

               if (XPtotal > 1000000 || XPtotal < -1000000)
                  UnitXPtotal.Format("%9.2f M", XPtotal / 1000000.0 );
               else if (XPtotal > 1000 || XPtotal < -1000)
                  UnitXPtotal.Format("%9.2f K", XPtotal / 1000.0 );
               else
                  UnitXPtotal.Format("%9.0f", XPtotal / 1.0);

               if (XPheure > 1000000 || XPheure < -1000000)
                  UnitXPheure.Format("%9.2f M", XPheure / 1000000.0 );
               else if (XPheure > 1000 || XPheure < -1000)
                  UnitXPheure.Format("%9.2f K", XPheure / 1000.0 );
               else
                  UnitXPheure.Format("%9.0f", XPheure / 1.0);

               strMsgTmp.Format(_STR(15184, pPlayer->self->GetLang()),UnitXPtime.GetBuffer(0), UnitXPtotal.GetBuffer(0), UnitXPheure.GetBuffer(0));
               pPlayer->self->SendSystemMessage(strMsgTmp.GetBuffer(0));
            }
            return true;
         }
      }
      ///////////////////////////////////////////////////////
      else if (_stricmp(Message.c_str(), ".DPSRESET") == 0)
      {
         if(pPlayer->IsGod())
         {
            pPlayer->m_dwDPSLastTickCounter = pPlayer->m_dwDPSCurrentTick;
            pPlayer->m_DPSCounter = 0;
            strMsgTmp.Format(_STR(15258, pPlayer->self->GetLang()));
            pPlayer->self->SendSystemMessage(strMsgTmp.GetBuffer(0));
         }
         return true;
      }
      else if (_stricmp(Message.c_str(), ".DPSSTAT") == 0)
      {
         if(pPlayer->IsGod())
         {
            float DPStime = (pPlayer->m_dwDPSCurrentTick - pPlayer->m_dwDPSLastTickCounter) / 1000.0;
            if (DPStime > 0)
            {
               CString UnitDPStime, UnitDPStotal, UnitDPSheure;
               float DPStotal = ((__int64) pPlayer->m_DPSCounter) / 1.0;
               float DPSSec = DPStotal / DPStime;

               strMsgTmp.Format(_STR(15259, pPlayer->self->GetLang()),DPSSec);
               pPlayer->self->SendSystemMessage(strMsgTmp.GetBuffer(0));
            }
         }
         return true;
      }
      ///////////////////////////////////////
      else if (_stricmp(Message.c_str(), ".ROLL") == 0)
      {
         int dwRollVal = rand()%100;
         CString strMessageRoll;
         strMessageRoll.Format(_STR(15186, pPlayer->self->GetLang()),pPlayer->self->GetTrueName().GetBuffer(0),dwRollVal);
         CPlayerManager::SendNeerUnitMessage(pPlayer->self->GetWL().X,pPlayer->self->GetWL().Y,pPlayer->self->GetWL().world,strMessageRoll);
         return true;
      }

     else if (_strnicmp(Message.c_str(), ".DICE ", 6) == 0)
	  {
         int iNbrFace =  atoi(Message.substr(6).c_str()); 
         if(iNbrFace<=0)
            iNbrFace = 100;
         int dwRollVal = rand()%iNbrFace;
         CString strMessageRoll;
		   strMessageRoll.Format(_STR(15359, pPlayer->self->GetLang()),pPlayer->self->GetName().GetBuffer(0),iNbrFace,dwRollVal);
         CPlayerManager::SendNeerUnitMessage(pPlayer->self->GetWL().X,pPlayer->self->GetWL().Y,pPlayer->self->GetWL().world,strMessageRoll);
         return true;
	  }
      /*
      else if (_stricmp(Message.c_str(), ".HLL") == 0)
      {
         if(m_dwPVPSyetemOldActif == 1 && pPlayer->IsGod()) //NMNMNM_PVP3
         {
            CString strMsg;
            strMsg.Format(_STR(15187, pPlayer->self->GetLang()),_STR(15188, pPlayer->self->GetLang()));
            pPlayer->self->SendSystemMessage(strMsg.GetBuffer(0));
  
            int dwNbr = CPlayerManager::SendHLLListTo(pPlayer);
            if(dwNbr == 0)
            {
               CString strName;
               strName.Format("- - - -");
               pPlayer->self->SendSystemMessage(strName.GetBuffer(0));
            }
         }
         return true;
      }
      else if (_stricmp(Message.c_str(), ".PVPSYSTEM") == 0)
      {
         if(m_dwPVPSyetem2Actif == 1) //NMNMNM_PVP3
         {
            pPlayer->self->SendInfoMessage  (_STR(15240, pPlayer->self->GetLang()),0xFFFFFF);
            pPlayer->self->SendInfoMessage  ("----------------------------------------",0xFFFFFF);
            pPlayer->self->SendSystemMessage(_STR(15460, pPlayer->self->GetLang()));
            pPlayer->self->SendSystemMessage(_STR(15461, pPlayer->self->GetLang()));
            pPlayer->self->SendSystemMessage(_STR(15462, pPlayer->self->GetLang()));
            pPlayer->self->SendSystemMessage(_STR(15463, pPlayer->self->GetLang()));
            pPlayer->self->SendSystemMessage(_STR(15464, pPlayer->self->GetLang()));
            pPlayer->self->SendSystemMessage(_STR(15465, pPlayer->self->GetLang()));
            pPlayer->self->SendSystemMessage(_STR(15466, pPlayer->self->GetLang()));
            pPlayer->self->SendSystemMessage(_STR(15467, pPlayer->self->GetLang()));
            pPlayer->self->SendInfoMessage  ("----------------------------------------",0xFFFFFF);
            pPlayer->self->SendSystemMessage(_STR(15250, pPlayer->self->GetLang()));
            pPlayer->self->SendInfoMessage  (_STR(15251, pPlayer->self->GetLang()),0x2020FF);
            return true;
         }
      }
      */
      else if (_stricmp(Message.c_str(), ".NOPSEUDO") == 0)
      {
         if(m_dwPseudoname)
         {
            pPlayer->m_strPseudo = "";
            strMsgTmp.Format(_STR(15189, pPlayer->self->GetLang()));
            pPlayer->self->SendSystemMessage(strMsgTmp.GetBuffer(0));
            return true;
         }
      }
      else if (_stricmp(Message.c_str(), ".PSEUDO") == 0)
      {
         if(m_dwPseudoname)
         {
            if (pPlayer->m_strPseudo == "")
            {
               strMsgTmp.Format(_STR(15190, pPlayer->self->GetLang()));
               pPlayer->self->SendSystemMessage(strMsgTmp.GetBuffer(0));
            }
            else
            {
               strMsgTmp.Format(_STR(15191, pPlayer->self->GetLang()),pPlayer->m_strPseudo);
               pPlayer->self->SendSystemMessage(strMsgTmp.GetBuffer(0));
            }
            return true;
         }
      }

      else if (_strnicmp(Message.c_str(), ".PSEUDO ", 8) == 0)
      {
         if(m_dwPseudoname)
         {
            if (Message.substr(8).size() <= 80 && Message.substr(8).size() >= 3)
            {
               pPlayer->m_strPseudo.Format("%s",Message.substr(8).c_str());
               strMsgTmp.Format(_STR(15191, pPlayer->self->GetLang()),pPlayer->m_strPseudo);
               pPlayer->self->SendSystemMessage(strMsgTmp.GetBuffer(0));
            }
            else
            {
               strMsgTmp.Format(_STR(15192, pPlayer->self->GetLang()));
               pPlayer->self->SendSystemMessage(strMsgTmp.GetBuffer(0));
            }
            return true;
         }
      }
      else if (_strnicmp(Message.c_str(), ".DUELVIEW", 9) == 0)
      {
         if(m_dwDUELSyetemActif)
         {
            strMsgTmp.Format(_STR(15196, pPlayer->self->GetLang()),pPlayer->self->ViewFlag(__FLAG_DUEL_WIN),pPlayer->self->ViewFlag(__FLAG_DUEL_LOSE));
            pPlayer->self->SendSystemMessage(strMsgTmp.GetBuffer(0));
            return true;
         }
      }
      else if (_strnicmp(Message.c_str(), ".DUEL ", 6) == 0)
      {
         if(m_dwDUELSyetemActif)
         {
            CAutoLock autoDuelLock( &g_ALockDuel );
            std::string Player = Message.substr(6);

            Players *lpPlayer;
            lpPlayer = CPlayerManager::GetCharacterOld( Player.c_str() );//lock pas command User
            if(lpPlayer)
            {
               // on dois verifier si vous etes deja en duel...
               for(int i=0;i<m_asDuel.GetSize();i++)
               {
                  if(m_asDuel[i].dwIDMaster == pPlayer->self->GetID() || m_asDuel[i].dwIDSlave == pPlayer->self->GetID())
                  {
                     strMsgTmp.Format(_STR(15197, pPlayer->self->GetLang()));
                     pPlayer->self->SendInfoMessage(strMsgTmp.GetBuffer(0),0x0570D5);
                     return true;
                  }
                  else if(m_asDuel[i].dwIDMaster == lpPlayer->self->GetID() || m_asDuel[i].dwIDSlave == lpPlayer->self->GetID())
                  {
                     strMsgTmp.Format(_STR(15198, pPlayer->self->GetLang()));
                     pPlayer->self->SendInfoMessage(strMsgTmp.GetBuffer(0),0x0570D5);
                     return true;
                  }
               }

               //NMNMNM
               // on dois verifier la proximite des 2 joueurs...
               bool bOK = false;
               WorldPos wlM = pPlayer->self->GetWL();
               WorldPos wlS = lpPlayer->self->GetWL();
               if(wlM.world == wlS.world)
               {
                  //deja on est dans le meme monde
                  int dwX = abs(wlM.X - wlS.X);
                  int dwY = abs(wlM.Y - wlS.Y);
                  if(dwX <30 && dwY <30)
                  {
                     // on est pres de lui...
                     bOK = true;
                  }
               }
               if(bOK)
               {
                  //on peu construire le Duel..
                  sDuel newDuel;
                  newDuel.dwIDMaster  = pPlayer->self->GetID();
                  newDuel.dwIDSlave   = lpPlayer->self->GetID();
                  newDuel.dwStartTime = 0;
                  newDuel.dwAccepTime = GetTickCount(); 
                  m_asDuel.Add(newDuel);

                  strMsgTmp.Format(_STR(15199, lpPlayer->self->GetLang()),pPlayer->self->GetTrueName());
                  lpPlayer->self->SendInfoMessage(strMsgTmp.GetBuffer(0),0x0570D5);

                  strMsgTmp.Format(_STR(15200, pPlayer->self->GetLang()),lpPlayer->self->GetTrueName());
                  pPlayer->self->SendInfoMessage(strMsgTmp.GetBuffer(0),0x0570D5);
               }
               else
               {
                  strMsgTmp.Format(_STR(15201, pPlayer->self->GetLang()),lpPlayer->self->GetTrueName());
                  pPlayer->self->SendInfoMessage(strMsgTmp.GetBuffer(0),0x0570D5);
               }
               return true;
            }
            else
            {
               strMsgTmp.Format(_STR(15202, pPlayer->self->GetLang()));
               pPlayer->self->SendInfoMessage(strMsgTmp.GetBuffer(0),0x0570D5);
               return true;
            }
         }
      }
      else if (_stricmp(Message.c_str(), ".ACCEPT") == 0)
      {
         if(m_dwDUELSyetemActif)
         {
            CAutoLock autoDuelLock( &g_ALockDuel );
            CAutoLock autoDuelDLock( &g_ALockDuelD );
            // on dois verifier que vous etes en attente de duel...
            for(int i=0;i<m_asDuel.GetSize();i++)
            {
               if(m_asDuel[i].dwAccepTime >0 && m_asDuel[i].dwIDSlave == pPlayer->self->GetID())
               {
                  //il accepte le duel peu commencer...
                  m_asDuel[i].dwStartTime = GetTickCount(); 
                  m_asDuel[i].dwAccepTime = 0;

                  strMsgTmp.Format(_STR(15203, pPlayer->self->GetLang()));
                  pPlayer->self->SendInfoMessage(strMsgTmp.GetBuffer(0),0x0570D5);

                  Players *lpPlayer = CPlayerManager::GetCharacterOldByID(m_asDuel[i].dwIDMaster);//command user ,lock pas...
                  if(lpPlayer)
                  {
                     strMsgTmp.Format(_STR(15203, lpPlayer->self->GetLang()));
                     lpPlayer->self->SendInfoMessage(strMsgTmp.GetBuffer(0),0x0570D5);

                     //dois lancer le thread de decompte
                     sDuelDecompte newDecompte;
                     newDecompte.uiIDM     = m_asDuel[i].dwIDMaster;
                     newDecompte.uiIDS     = m_asDuel[i].dwIDSlave;
                     newDecompte.dwCmpteur = 10;

                     m_asDuelDecompte.Add(newDecompte);
                  }
                  else
                  {
                     strMsgTmp.Format(_STR(15204, pPlayer->self->GetLang()));
                     pPlayer->self->SendInfoMessage(strMsgTmp.GetBuffer(0),0x0570D5);
                  }
                  return true;
               }
            }
            strMsgTmp.Format(_STR(15204, pPlayer->self->GetLang()));
            pPlayer->self->SendInfoMessage(strMsgTmp.GetBuffer(0),0x0570D5);
            return true;
         }
      }
      else if (_stricmp(Message.c_str(), ".REFUSE") == 0)
      {
         if(m_dwDUELSyetemActif)
         {
            CAutoLock autoDuelLock( &g_ALockDuel );
            // on dois verifier que vous etes en attente de duel...
            for(int i=0;i<m_asDuel.GetSize();i++)
            {
               if(m_asDuel[i].dwAccepTime >0 && m_asDuel[i].dwIDSlave == pPlayer->self->GetID())
               {
                  //il refuse le duel
                  int dwMasterID = m_asDuel[i].dwIDMaster;
                  m_asDuel.RemoveAt(i);

                  Players *lpPlayer = CPlayerManager::GetCharacterOldByID(dwMasterID); //command user on lock pas...
                  if(lpPlayer)
                  {
                     strMsgTmp.Format(_STR(15205, lpPlayer->self->GetLang()),pPlayer->self->GetTrueName());
                     lpPlayer->self->SendInfoMessage(strMsgTmp.GetBuffer(0),0x0570D5);
                  }
                  else
                  {
                     strMsgTmp.Format(_STR(15173, pPlayer->self->GetLang()));
                     pPlayer->self->SendInfoMessage(strMsgTmp.GetBuffer(0),0x0570D5);
                  }
                  return true;
               }
            }
            strMsgTmp.Format(_STR(15173, pPlayer->self->GetLang()));
            pPlayer->self->SendInfoMessage(strMsgTmp.GetBuffer(0),0x0570D5);
            return true;
         }
      }
      else if (_strnicmp(Message.c_str(), ".GO ", 4) == 0)
      {
         if(pPlayer->IsGod())
         {
            std::string Msg = "teleport to " + Message.substr(4);
            SysopCmd::VerifySysopCommand( pPlayer, (char*)Msg.c_str() );
            return true;
         }
         return true;
      }
      else if (_strnicmp(Message.c_str(), ".GET ", 5) == 0)
      {
         if(pPlayer->IsGod())
         {
            Players *lpPlayer;
            lpPlayer = CPlayerManager::GetCharacterOld(Message.substr(5).c_str());//pas besoin lock Command GM
            if(lpPlayer)
            {
               lpPlayer->SetLastGetPos(lpPlayer->self->GetWL().X,lpPlayer->self->GetWL().Y,lpPlayer->self->GetWL().world);
               m_strReturnName = Message.substr(5);

               strMsgTmp.Format("teleport user %s to %i,%i,%i",Message.substr(5).c_str(),pPlayer->self->GetWL().X - 1 , pPlayer->self->GetWL().Y + 1, pPlayer->self->GetWL().world);
               SysopCmd::VerifySysopCommand( pPlayer, strMsgTmp.GetBuffer(0) );
            }
            else
            {
               strMsgTmp.Format(_STR(15202, pPlayer->self->GetLang()));
               pPlayer->self->SendInfoMessage(strMsgTmp.GetBuffer(0),0x0570D5);
            }
            return true;
         }
      }
      else if (_strnicmp(Message.c_str(), ".RETURN ", 8) == 0)
      {
         if(pPlayer->IsGod())
         {
            CString strName = Message.substr(8).c_str();
            if(strName.CompareNoCase("last") == 0)
            {
               strName = m_strReturnName.c_str();
            }
               
            Players *lpPlayer;
            lpPlayer = CPlayerManager::GetCharacterOld(strName.GetBuffer());//pas besoin lock Command GM
            if(lpPlayer)
            {
               WorldPos wlRet;
               lpPlayer->GetLastGetPos(wlRet);
               lpPlayer->SetLastGetPos(0,0,0);
               if(wlRet.X == 0 && wlRet.Y == 0 && wlRet.world == 0)
               {
                  strMsgTmp.Format(_STR(15230, pPlayer->self->GetLang()));
                  pPlayer->self->SendInfoMessage(strMsgTmp.GetBuffer(0),0x0570D5);
               }
               else
               {
                  strMsgTmp.Format("teleport user %s to %i,%i,%i",strName.GetBuffer(0),wlRet.X, wlRet.Y, wlRet.world);
                  SysopCmd::VerifySysopCommand( pPlayer, strMsgTmp.GetBuffer(0) );
               }
            }
         }
         return true;
      }
      else if (_stricmp(Message.c_str(), ".GETPOS") == 0)
      {
         if(pPlayer->IsGod())
         {
            strMsgTmp.Format(_STR(15206, pPlayer->self->GetLang()),pPlayer->self->GetTrueName().GetBuffer(0),pPlayer->self->GetID(),
                                                                   pPlayer->self->GetWL().X,pPlayer->self->GetWL().Y,pPlayer->self->GetWL().world);
            pPlayer->self->SendSystemMessage(strMsgTmp.GetBuffer(0));
            return true;
         }
      }
      else if (_strnicmp(Message.c_str(), ".GETPOS ", 8) == 0)
      {
         if(pPlayer->IsGod())
         {
            std::string Player = Message.substr(8);

            Players *lpPlayer;
            lpPlayer = CPlayerManager::GetCharacterOld( Player.c_str() );//pas besoin lock Command GM
            if(lpPlayer)
            {
               strMsgTmp.Format(_STR(15206, pPlayer->self->GetLang()),lpPlayer->self->GetTrueName().GetBuffer(0),lpPlayer->self->GetID(),
                                                                      lpPlayer->self->GetWL().X,lpPlayer->self->GetWL().Y,lpPlayer->self->GetWL().world);
               pPlayer->self->SendSystemMessage(strMsgTmp.GetBuffer(0));
               return true;
            }
            else
            {
               
               strMsgTmp.Format(_STR(15202, pPlayer->self->GetLang()));
               pPlayer->self->SendSystemMessage(strMsgTmp.GetBuffer(0));
               return true;
            }
         }
      }
      else if (_strnicmp(Message.c_str(), ".XP ", 4) == 0)
      {
         if(pPlayer->IsGod())
         {
            std::string Player = Message.substr(4);

            Players *lpPlayer;
            lpPlayer = CPlayerManager::GetCharacterOld( Player.c_str() );//pas besoin lock Command GM
            if(lpPlayer)
            {
               char buf[256];
               WORD wLevel = lpPlayer->self->GetLevel();
               __int64 nextLevelXP      = Character::sm_n64XPchart[wLevel];
               __int64 XPResteFait      = lpPlayer->self->XPtoLevel();
               __int64 XPFait           = nextLevelXP - XPResteFait;

               CString strXPT;
               CString strXPF;
               CString strXPPC;

               _i64toa_s(nextLevelXP,buf,256,10);
               strXPT.Format("%s",buf);
               _i64toa_s(XPFait,buf,256,10);
               strXPF.Format("%s",buf);
               strXPPC.Format("%.02f",(double)((XPFait/nextLevelXP)*100));


               strMsgTmp.Format(_STR(15207, pPlayer->self->GetLang()),lpPlayer->self->GetTrueName(),lpPlayer->self->GetID(),
                                                                      strXPF,strXPT,atoi(strXPPC.GetBuffer(0)),wLevel);
               pPlayer->self->SendSystemMessage(strMsgTmp.GetBuffer(0));
               return true;
            }
            else
            {
               strMsgTmp.Format(_STR(15202, pPlayer->self->GetLang()));
               pPlayer->self->SendSystemMessage(strMsgTmp.GetBuffer(0));
               return true;
            }
         }
      }
      else if (_strnicmp(Message.c_str(), ".SHOWID ON", 10) == 0)
      {
         if(pPlayer->IsGod())
         {
            pPlayer->m_dwShowID = 1;
            
            strMsgTmp.Format(_STR(15209, pPlayer->self->GetLang()));
            pPlayer->self->SendSystemMessage(strMsgTmp.GetBuffer(0));
            pPlayer->self->Teleport(pPlayer->self->GetWL(),0);
            return true;
         }
      }
      else if (_strnicmp(Message.c_str(), ".SHOWID OFF", 11) == 0)
      {
         if(pPlayer->IsGod())
         {
            pPlayer->m_dwShowID = 0;

            strMsgTmp.Format(_STR(15210, pPlayer->self->GetLang()));
            pPlayer->self->SendSystemMessage(strMsgTmp.GetBuffer(0));
            pPlayer->self->Teleport(pPlayer->self->GetWL(),0);
            return true;
         }
      }

      else if (_strnicmp(Message.c_str(), ".SPOOF OFF", 10) == 0)
      {
         if(pPlayer->IsGod())
         {
            pPlayer->m_dwSpoofedID      = 0;

            pPlayer->self->SetFlag(__FLAG_UNIT_COLOR    ,pPlayer->self->ViewFlag(__FLAG_UNIT_COLOR_OLD));
            pPlayer->self->SetFlag(__FLAG_UNIT_COLOR_OLD,pPlayer->self->ViewFlag(__FLAG_UNIT_COLOR    ));

            strMsgTmp.Format(_STR(15211, pPlayer->self->GetLang()));
            pPlayer->self->SendSystemMessage(strMsgTmp.GetBuffer(0));
            pPlayer->self->SetPseudoName(pPlayer->self->GetTrueName());
            pPlayer->self->Teleport(pPlayer->self->GetWL(),0);
            return true;
         }
      }
      else if (_strnicmp(Message.c_str(), ".SPOOF LAST", 11) == 0)
      {
         if(pPlayer->IsGod())
         {
            if(pPlayer->m_dwLastShowID != 0)
            {
               Unit *lpUnit = Unit::GetByID(pPlayer->m_dwLastShowID);
               if(lpUnit)
               {
                  int dwNewColor;
                  if(  lpUnit->GetType() == U_OBJECT ) 
                     dwNewColor = U_OBJECT_COLOR
                  else if ( lpUnit->GetType() == U_NPC ) 
                     dwNewColor = U_NPC_COLOR
                  else if ( lpUnit->GetType() == U_PC ) 
                  {
                     dwNewColor = lpUnit->ViewFlag(__FLAG_UNIT_COLOR);
                     if(!dwNewColor)
                     {
                        Players *lpPlayerTmp = static_cast< Character * >( lpUnit )->GetPlayer();

                        if(theApp.m_dwPVPSyetem2Actif == 1) //PVP SYSTEM
                        {
                           if ( lpPlayerTmp->IsGod() ) 
                           {
                              dwNewColor = U_GOD_COLOR
                           }
                           else
                           {
                              if(lpPlayerTmp->self->GetCrime() == 0 && lpPlayerTmp->self->GetHonor() == 0)
                                 dwNewColor = U_OBJECT_COLOR
                              else if(lpPlayerTmp->self->GetCrime() >= lpPlayerTmp->self->GetHonor())
                                 dwNewColor = U_PC_COLOR
                              else
                                 dwNewColor = U_PCRP_COLOR
                           }
                        }
                        else
                        {
                           if ( lpPlayerTmp->IsGod() ) 
                              dwNewColor = U_GOD_COLOR
                           else if(lpUnit->ViewFlag(__FLAG_RPHRP_STATUS) == 1)
                              dwNewColor = U_PCRP_COLOR
                           else
                              dwNewColor = U_PC_COLOR
                        }
                     }
                  }
                  pPlayer->m_dwSpoofedID = pPlayer->m_dwLastShowID;
                  strMsgTmp.Format(_STR(15212, pPlayer->self->GetLang()),lpUnit->GetName(IntlText::GetDefaultLng()),pPlayer->m_dwSpoofedID);
                  pPlayer->self->SendSystemMessage(strMsgTmp.GetBuffer(0));
                  pPlayer->self->SetPseudoName(lpUnit->GetName(IntlText::GetDefaultLng()));
                  pPlayer->self->SetFlag(__FLAG_UNIT_COLOR_OLD,pPlayer->self->ViewFlag(__FLAG_UNIT_COLOR    ));
                  pPlayer->self->SetFlag(__FLAG_UNIT_COLOR    ,dwNewColor);
                  pPlayer->self->Teleport(pPlayer->self->GetWL(),0);
               }
               else
               {
                  Players *lpPlayer = CPlayerManager::GetCharacterOldByID(pPlayer->m_dwLastShowID);
                  if(lpPlayer)
                  {
                     int dwNewColor = lpPlayer->self->ViewFlag(__FLAG_UNIT_COLOR);
                     if(!dwNewColor)
                     {
                        if(theApp.m_dwPVPSyetem2Actif == 1) //PVP SYSTEM
                        {
                           if ( lpPlayer->IsGod() ) 
                           {
                              dwNewColor = U_GOD_COLOR
                           }
                           else
                           {
                              if(lpPlayer->self->GetCrime() == 0 && lpPlayer->self->GetHonor() == 0)
                                 dwNewColor = U_OBJECT_COLOR
                              else if(lpPlayer->self->GetCrime() >= lpPlayer->self->GetHonor())
                                 dwNewColor = U_PC_COLOR
                              else
                                 dwNewColor = U_PCRP_COLOR
                           }
                        }
                        else
                        {
                           if ( lpPlayer->IsGod() ) 
                              dwNewColor = U_GOD_COLOR
                           else if(lpPlayer->self->ViewFlag(__FLAG_RPHRP_STATUS) == 1)
                              dwNewColor = U_PCRP_COLOR
                           else
                              dwNewColor = U_PC_COLOR
                        }
                     }

                     pPlayer->m_dwSpoofedID = pPlayer->m_dwLastShowID;
                     strMsgTmp.Format(_STR(15212, pPlayer->self->GetLang()),lpPlayer->self->GetName(lpPlayer->self->GetLang()),pPlayer->m_dwSpoofedID);
                     pPlayer->self->SendSystemMessage(strMsgTmp.GetBuffer(0));
                     pPlayer->self->SetPseudoName(lpPlayer->self->GetName(IntlText::GetDefaultLng()));
                     pPlayer->self->SetFlag(__FLAG_UNIT_COLOR_OLD,pPlayer->self->ViewFlag(__FLAG_UNIT_COLOR    ));
                     pPlayer->self->SetFlag(__FLAG_UNIT_COLOR    ,dwNewColor);
                     pPlayer->self->Teleport(pPlayer->self->GetWL(),0);
                  }
                  else
                  {
                     strMsgTmp.Format(_STR(15213, pPlayer->self->GetLang()),pPlayer->m_dwLastShowID);
                     pPlayer->self->SendSystemMessage(strMsgTmp.GetBuffer(0));
                  }
               }
            }
            else
            {
               strMsgTmp.Format(_STR(15214, pPlayer->self->GetLang()));
               pPlayer->self->SendSystemMessage(strMsgTmp.GetBuffer(0));
            }
            return true;
         }
      }

      else if (_strnicmp(Message.c_str(), ".SPOOF ", 7) == 0)
      {
         if(pPlayer->IsGod())
         {
            int iID =  atoi(Message.substr(7).c_str());
            if(iID)
            {
               Unit *lpUnit = Unit::GetByID(iID);
               if(lpUnit)
               {
                  int dwNewColor;
                  if(  lpUnit->GetType() == U_OBJECT ) 
                     dwNewColor = U_OBJECT_COLOR
                  else if ( lpUnit->GetType() == U_NPC ) 
                  dwNewColor = U_NPC_COLOR
                  else if ( lpUnit->GetType() == U_PC ) 
                  {
                     dwNewColor = lpUnit->ViewFlag(__FLAG_UNIT_COLOR);
                     if(!dwNewColor)
                     {
                        Players *lpPlayerTmp = static_cast< Character * >( lpUnit )->GetPlayer();
                        if(theApp.m_dwPVPSyetem2Actif == 1) //PVP SYSTEM
                        {
                           if ( lpPlayerTmp->IsGod() ) 
                           {
                              dwNewColor = U_GOD_COLOR
                           }
                           else
                           {
                              if(lpPlayerTmp->self->GetCrime() == 0 && lpPlayerTmp->self->GetHonor() == 0)
                                 dwNewColor = U_OBJECT_COLOR
                              else if(lpPlayerTmp->self->GetCrime() >= lpPlayerTmp->self->GetHonor())
                                 dwNewColor = U_PC_COLOR
                              else
                                 dwNewColor = U_PCRP_COLOR
                           }
                        }
                        else
                        {
                           if ( lpPlayerTmp->IsGod() ) 
                              dwNewColor = U_GOD_COLOR
                           else if(lpUnit->ViewFlag(__FLAG_RPHRP_STATUS) == 1)
                              dwNewColor = U_PCRP_COLOR
                           else
                              dwNewColor = U_PC_COLOR
                        }
                     }
                  }

                  

                  pPlayer->m_dwSpoofedID = iID;
                  strMsgTmp.Format(_STR(15212, pPlayer->self->GetLang()),lpUnit->GetName(IntlText::GetDefaultLng()),pPlayer->m_dwSpoofedID);
                  pPlayer->self->SendSystemMessage(strMsgTmp.GetBuffer(0));
                  pPlayer->self->SetPseudoName(lpUnit->GetName(IntlText::GetDefaultLng()));
                  pPlayer->self->SetFlag(__FLAG_UNIT_COLOR_OLD,pPlayer->self->ViewFlag(__FLAG_UNIT_COLOR    ));
                  pPlayer->self->SetFlag(__FLAG_UNIT_COLOR    ,dwNewColor);
                  pPlayer->self->Teleport(pPlayer->self->GetWL(),0);
               }
               else
               {
                  Players *lpPlayer = CPlayerManager::GetCharacterOldByID(pPlayer->m_dwLastShowID); //pas de lock ICI commande GM...
                  if(lpPlayer)
                  {
                     int dwNewColor = lpPlayer->self->ViewFlag(__FLAG_UNIT_COLOR);
                     if(!dwNewColor)
                     {
                        if(theApp.m_dwPVPSyetem2Actif == 1) //PVP SYSTEM
                        {
                           if ( lpPlayer->IsGod() ) 
                           {
                              dwNewColor = U_GOD_COLOR
                           }
                           else
                           {
                              if(lpPlayer->self->GetCrime() == 0 && lpPlayer->self->GetHonor() == 0)
                                 dwNewColor = U_OBJECT_COLOR
                              else if(lpPlayer->self->GetCrime() >= lpPlayer->self->GetHonor())
                                 dwNewColor = U_PC_COLOR
                              else
                                 dwNewColor = U_PCRP_COLOR
                           }
                        }
                        else
                        {
                           if ( lpPlayer->IsGod() ) 
                              dwNewColor = U_GOD_COLOR
                           else if(lpPlayer->self->ViewFlag(__FLAG_RPHRP_STATUS) == 1)
                              dwNewColor = U_PCRP_COLOR
                           else
                              dwNewColor = U_PC_COLOR
                        }
                     }

                     pPlayer->m_dwSpoofedID = pPlayer->m_dwLastShowID;
                     strMsgTmp.Format(_STR(15212, pPlayer->self->GetLang()),lpPlayer->self->GetName(lpPlayer->self->GetLang()),pPlayer->m_dwSpoofedID);
                     pPlayer->self->SetPseudoName(lpPlayer->self->GetName(IntlText::GetDefaultLng()));
                     pPlayer->self->SetFlag(__FLAG_UNIT_COLOR_OLD,pPlayer->self->ViewFlag(__FLAG_UNIT_COLOR    ));
                     pPlayer->self->SetFlag(__FLAG_UNIT_COLOR    ,dwNewColor);
                     pPlayer->self->Teleport(pPlayer->self->GetWL(),0);
                  }
                  else
                  {
                     strMsgTmp.Format(_STR(15213, pPlayer->self->GetLang()),pPlayer->m_dwLastShowID);
                     pPlayer->self->SendSystemMessage(strMsgTmp.GetBuffer(0));
                  }
               }
            }
            else
            {
               strMsgTmp.Format(_STR(15214, pPlayer->self->GetLang()));
               pPlayer->self->SendSystemMessage(strMsgTmp.GetBuffer(0));
            }
            return true;
         }
      }
      else if (_strnicmp(Message.c_str(), ".GETAREALINK ", 13) == 0)
      {
         if(pPlayer->IsGod())
         {
            int iWorld =  atoi(Message.substr(13).c_str());
            CAutoLock autoArealinkLock( &g_ALockAreaLink );

            int iNbrArealink = 0;

            for(int i=0;i<m_aServerArealinkList.GetSize();i++)
            {
               if(m_aServerArealinkList[i].dwSrcW == iWorld)
                  iNbrArealink++;

            }

            if(iNbrArealink > 0)
            {

               TFCPacket sending;
               sending << (RQ_SIZE)RQ_GetAllArealinkForWorld;
               sending << (long)iNbrArealink;
               sending << (long)iWorld;

               for(int i=0;i<m_aServerArealinkList.GetSize();i++)
               {
                  if(m_aServerArealinkList[i].dwSrcW == iWorld)
                  {
                     sending << (short)m_aServerArealinkList[i].dwSrcX;
                     sending << (short)m_aServerArealinkList[i].dwSrcY;
                     sending << (short)m_aServerArealinkList[i].dwSrcW;
                     sending << (short)m_aServerArealinkList[i].dwDesX;
                     sending << (short)m_aServerArealinkList[i].dwDesY;
                     sending << (short)m_aServerArealinkList[i].dwDesW;
                  }
               }
               pPlayer->self->SendPlayerMessage(sending);
               strMsgTmp.Format(_STR(15346, pPlayer->self->GetLang()));
               pPlayer->self->SendSystemMessage(strMsgTmp.GetBuffer(0));
            }
            else
            {
               strMsgTmp.Format(_STR(15347, pPlayer->self->GetLang()));
               pPlayer->self->SendSystemMessage(strMsgTmp.GetBuffer(0));
            }


            return true;
         }
      }

      

      else if (_stricmp(Message.c_str(), ".GMHIDE") == 0)
      {
         if(pPlayer->IsGod())
         {
			   std::string Msg = "giveme gameop flag god_true_invisibility";
            SysopCmd::VerifySysopCommand( pPlayer, (char*)Msg.c_str() );

            strMsgTmp.Format("teleport to %i,%i,%i", pPlayer->self->GetWL().X , pPlayer->self->GetWL().Y, pPlayer->self->GetWL().world);
            SysopCmd::VerifySysopCommand( pPlayer, strMsgTmp.GetBuffer(0) );

            return true;
         }
         return true;
      }
      else if (_stricmp(Message.c_str(), ".GMSHOW") == 0)
      {
         if(pPlayer->IsGod())
         {
            std::string Msg = "removeme gameop flag god_true_invisibility";
            SysopCmd::VerifySysopCommand( pPlayer, (char*)Msg.c_str() );

            strMsgTmp.Format("teleport to %i,%i,%i", pPlayer->self->GetWL().X , pPlayer->self->GetWL().Y, pPlayer->self->GetWL().world);
            SysopCmd::VerifySysopCommand( pPlayer, strMsgTmp.GetBuffer(0) );

            return true;
         }
         return true;
      }

      else if (_stricmp(Message.c_str(), ".CCHIDE") == 0)
      {
         if(pPlayer->IsGod())
         {
			 strMsgTmp.Format("remove %s from user listing", pPlayer->self->GetTrueName().GetBuffer(0));
            SysopCmd::VerifySysopCommand( pPlayer, strMsgTmp.GetBuffer(0) );
            return true;
         }
      }
      else if (_stricmp(Message.c_str(), ".CCSHOW") == 0)
      {
         if(pPlayer->IsGod())
         {
			 strMsgTmp.Format("restore %s to user listing", pPlayer->self->GetTrueName().GetBuffer(0));
            SysopCmd::VerifySysopCommand( pPlayer, strMsgTmp.GetBuffer(0) );
            return true;
         }
      }
      else if (_stricmp(Message.c_str(), ".MONSTER") == 0)
      {
         if(pPlayer->IsGod())
         {
   		   std::string Msg = "removeme gameop flag god_no_monsters";
            SysopCmd::VerifySysopCommand( pPlayer, (char*)Msg.c_str() );

            return true;
         }
         return true;
      }
      else if (_stricmp(Message.c_str(), ".NOMONSTER") == 0)
      {
         if(pPlayer->IsGod())
         {
			   std::string Msg = "giveme gameop flag god_no_monsters";
            SysopCmd::VerifySysopCommand( pPlayer, (char*)Msg.c_str() );

            return true;
         }
         return true;
      }

      else if (_stricmp(Message.c_str(), ".DEV") == 0)
      {
         if(pPlayer->IsGod())
         {
			   std::string Msg = "giveme gameop flag god_developper";
            SysopCmd::VerifySysopCommand( pPlayer, (char*)Msg.c_str() );
   
            return true;
         }
         return true;
      }
      else if (_stricmp(Message.c_str(), ".NODEV") == 0)
      {
         if(pPlayer->IsGod())
         {
			   std::string Msg = "removeme gameop flag god_developper";
            SysopCmd::VerifySysopCommand( pPlayer, (char*)Msg.c_str() );

            return true;
         }
         return true;
      }
      else if (_stricmp(Message.c_str(), ".NODIE") == 0)
      {
         if(pPlayer->IsGod())
         {
			   std::string Msg = "giveme gameop flag god_cannot_die";
            SysopCmd::VerifySysopCommand( pPlayer, (char*)Msg.c_str() );

            return true;
         }
         return true;
      }
      else if (_stricmp(Message.c_str(), ".DIE") == 0)
      {
         if(pPlayer->IsGod())
         {
			   std::string Msg = "removeme gameop flag god_cannot_die";
            SysopCmd::VerifySysopCommand( pPlayer, (char*)Msg.c_str() );

            return true;
         }
         return true;
      }
	   else if (_stricmp(Message.c_str(), ".NOINVINCIBLE") == 0)
      {
         if(pPlayer->IsGod())
         {
			   std::string Msg = "giveme gameop flag god_invincible";
            SysopCmd::VerifySysopCommand( pPlayer, (char*)Msg.c_str() );

            return true;
         }
         return true;
      }
      else if (_stricmp(Message.c_str(), ".INVINCIBLE") == 0)
      {
         if(pPlayer->IsGod())
         {
			   std::string Msg = "removeme gameop flag god_invincible";
            SysopCmd::VerifySysopCommand( pPlayer, (char*)Msg.c_str() );

            return true;
         }
         return true;
      }
	  
      else if (_stricmp(Message.c_str(), ".NOCLIP") == 0)
      {
         if(pPlayer->IsGod())
         {
			   std::string Msg = "giveme gameop flag god_no_clip";
            SysopCmd::VerifySysopCommand( pPlayer, (char*)Msg.c_str() );

            return true;
         }
         return true;
      }
      else if (_stricmp(Message.c_str(), ".CLIP") == 0)
      {
         if(pPlayer->IsGod())
         {
            std::string Msg = "removeme gameop flag god_no_clip";
            SysopCmd::VerifySysopCommand( pPlayer, (char*)Msg.c_str() );

            return true;
         }
         return true;
      }

      else if (_stricmp(Message.c_str(), ".TITLE") == 0)
      {
         if(pPlayer->IsGod())
         {
			   std::string Msg = "removeme gameop flag god_can_see_accounts";
            SysopCmd::VerifySysopCommand( pPlayer, (char*)Msg.c_str() );

            return true;
         }
         return true;
      }
      else if (_stricmp(Message.c_str(), ".ACCOUNT") == 0)
      {
         if(pPlayer->IsGod())
         {
			   std::string Msg = "giveme gameop flag god_can_see_accounts";
            SysopCmd::VerifySysopCommand( pPlayer, (char*)Msg.c_str() );

            return true;
         }
         return true;
      }

	  else if (_stricmp(Message.c_str(), ".GETCOMMR") == 0)
	  {
		  if(pPlayer->IsGod())
		  {
			  CString strMsg;
			  NMPacketManager *lpComm = CPacketManager::GetCommCenter();
			  strMsg.Format("Number of received packet by packetID //Only >0 displayed");
			  pPlayer->self->SendSystemMessage(strMsg.GetBuffer(0));
			  for(int i=0;i<0xFFFF;i++)
			  {
				  if(lpComm->m_pdwNbrPacketRecvbyPacket[i] >0)
				  {
					  strMsg.Format("Packet ID %05d --> %d",i,lpComm->m_pdwNbrPacketRecvbyPacket[i]);
					  pPlayer->self->SendSystemMessage(strMsg.GetBuffer(0));
				  }
			  }

			  return true;
		  }
		  return true;
	  }

	  else if (_stricmp(Message.c_str(), ".GETCOMMS") == 0)
	  {
		  if(pPlayer->IsGod())
		  {
			  CString strMsg;
			  NMPacketManager *lpComm = CPacketManager::GetCommCenter();
			  strMsg.Format("Number of sended packet by packetID //Only >0 displayed and not UDP but t4c packet");
			  pPlayer->self->SendSystemMessage(strMsg.GetBuffer(0));
			  for(int i=0;i<0xFFFF;i++)
			  {
				  if(lpComm->m_pdwNbrPacketSendbyPacket[i] >0)
				  {
					  strMsg.Format("Packet ID %05d --> %d",i,lpComm->m_pdwNbrPacketSendbyPacket[i]);
					  pPlayer->self->SendSystemMessage(strMsg.GetBuffer(0));
				  }
			  }

			  return true;
		  }
		  return true;
	  }

	  else if (_stricmp(Message.c_str(), ".GETCOMMP") == 0)
	  {
		  if(pPlayer->IsGod())
		  {
			  CString strMsg;
			  NMPacketManager *lpComm = CPacketManager::GetCommCenter();
		   	  strMsg.Format("Nbr Packet in process Queue = %d",lpComm->GetIntrQueueSize());
			  pPlayer->self->SendSystemMessage(strMsg.GetBuffer(0));

			  return true;
		  }
		  return true;
	  }

	  
      
      else if (_stricmp(Message.c_str(), ".GETCOMMSTAT") == 0)
      {
         if(pPlayer->IsGod())
         {
            CString strMsg;
            NMPacketManager *lpComm = CPacketManager::GetCommCenter();
            strMsg.Format("COMM: Nbr COMM = %d",lpComm->GetNbrConnection());
            pPlayer->self->SendSystemMessage(strMsg.GetBuffer(0));
            strMsg.Format("COMM: Nbr Send packet = %d",lpComm->m_dwNbrPacketSend);
            pPlayer->self->SendSystemMessage(strMsg.GetBuffer(0));
            strMsg.Format("COMM: Nbr Recv packet = %d",lpComm->m_dwNbrPacketRecv);
            pPlayer->self->SendSystemMessage(strMsg.GetBuffer(0));
            strMsg.Format("COMM: Nbr Retry packet = %d",lpComm->m_dwGlobalNbrRetry);
            pPlayer->self->SendSystemMessage(strMsg.GetBuffer(0));
            strMsg.Format("COMM: Nbr Lost packet = %d",lpComm->m_dwGlobalNbrLost);
            pPlayer->self->SendSystemMessage(strMsg.GetBuffer(0));
            strMsg.Format("COMM: Nbr Already registred packet = %d",lpComm->m_dwNbrPacketRecvAlreadyRegistred);
            pPlayer->self->SendSystemMessage(strMsg.GetBuffer(0));

       

            return true;
         }
         return true;
      }

      else if (_stricmp(Message.c_str(), ".GETUNITMAPSIZE") == 0)
      {
         if(pPlayer->IsGod())
         {
            CString strMsg;
            NMPacketManager *lpComm = CPacketManager::GetCommCenter();
            strMsg.Format("Unit Map Size: %d", Unit::GetNbrUnitMap());
            pPlayer->self->SendSystemMessage(strMsg.GetBuffer(0));
            return true;
         }
         return true;
      }

     

	  
      
      else if (_strnicmp(Message.c_str(), ".NAME ", 6) == 0)
      {
         if(pPlayer->IsGod())
         {
            std::string Name = Message.substr(6);

            if (Name.size() > 2)
            {
				   strMsgTmp.Format("set %s's name to %s",pPlayer->self->GetTrueName(),Name.c_str());
               SysopCmd::VerifySysopCommand( pPlayer, strMsgTmp.GetBuffer(0) );
            }
            return true;
         }
         return true;
      }

      else if (_strnicmp(Message.c_str(), ".PNAME ", 7) == 0)
      {
         if(pPlayer->IsGod())
         {
            std::string Name = Message.substr(7);

            if (Name.size() > 2)
            {
				strMsgTmp.Format("set %s's pseudoname to %s",pPlayer->self->GetTrueName(),Name.c_str());
               SysopCmd::VerifySysopCommand( pPlayer, strMsgTmp.GetBuffer(0) );
            }
            return true;
         }
         return true;
      }

      else if (_strnicmp(Message.c_str(), ".TITLE ", 7) == 0)
      {
         if(pPlayer->IsGod())
         {
            std::string Name = Message.substr(7);

            if (Name.size() > 2)
            {
				strMsgTmp.Format("set %s's title to %s",pPlayer->self->GetTrueName(),Name.c_str());
               SysopCmd::VerifySysopCommand( pPlayer, strMsgTmp.GetBuffer(0) );
            }
            return true;
         }
         return true;
      }
      else if (_stricmp(Message.c_str(), ".TAKEALL") == 0)
      {
         if(pPlayer->IsGod())
         {
            if(m_bProcessTakeALLprogress)
            {
               pPlayer->self->SendSystemMessage("TAKEALL already in progress by GM... only 1 in same time.");
               return true;
            }
            m_bProcessTakeALLprogress = true;
            m_pTakeAllPlayer = pPlayer;

            UINT threadId;
            (HANDLE)_beginthreadex( NULL, 0, ProcessAsyncTakeALL    , this, 0, &threadId ); //OK: thread qui ramasse tous les objets au sol pres du PLAYEr GM only et auto detruit
            return true;
         }
         return true;
      }
      else if (m_dwCCShortcut)
      {
         std::string::size_type pos = Message.find (" ",0);
         if (pos != std::string::npos)
         {
            vector< ChatterChannels::Channel > vChannels;
            vector< ChatterChannels::Channel > vChannelp;

            ChatterChannels &cChatter = CPlayerManager::GetChatter();
            // Get the list of private and system channels.
            cChatter.GetSystemChannelList( pPlayer, vChannels );
            cChatter.GetRegisteredChannelList( pPlayer, vChannelp );
            std::string Message2 = Message.substr(pos + 1);
            std::string Channel2 = Message.substr(1, pos - 1);

            for(int n=0;n<vChannelp.size();n++)
            {

               CString strChanF;
               CString strChanP;
               strChanF.Format("%s",vChannelp[n].GetID().c_str());
               strChanP.Format("%s",Channel2.c_str());
               strChanF.MakeLower();
               strChanP.MakeLower();

               if(strChanF.Find(strChanP.GetBuffer(0),0) == 0)
               {
                  ChatterChannels &cChatter = CPlayerManager::GetChatter();
                  cChatter.Talk( pPlayer, vChannelp[n].GetID().c_str(), Message2.c_str() );

                  return true;
               }
            }
            for(int n=0;n<vChannels.size();n++)
            {

               CString strChanF;
               CString strChanP;
               strChanF.Format("%s",vChannels[n].GetID().c_str());
               strChanP.Format("%s",Channel2.c_str());
               strChanF.MakeLower();
               strChanP.MakeLower();

               if(strChanF.Find(strChanP.GetBuffer(0),0) == 0)
               {
                  ChatterChannels &cChatter = CPlayerManager::GetChatter();
                  cChatter.Talk( pPlayer, vChannels[n].GetID().c_str(), Message2.c_str() );
                  return true;
               }
            }
         }
      }
   }
   return false;
}
 

void CTFCServerApp::ManageDuelClean()
{
   CAutoLock autoDuelLock( &g_ALockDuel );
   CAutoLock autoDuelDLock( &g_ALockDuelD );
   CString strMsgTmp;
   for(int i=m_asDuel.GetSize()-1;i>=0;i--)
   {
      //1000 == 1sec;
      if(m_asDuel[i].dwAccepTime >0)
      {
         if(GetTickCount()-m_asDuel[i].dwAccepTime > 30000) // si une demande a ete envoyer et plus de 30 secondes s'ecoule
         {
            Players *lpPlayerM = CPlayerManager::GetCharacterRessourceByID(m_asDuel[i].dwIDMaster);//PM
            if(lpPlayerM)
            {
               CString strName = "---";
               Players *lpPlayerS = CPlayerManager::GetCharacterRessourceByID(m_asDuel[i].dwIDSlave);//PM
               if(lpPlayerS)
               {
                  strName = lpPlayerS->self->GetTrueName();
                  CPlayerManager::FreePlayerResource(lpPlayerS);
               }
               

               strMsgTmp.Format(_STR(15158, lpPlayerM->self->GetLang()), strName);
               lpPlayerM->self->SendInfoMessage(strMsgTmp.GetBuffer(0),0x0570D5);

               CPlayerManager::FreePlayerResource(lpPlayerM);
            }
            m_asDuel.RemoveAt(i);
         }
      }
      else if(m_asDuel[i].dwStartTime >0  && GetTickCount()-m_asDuel[i].dwStartTime > 600000) // Duree trop longue on libere la zone...
      {
         Players *lpPlayerM = CPlayerManager::GetCharacterRessourceByID(m_asDuel[i].dwIDMaster);//PM
         if(lpPlayerM)
         {
            strMsgTmp.Format(_STR(15159, lpPlayerM->self->GetLang()));
            lpPlayerM->self->SendInfoMessage(strMsgTmp.GetBuffer(0),0x0570D5);
            CPlayerManager::FreePlayerResource(lpPlayerM);
            
         }

         Players *lpPlayerS = CPlayerManager::GetCharacterRessourceByID(m_asDuel[i].dwIDSlave);//PM
         if(lpPlayerS)
         {
            strMsgTmp.Format(_STR(15159, lpPlayerS->self->GetLang()));
            lpPlayerS->self->SendInfoMessage(strMsgTmp.GetBuffer(0),0x0570D5);
            CPlayerManager::FreePlayerResource(lpPlayerS);
         }
         m_asDuel.RemoveAt(i);
      }
      else
      {
         //duel en cours, on verifie si les 2 membre sont encore present en jeux...
         Players *lpPlayerM = CPlayerManager::GetCharacterRessourceByID(m_asDuel[i].dwIDMaster);//PM
         Players *lpPlayerS = CPlayerManager::GetCharacterRessourceByID(m_asDuel[i].dwIDSlave);//PM

         if(!lpPlayerM || !lpPlayerS)
         {
            if(lpPlayerM)
            {
               strMsgTmp.Format(_STR(15175, lpPlayerM->self->GetLang()));
               lpPlayerM->self->SendInfoMessage(strMsgTmp.GetBuffer(0),0x0570D5);
               CPlayerManager::FreePlayerResource(lpPlayerM);
            }
            if(lpPlayerS)
            {
               strMsgTmp.Format(_STR(15175, lpPlayerS->self->GetLang()));
               lpPlayerS->self->SendInfoMessage(strMsgTmp.GetBuffer(0),0x0570D5);
               CPlayerManager::FreePlayerResource(lpPlayerS);
            }
            m_asDuel.RemoveAt(i);
         }
         else
         {
            if(lpPlayerM)
               CPlayerManager::FreePlayerResource(lpPlayerM);
            if(lpPlayerS)
               CPlayerManager::FreePlayerResource(lpPlayerS);
         }
      }
   }
}



void CTFCServerApp::ManageDuelDecompte()
{
   CAutoLock autoDuelDLock( &g_ALockDuelD );
   for(int i=m_asDuelDecompte.GetSize()-1;i>=0;i--)
   {
      if(m_asDuelDecompte[i].dwCmpteur >=0)
      {
         Players *lpPlayerM = CPlayerManager::GetCharacterRessourceByID(m_asDuelDecompte[i].uiIDM ); //PM   
         Players *lpPlayerS = CPlayerManager::GetCharacterRessourceByID(m_asDuelDecompte[i].uiIDS ); //PM
         if(lpPlayerM && lpPlayerS)
         {
            char strTmp[5];
            if(m_asDuelDecompte[i].dwCmpteur >0)
               sprintf_s(strTmp,"%d",m_asDuelDecompte[i].dwCmpteur);
            else
               sprintf_s(strTmp,"GO !");

            if(lpPlayerM)
            {
               //lpPlayerM->self->SendInfoMessage(strTmp,0x0570D5);
               SendIndirectTalkMessage(lpPlayerM,0,strTmp);
               CPlayerManager::FreePlayerResource(lpPlayerM);
            }
            if(lpPlayerS)
            {
               //lpPlayerS->self->SendInfoMessage(strTmp,0x0570D5);
               SendIndirectTalkMessage(lpPlayerS,0,strTmp);
               CPlayerManager::FreePlayerResource(lpPlayerS);
            }
            if(m_asDuelDecompte[i].dwCmpteur == 0)
               m_asDuelDecompte.RemoveAt(i);
            else
               m_asDuelDecompte[i].dwCmpteur--;
         }
         else
         {
            if(lpPlayerM)
               CPlayerManager::FreePlayerResource(lpPlayerM);
            if(lpPlayerS)
               CPlayerManager::FreePlayerResource(lpPlayerS);

            m_asDuelDecompte.RemoveAt(i);
         }
      }
   }
}

UINT CTFCServerApp::ProcessAsyncTakeALL(LPVOID lpData) 
{
   CTFCServerApp *pMe = (CTFCServerApp *)lpData;


   CString strMsgTmp;
   TFCPacket sending;



   if(pMe->m_pTakeAllPlayer->in_game) // If we can process this message..
   {
      try
      {				
         WorldMap *world = TFCMAIN::GetWorld( pMe->m_pTakeAllPlayer->self->GetWL().world );
         if(world)
         {
            DWORD dwList[500];
            WorldPos wlID[500];
            DWORD dwNbItem = world->GetInViewObjectsUnits( pMe->m_pTakeAllPlayer->self->GetWL(), 30,dwList,wlID);

            if( dwNbItem == 0 )
            {  
               strMsgTmp.Format(_STR(15218, pMe->m_pTakeAllPlayer->self->GetLang()));
               pMe->m_pTakeAllPlayer->self->SendSystemMessage(strMsgTmp.GetBuffer(0));
               pMe->m_pTakeAllPlayer          = NULL;
               pMe->m_bProcessTakeALLprogress = false;
               return 0;
            }

            DWORD dwID = 0;
            WorldPos wlPos = { 0, 0, 0 };
            Unit *lpuGet;

            for(int ii=0;ii<dwNbItem;ii++)
            {
               wlPos.X  = 	wlID[ii].X;			
               wlPos.Y  = 	wlID[ii].Y;
               dwID     =  dwList[ii];

               lpuGet = world->FindNearUnit( wlPos, dwID );
               if( lpuGet && lpuGet->GetType() == U_OBJECT )
               {
                  wlPos = lpuGet->GetWL();
                  // If the user is able to hold this item.
                  if(pMe->m_pTakeAllPlayer && pMe->m_pTakeAllPlayer->in_game)
                  {
                     pMe->m_pTakeAllPlayer->self->Lock();
                     if( pMe->m_pTakeAllPlayer->self->can_get( wlPos, static_cast< Objects *>( lpuGet ) ) )
                     {

                        // Remove the object from the world.						
                        world->remove_world_unit( wlPos, dwID );

                        char lpszID[ 256 ];
                        strcpy_s( lpszID,256, "UNDEFINED ID" );
                        Unit::GetNameFromID( lpuGet->GetStaticReference(), lpszID, U_OBJECT );

                        _LOG_ITEMS
                           LOG_MISC_1,
                              "Player %s (%s) got %u item %s ID( %s ) from ( %u, %u, %u ).",
                              (LPCTSTR)pMe->m_pTakeAllPlayer->self->GetTrueName(),
                              (LPCTSTR)pMe->m_pTakeAllPlayer->GetFullAccountName(),
                              static_cast<Objects*>(lpuGet)->GetQty(),
                              (LPCTSTR)lpuGet->GetName( _DEFAULT_LNG ),
                              lpszID,
                              wlPos.X,
                              wlPos.Y,
                              pMe->m_pTakeAllPlayer->self->GetWL().world
                           LOG_


                        Broadcast::BCObjectRemoved( wlPos, _DEFAULT_RANGE_REMOVE,lpuGet->GetID()); //Takeall
                        // Give item to character.
                        if(ii == dwNbItem-1)
                           pMe->m_pTakeAllPlayer->self->GetUnit( wlPos, lpuGet ,true);
                        else
                           pMe->m_pTakeAllPlayer->self->GetUnit( wlPos, lpuGet ,false);

                     }
                     pMe->m_pTakeAllPlayer->self->Unlock();
                     Sleep(2);

                  }
                  else
                  {

                  }
               }
            }

         }
      }
      catch(TFCPacketException *e)
      {
         delete e;
      }
   }
   pMe->m_pTakeAllPlayer          = NULL;
   pMe->m_bProcessTakeALLprogress = false;

   return 0;
}


/******************************************************************************/
int CTFCServerApp::NMSGOLD_Remort(Unit *self,DWORD x,DWORD y,DWORD world,UINT uiType, bool bLevel1)
{
   if( self == NULL )
      return -1;
   if( self->GetType() != U_PC )
      return -1;

   Character *ch = static_cast< Character * >( self );

   int iNbrSeraphBefore = ch->ViewFlag( __FLAG_NUMBER_OF_REMORTS  );
   int iNbrSeraph       = ch->ViewFlag( __FLAG_NUMBER_OF_REMORTS  );
   int iCurrentAlign = ch->ViewFlag(__FLAG_USER_ALIGNMENT);
   if(iCurrentAlign == 1 && uiType == 1)
   {
      //un good essaie de passer en evil...
      return -2;
   }
   else if(iCurrentAlign == -1 && uiType == 0)
   {
      //un evil essaie de passer en good...
      return -2;
   }
   else if(iCurrentAlign != 0 && uiType == 2)
   {
      //un seraph ou humain essaie passera  dechu...
      return -2;
   }
   else if(iNbrSeraph == 6)
   {
      //un evil essaie de passer en good...
      return -3;
   }
   


      

   /////////////////////////////
   int iFlagResetChar         = ch->ViewFlag( __FLAG_RESET_BOUST_EQUIP_POS);
   int FlagBankOfWindhowl     = ch->ViewFlag( __FLAG_BANK_OF_WINDHOWL );
   int iBankIF1 = ch->ViewFlag( __FLAG_NMSBANK_TYPE_CDOMPTE         );
   int iBankIF2 = ch->ViewFlag( __FLAG_NMSBANK_OR_EN_BANK           );
   int iBankIF3 = ch->ViewFlag( __FLAG_NMSBANK_OR_MINIMUM_WEEKS     );
   int iBankIF4 = ch->ViewFlag( __FLAG_NMSBANK_MAX_OR               );
   int iBankIF5 = ch->ViewFlag( __FLAG_NMSBANK_MIN_OR               );
   int iBankIF6 = ch->ViewFlag( __FLAG_NMSBANK_MAX_EMPRUNT          );
   int iBankIF7 = ch->ViewFlag( __FLAG_NMSBANK_OR_EMPRUNT_NBR_WEEKS );
   int iBankIF8 = ch->ViewFlag( __FLAG_NMSBANK_NEXT_INTERET_TIME    );
   int UserKnowsAboutDopplganger = ch->ViewFlag( __FLAG_ADDON_USER_KNOWS_ABOUT_DOPPELGANGER );

   int iInterRP     = ch->ViewFlag( __FLAG_INTERACTION_RP    );
   int iPLNewChest  = ch->ViewFlag( __FLAG_PLAYER_USE_NEW_CHEST    );
   int iPLEventsPts = ch->ViewFlag( __FLAG_POINTS_RP_XP_EVENTS    );
   int iPLEventsPtsT= ch->ViewFlag( __FLAG_POINTS_RP_XP_EVENTS_TOTAL);

   //Backup les profession
   int iProf01 = ch->ViewFlag( __FLAG_PROF_APOTICAIRE);
   int iProf02 = ch->ViewFlag( __FLAG_PROF_BIJOUTIER );
   int iProf03 = ch->ViewFlag( __FLAG_PROF_COUTURIER );
   int iProf04 = ch->ViewFlag( __FLAG_PROF_ARMURIER  );
   int iProf05 = ch->ViewFlag( __FLAG_PROF_FORGERON  );
   int iProf06 = ch->ViewFlag( __FLAG_PROF_EBENISTE  );

   if(theApp.dwResetProfessionRemort)
   {
      iProf01 = 0;
      iProf02 = 0;
      iProf03 = 0;
      iProf04 = 0;
      iProf05 = 0;
      iProf06 = 0;
   }

   int iNMSG1 = ch->GetPlayer()->GetNMSGoldSGLv1();
   int iNMSG2 = ch->GetPlayer()->GetNMSGoldSG();
   int iNMSG3 = ch->GetPlayer()->GetNMSGoldSELv1();
   int iNMSG4 = ch->GetPlayer()->GetNMSGoldSE();
   int iNMSG5 = ch->GetPlayer()->GetNMSGoldSDLv1();
   int iNMSG6 = ch->GetPlayer()->GetNMSGoldSD();
   int iNMSG7 = ch->GetPlayer()->GetNMSGoldReroll();
   int iNMSG8 = ch->GetPlayer()->GetNMSGoldToD();
   int iNMSG9 = ch->GetPlayer()->GetNMSGoldToDLv1();

   int iNMSG1C = ch->GetPlayer()->GetNMSGoldSGLv1_C();
   int iNMSG2C = ch->GetPlayer()->GetNMSGoldSG_C();
   int iNMSG3C = ch->GetPlayer()->GetNMSGoldSELv1_C();
   int iNMSG4C = ch->GetPlayer()->GetNMSGoldSE_C();
   int iNMSG5C = ch->GetPlayer()->GetNMSGoldSDLv1_C();
   int iNMSG6C = ch->GetPlayer()->GetNMSGoldSD_C();

   int iNbrDechuBefore  = ch->ViewFlag( __FLAG_NMS_DECHU          );
   int iNbrDechu        = ch->ViewFlag( __FLAG_NMS_DECHU          );
   int FlagUserAlignment      = 0;

   int iUpgradeNbr = 0;

   if(uiType == 0) //Artherk
   {
      FlagUserAlignment = 1;
      if(bLevel1)
      {
         iNbrSeraph +=iNMSG1;
         if(iNbrSeraph > 6)
         {
            iNMSG1  = iNbrSeraph-6;
            iNMSG1C = 1;
            iNbrSeraph = 6;
            
         }
         else
            iNMSG1 = 0;
      }
      else
      {
         iNbrSeraph +=iNMSG2;
         if(iNbrSeraph > 6)
         {
            iNMSG2  = iNbrSeraph-6;
            iNMSG2C = 1;
            iNbrSeraph = 6;
         }
         else
            iNMSG2 = 0;
      }

      iUpgradeNbr = iNbrSeraph-iNbrSeraphBefore;
   }
   else if(uiType == 1) //Ogrimar
   {
      FlagUserAlignment = -1;

      if(bLevel1)
      {
         iNbrSeraph +=iNMSG3;
         if(iNbrSeraph > 6)
         {
            iNMSG3  = iNbrSeraph-6;
            iNMSG3C = 1;
            iNbrSeraph = 6;
         }
         else
            iNMSG3 = 0;
      }
      else
      {
         iNbrSeraph +=iNMSG4;   
         if(iNbrSeraph > 6)
         {
            iNMSG4  = iNbrSeraph-6;
            iNMSG4C = 1;
            iNbrSeraph = 6;
         }
         else
            iNMSG4 = 0;
      }

      iUpgradeNbr = iNbrSeraph-iNbrSeraphBefore;
   }
   else if(uiType == 2) //Dechu
   {
      FlagUserAlignment = 0;

      if(bLevel1)
      {
         iNbrDechu +=iNMSG5;
         if(iNbrDechu > 6)
         {
            iNMSG5 = iNbrDechu-6;
            iNMSG5C = 1;
            iNbrDechu = 6;
         }
         else
            iNMSG5 = 0;
      }
      else
      {
         iNbrDechu +=iNMSG6;   
         if(iNbrDechu > 6)
         {
            iNMSG6 = iNbrDechu-6;
            iNMSG6C = 1;
            iNbrDechu = 6;
         }
         else
            iNMSG6 = 0;
      }

      iUpgradeNbr = iNbrDechu-iNbrDechuBefore;
   }

 
   //Load les flag a ne pas detruiire du control panel...
   RegKeyHandler regKey;
   regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY );

   list< FlagListSaved > aflagList;
   aflagList.clear();
   TFormat format;
   int i = 1;
   DWORD flags = static_cast< DWORD >( regKey.GetProfileInt( "OracleFlag1", 0 ) );
   while( flags != 0 )
   {
      FlagListSaved flag;
      flag.flagID = flags;
      flag.flagValue = ch->ViewFlag( flags );
      aflagList.push_back( flag );
      i++;
      flags = static_cast< DWORD >( regKey.GetProfileInt( format( "OracleFlag%u", i ), 0 ) );
   }

   //Si level 1 demander on detruit la liste des flags...
   if(bLevel1)
   {
      ch->DestroyFlags();
      //reset le sanctu si ye custom...
      if ( theApp.dwCustomStartupSanctuaryOnOff) 
      { 
         WorldPos WLSanc;
         WLSanc.X	 = theApp.dwCustomStartupSanctuaryX;
         WLSanc.Y	 = theApp.dwCustomStartupSanctuaryY;
         WLSanc.world = theApp.dwCustomStartupSanctuaryW;

         // Test if its a valid position.
         WorldMap *wlWorld = TFCMAIN::GetWorld( WLSanc.world );
         // If world exist and the position is valid, use custom sanctuary position else use the defaut
         if( wlWorld != NULL && wlWorld->IsValidPosition( WLSanc ) )
         {
            DWORD dwStartSanctuary = ( ( (DWORD)( (WORD)WLSanc.X ) << 20 ) + ( (DWORD)( (WORD)WLSanc.Y ) << 8 ) + (DWORD)( (BYTE)WLSanc.world ) );
            ch->SetFlag( __FLAG_DEATH_LOCATION, dwStartSanctuary);
         }
      }
   }
   else
   {
      //remove spell Puissance humaine ancestral
      WORD wSpellID = 10840;
      TemplateList< USER_SKILL > *lpSpells = ch->GetSpells();
      lpSpells->Lock();
      lpSpells->ToHead();
      while( lpSpells->QueryNext() )
      {
         if( lpSpells->Object()->GetSkillID() == wSpellID )
            lpSpells->DeleteAbsolute();                        
      }
      lpSpells->Unlock();
     
   }

   int iNbrReborn = 0;
   //on recopie les flag garder en banque...
   //si pas level un sa fait que reecraser tampis...
   ch->SetFlag( __FLAG_BANK_OF_WINDHOWL   , FlagBankOfWindhowl + (iUpgradeNbr*200000));
   if(uiType == 0 || uiType == 1)
   {
      if(uiType == 0 )
         ch->SetKarma(((iNbrSeraph*50)+101));
      else
         ch->SetKarma(((iNbrSeraph*50)+101)*-1);

      iNbrReborn = iNbrSeraph;
      ch->SetFlag( __FLAG_NUMBER_OF_REMORTS  , iNbrSeraph );
      ch->SetFlag( __FLAG_NMS_DECHU          , 0 );
   }
   else
   {
      ch->SetKarma(0);
      iNbrReborn = iNbrDechu;
      ch->SetFlag( __FLAG_NMS_DECHU          , iNbrDechu );
      ch->SetFlag( __FLAG_NUMBER_OF_REMORTS  , 0 );
   }
   
   ch->SetFlag( __FLAG_USER_ALIGNMENT     , FlagUserAlignment );
   ch->SetFlag( __QUEST_FIXED_ALIGNMENT   , FlagUserAlignment );

   ch->SetFlag( __FLAG_NMSBANK_TYPE_CDOMPTE         ,iBankIF1);
   ch->SetFlag( __FLAG_NMSBANK_OR_EN_BANK           ,iBankIF2);
   ch->SetFlag( __FLAG_NMSBANK_OR_MINIMUM_WEEKS     ,iBankIF3);
   ch->SetFlag( __FLAG_NMSBANK_MAX_OR               ,iBankIF4);
   ch->SetFlag( __FLAG_NMSBANK_MIN_OR               ,iBankIF5);
   ch->SetFlag( __FLAG_NMSBANK_MAX_EMPRUNT          ,iBankIF6);
   ch->SetFlag( __FLAG_NMSBANK_OR_EMPRUNT_NBR_WEEKS ,iBankIF7);
   ch->SetFlag( __FLAG_NMSBANK_NEXT_INTERET_TIME    ,iBankIF8);

   ch->SetFlag( __FLAG_RESET_BOUST_EQUIP_POS,iFlagResetChar);

   ch->SetFlag( __FLAG_INTERACTION_RP      ,iInterRP);
   ch->SetFlag( __FLAG_PLAYER_USE_NEW_CHEST,iPLNewChest);
   ch->SetFlag( __FLAG_POINTS_RP_XP_EVENTS ,iPLEventsPts);
   ch->SetFlag( __FLAG_POINTS_RP_XP_EVENTS_TOTAL ,iPLEventsPtsT);


   //Recopie les profession
   ch->SetFlag( __FLAG_PROF_APOTICAIRE, iProf01); 
   ch->SetFlag( __FLAG_PROF_BIJOUTIER , iProf02); 
   ch->SetFlag( __FLAG_PROF_COUTURIER , iProf03); 
   ch->SetFlag( __FLAG_PROF_ARMURIER  , iProf04); 
   ch->SetFlag( __FLAG_PROF_FORGERON  , iProf05); 
   ch->SetFlag( __FLAG_PROF_EBENISTE  , iProf06); 

   ch->SetFlag( __FLAG_ADDON_USER_KNOWS_ABOUT_DOPPELGANGER, UserKnowsAboutDopplganger );

   ch->GetPlayer()->SetNMSGoldSGLv1  (iNMSG1);
   ch->GetPlayer()->SetNMSGoldSG     (iNMSG2);
   ch->GetPlayer()->SetNMSGoldSELv1  (iNMSG3);
   ch->GetPlayer()->SetNMSGoldSE     (iNMSG4);
   ch->GetPlayer()->SetNMSGoldSDLv1  (iNMSG5);
   ch->GetPlayer()->SetNMSGoldSD     (iNMSG6);
   ch->GetPlayer()->SetNMSGoldReroll (iNMSG7);
   ch->GetPlayer()->SetNMSGoldToD    (iNMSG8);
   ch->GetPlayer()->SetNMSGoldToDLv1 (iNMSG9);

   ch->GetPlayer()->SetNMSGoldSGLv1_C(iNMSG1C);
   ch->GetPlayer()->SetNMSGoldSG_C   (iNMSG2C);
   ch->GetPlayer()->SetNMSGoldSELv1_C(iNMSG3C);
   ch->GetPlayer()->SetNMSGoldSE_C   (iNMSG4C);
   ch->GetPlayer()->SetNMSGoldSDLv1_C(iNMSG5C);
   ch->GetPlayer()->SetNMSGoldSD_C   (iNMSG6C);

   //restaure les flag sauvegarder ceux du control panel
   list< FlagListSaved >::iterator j;
   for( j = aflagList.begin(); j != aflagList.end(); j++ )
   {
      ch->SetFlag ( (*j).flagID , (*j).flagValue );
     
   }

   //////////////////////////////
   // REMOVAL OF ALL ITEMS
   for( i = 0; i < EQUIP_POSITIONS; i++ )
   {
      ch->unequip_object( i );
   }
   Unit **equip = ch->GetEquipment();
   for( i = 0; i < EQUIP_POSITIONS; i++ )
   {
      equip[ i ] = NULL;
   }

   if(bLevel1)
   {
      TemplateList< Unit > *backpack = ch->GetBackpack();
      if( backpack != NULL )
      {
         backpack->Lock();
         backpack->ToHead();
         while( backpack->QueryNext() )
         {
            Objects *obj = static_cast< Objects * >( backpack->Object() );

            if( obj->GetStaticReference() == __OBJ_GEM_OF_DESTINY  || obj->GetStaticReference() == theApp.m_dwMinionGemID)
            {
               continue;
            }

            _item *itemStructure = NULL;

            // Get the item structure.
            obj->SendUnitMessage(MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &itemStructure );

            if( itemStructure == NULL )
               continue;

            // If the object should be junked.
            if( itemStructure->dwDropFlags & JUNK_AT_SERAPH )
            {
               // Destroy it.
               backpack->Remove();
               obj->DeleteUnit();
            }            
         }
         backpack->Unlock();
      }
   
      //donne tous les points seraph...
      ch->SetFlag( __FLAG_REMORT_POINTS      , 8 + 2 * iNbrReborn );

      //Remove AllBoust
      ch->ClearAllSkillsAndSpells();
      ch->RemoveAllBoost();

      //////////////////////////////
      // STAT RESET
      ch->SetAirResist  ( 100 );
      ch->SetWaterResist( 100 );
      ch->SetFireResist ( 100 );
      ch->SetEarthResist( 100 );
      ch->SetLightResist( 5000 );
      ch->SetDarkResist ( 100 );

      ch->SetAirPower  ( 100 );
      ch->SetWaterPower( 100 );
      ch->SetFirePower ( 100 );
      ch->SetEarthPower( 100 );
      ch->SetLightPower( 100 );
      ch->SetDarkPower ( 100 );

      ch->SetINT( 20 + 5 * iNbrReborn );
      ch->SetEND( 20 + 5 * iNbrReborn );
      ch->SetSTR( 20 + 5 * iNbrReborn );
      ch->SetAGI( 20 + 5 * iNbrReborn );
      ch->SetWIS( 20 + 5 * iNbrReborn );
      ch->SetATTACK( 15 );
      ch->SetDODGE( 15 );  

      ch->SetMaxHP( rnd.roll( dice( 2, 5 ) ) + 48 + ch->GetTrueEND() );
      ch->SetHP( ch->GetTrueMaxHP(), false );

      ch->SetMaxMana( 10 + ch->GetTrueINT() * 2 / 3 + ch->GetTrueWIS() / 3 + rnd( 0, 5 ) );
      ch->SetMana( ch->GetMaxMana() ,FALSE);    

      ch->SetLevel( 1 );
      ch->SetXP( 0 );
      ch->SetStatPoints(0);
      ch->GiveSkillPoints(0);

   }
   else
   {
      // Scroll the backpack for "quest" items
      TemplateList< Unit > *backpack = ch->GetBackpack();
      if( backpack != NULL )
      {
         backpack->Lock();
         backpack->ToHead();
         while( backpack->QueryNext() )
         {
            Objects *obj = static_cast< Objects * >( backpack->Object() );

            if( obj->GetStaticReference() == __OBJ_GEM_OF_DESTINY  || obj->GetStaticReference() == theApp.m_dwMinionGemID)
            {
               continue;
            }
            else if( obj->GetStaticReference() == 42548 || 
                     obj->GetStaticReference() == 42631 || 
                     obj->GetStaticReference() == 42630 ||
                     obj->GetStaticReference() == 44180 ||
                     obj->GetStaticReference() == 44181 ||
                     obj->GetStaticReference() == 44177 ||   
                     obj->GetStaticReference() == 42782    ) //badge du gladiateur
            {
               backpack->Remove();
               obj->DeleteUnit();
            }
         }
         backpack->Unlock();
      }


      //donne tous les points seraph...
      ch->SetFlag( __FLAG_REMORT_PROCESS,0);

	  int iOldNbrPts = ch->ViewFlag( __FLAG_REMORT_PROCESS);
      if(uiType == 0 || uiType == 1)
      {
         if(iNbrSeraphBefore == 0 )
            ch->SetFlag( __FLAG_REMORT_POINTS      , 8 + 2 * iNbrReborn );
         else
         {
            int iUpgrade = iNbrReborn - iNbrSeraphBefore;
            ch->SetFlag( __FLAG_REMORT_POINTS      ,iOldNbrPts+(2 * iUpgrade ));
         }
      }
      else
      {
         if(iNbrDechuBefore == 0 )
            ch->SetFlag( __FLAG_REMORT_POINTS      , 8 + 2 * iNbrReborn );
         else
         {
            int iUpgrade = iNbrReborn - iNbrDechuBefore;
            ch->SetFlag( __FLAG_REMORT_POINTS      ,iOldNbrPts+(2 * iUpgrade ));
         }
      }
         
      int iNbPtsTotal = 95 + (5 * ch->GetLevel()) + (25 * iNbrReborn);
      int iNbPtsActuels = ch->GetTrueINT() +
                          ch->GetTrueEND() + 
                          ch->GetTrueSTR() + 
                          ch->GetTrueAGI() + 
                          ch->GetTrueWIS() + 
                          ch->GetStatPoints();
      int iNbARepartir = iNbPtsTotal - iNbPtsActuels;

      int iPtsMin = 20  + (5* iNbrReborn);

      if(ch->GetTrueINT() < iPtsMin)
      {
         int iOffset = iPtsMin - ch->GetTrueINT();
         ch->SetINT( iPtsMin );
         iNbARepartir-= iOffset;
      }

      if(ch->GetTrueEND() < iPtsMin)
      {
         int iOffset = iPtsMin - ch->GetTrueEND();
         ch->SetEND( iPtsMin );
         iNbARepartir-= iOffset;
      }

      if(ch->GetTrueSTR() < iPtsMin)
      {
         int iOffset = iPtsMin - ch->GetTrueSTR();
         ch->SetSTR( iPtsMin );
         iNbARepartir-= iOffset;
      }

      if(ch->GetTrueAGI() < iPtsMin)
      {
         int iOffset = iPtsMin - ch->GetTrueAGI();
         ch->SetAGI( iPtsMin );
         iNbARepartir-= iOffset;
      }

      if(ch->GetTrueWIS() < iPtsMin)
      {
         int iOffset = iPtsMin - ch->GetTrueWIS();
         ch->SetWIS( iPtsMin );
         iNbARepartir-= iOffset;
      }

      ch->SetStatPoints(ch->GetStatPoints()+iNbARepartir);




      int iHPFormule       = 17;
      int iHPMax = (rnd.roll( dice( 2, 5 ) ) + 90 + iPtsMin);
      int iHpOld = (rnd.roll( dice( 2, 5 ) ) + 90 + iHPFormule);

      ch->SetMaxHP( ch->GetTrueMaxHP() +(iHPMax-iHpOld));
      ch->SetHP( ch->GetTrueMaxHP(), false );

     
   }


   //////////////////////////////////
   // NEW POWERS
   WorldPos wlPos = { 0, 0, 0 };

   // Remove The dechu Aura and setaph aura...
   SpellMessageHandler::ActivateSpell( 11369, ch, NULL, NULL, wlPos );
   SpellMessageHandler::ActivateSpell( 11021, ch, NULL, NULL, wlPos );
   
   if(uiType == 0 || uiType == 1)
   {
      //cast aura Seraph
      SpellMessageHandler::ActivateSpell( 10696, ch, NULL, NULL, wlPos );
   }
   else
   {
      //cast dechu Aura...
      SpellMessageHandler::ActivateSpell( 10414, ch, NULL, NULL, wlPos ); //Remort
   }

   /////////////////////////////////////
   // TELEPORT TO REMORT AREA

   DWORD itemId;
   if( FlagUserAlignment > 0 )
   {
      if(iNbrSeraph >=6)
         itemId = 44181;
      else
         itemId = 42631;
   }
   else if( FlagUserAlignment < 0 )
   {
      if(iNbrSeraph >=6)
         itemId = 44180;
      else
         itemId = 42630;
   }
   else
   {
      itemId = 44177;
   }

   // Create seraph wings.
   Objects *obj = new Objects;
   if( obj->Create( U_OBJECT, itemId ) )
   {
      DWORD itemGlobalId = obj->GetID();

      // Add the wings to the backpack.
      ch->AddToBackpack( obj );

      // And equip them
      ch->equip_object( itemGlobalId );
   }

   WorldPos wlPosTP = { x,y,world };
   ch->Teleport( wlPosTP, 0 );
   
   TFCPacket sending;
   sending << (RQ_SIZE)RQ_Remort;
   ch->SendPlayerMessage( sending );

   return 0;
}

int CTFCServerApp::NMSGOLD_PassageDechu(Unit *self,DWORD x,DWORD y,DWORD world)
{
   if( self == NULL )
      return -1;
   if( self->GetType() != U_PC )
      return -1;

   Character *ch = static_cast< Character * >( self );

   int iNbrSeraphBefore = ch->ViewFlag( __FLAG_NUMBER_OF_REMORTS  );
   int iNbrDechu        = ch->ViewFlag( __FLAG_NMS_DECHU          );

   if(iNbrDechu >0)
      return -2; // deja dechu

   int FlagUserAlignment      = 0;

   FlagUserAlignment = 0;
   iNbrDechu = 6;

   ch->SetFlag( __FLAG_NMS_DECHU                   , iNbrDechu );
   ch->SetFlag( __FLAG_NUMBER_OF_REMORTS           , 0 );
   ch->SetFlag( __FLAG_USER_ALIGNMENT              , FlagUserAlignment );
   ch->SetFlag( __QUEST_FIXED_ALIGNMENT            , FlagUserAlignment );
   ch->GetPlayer()->SetNMSGoldToD(0);
   ch->SetFlag( __FLAG_REMORT_PROCESS,0);
   ch->SetKarma(0);

   int FlagBankOfWindhowl     = ch->ViewFlag( __FLAG_BANK_OF_WINDHOWL );
   ch->SetFlag( __FLAG_BANK_OF_WINDHOWL   , FlagBankOfWindhowl + 200000);

   
   //////////////////////////////
   // REMOVAL OF ALL ITEMS
   int i = 0;
   for( i = 0; i < EQUIP_POSITIONS; i++ )
   {
      ch->unequip_object( i );
   }
   Unit **equip = ch->GetEquipment();
   for( i = 0; i < EQUIP_POSITIONS; i++ )
   {
      equip[ i ] = NULL;
   }

   
   if(iNbrSeraphBefore == 0 )
      ch->SetFlag( __FLAG_REMORT_POINTS      , 8 + 2 * iNbrDechu );
   else
   {
      int iUpgrade = iNbrDechu - iNbrSeraphBefore;
      ch->SetFlag( __FLAG_REMORT_POINTS      ,2 * iUpgrade );
   }

   int iNbPtsTotal = 95 + (5 * ch->GetLevel()) + (25 * iNbrDechu);
   int iNbPtsActuels = ch->GetTrueINT() +
      ch->GetTrueEND() + 
      ch->GetTrueSTR() + 
      ch->GetTrueAGI() + 
      ch->GetTrueWIS() + 
      ch->GetStatPoints();
   int iNbARepartir = iNbPtsTotal - iNbPtsActuels;

   int iPtsMin = 20  + (5* iNbrDechu);

   if(ch->GetTrueINT() < iPtsMin)
   {
      int iOffset = iPtsMin - ch->GetTrueINT();
      ch->SetINT( iPtsMin );
      iNbARepartir-= iOffset;
   }

   if(ch->GetTrueEND() < iPtsMin)
   {
      int iOffset = iPtsMin - ch->GetTrueEND();
      ch->SetEND( iPtsMin );
      iNbARepartir-= iOffset;
   }

   if(ch->GetTrueSTR() < iPtsMin)
   {
      int iOffset = iPtsMin - ch->GetTrueSTR();
      ch->SetSTR( iPtsMin );
      iNbARepartir-= iOffset;
   }

   if(ch->GetTrueAGI() < iPtsMin)
   {
      int iOffset = iPtsMin - ch->GetTrueAGI();
      ch->SetAGI( iPtsMin );
      iNbARepartir-= iOffset;
   }

   if(ch->GetTrueWIS() < iPtsMin)
   {
      int iOffset = iPtsMin - ch->GetTrueWIS();
      ch->SetWIS( iPtsMin );
      iNbARepartir-= iOffset;
   }

   ch->SetStatPoints(ch->GetStatPoints()+iNbARepartir);

   int iHPFormule = 17;
   int iHPMax = (rnd.roll( dice( 2, 5 ) ) + 90 + iPtsMin);
   int iHpOld = (rnd.roll( dice( 2, 5 ) ) + 90 + iHPFormule);


   ch->SetMaxHP( ch->GetTrueMaxHP() +(iHPMax-iHpOld));
   ch->SetHP( ch->GetTrueMaxHP(), false );


   //////////////////////////////////
   // NEW POWERS
   WorldPos wlPos = { 0, 0, 0 };

   // Remove The Seraph aura...
   SpellMessageHandler::ActivateSpell( 11021, ch, NULL, NULL, wlPos );
   //vu que le flag seraph ets a 0 l'aura partira...
   //cast dechu aura
   SpellMessageHandler::ActivateSpell( 10414, ch, NULL, NULL, wlPos ); //passageDechu

   /////////////////////////////////////
   // TELEPORT TO REMORT AREA

   DWORD itemId = 44177;

   // Create seraph wings.
   Objects *obj = new Objects;
   if( obj->Create( U_OBJECT, itemId ) )
   {
      DWORD itemGlobalId = obj->GetID();

      // Add the wings to the backpack.
      ch->AddToBackpack( obj );

      // And equip them
      ch->equip_object( itemGlobalId );
   }

   //delete old wings...
   TemplateList< Unit > *backpack = ch->GetBackpack();
   if( backpack != NULL )
   {
      backpack->Lock();
      backpack->ToHead();
      while( backpack->QueryNext() )
      {
         Objects *obj = static_cast< Objects * >( backpack->Object() );

            itemId = 44181;
            itemId = 42631;
            itemId = 44180;
            itemId = 42630;

         if( obj->GetStaticReference() == 44181 || 
             obj->GetStaticReference() == 42631 || 
             obj->GetStaticReference() == 44180 ||
             obj->GetStaticReference() == 42630     ) 
         {
            backpack->Remove();
            obj->DeleteUnit();
         }
      }
      backpack->Unlock();
   }


   WorldPos wlPosTP = { x,y,world };
   ch->Teleport( wlPosTP, 0 );

   TFCPacket sending;
   sending << (RQ_SIZE)RQ_Remort;
   ch->SendPlayerMessage( sending );

   return 0;
}

int CTFCServerApp::NMSGOLD_Reroll(Unit *self,DWORD x,DWORD y,DWORD world)
{
   if( self == NULL )
      return -1;
   if( self->GetType() != U_PC )
      return -1;

   Character *ch = static_cast< Character * >( self );

   int iNbrReroll             = ch->GetPlayer()->GetNMSGoldReroll();
   int iNbrRemortS            = ch->ViewFlag( __FLAG_NUMBER_OF_REMORTS );
   int iNbrRemortD            = ch->ViewFlag( __FLAG_NMS_DECHU );
   int FlagUserAlignment      = ch->ViewFlag( __QUEST_FIXED_ALIGNMENT );

   int iNbrRemort = iNbrRemortS+iNbrRemortD; //on peu pas etre dechu + seraph amoins etre anims...


   if(iNbrReroll <1000)
   {
      iNbrReroll--;
      if(iNbrReroll <0)
         iNbrReroll = 0;
   }

   // Set Flag to good value....
   if(iNbrRemort >0)
      ch->SetFlag( __FLAG_REMORT_POINTS, 8 + 2 * iNbrRemort );
   else
      ch->SetFlag( __FLAG_REMORT_POINTS,0 ); //humain...


   ch->SetFlag( __FLAG_REMORT_PROCESS,0);
   ch->SetFlag( __FLAG_REROLL_BLOCK_LEVELUP, 1);

   ch->GetPlayer()->SetNMSGoldReroll(iNbrReroll);


   //////////////////////////////
   // REMOVAL OF ALL ITEMS
   for(int i = 0; i < EQUIP_POSITIONS; i++ )
   {
      ch->unequip_object( i );
   }
   Unit **equip = ch->GetEquipment();
   for(int  i = 0; i < EQUIP_POSITIONS; i++ )
   {
      equip[ i ] = NULL;
   }

   // Scroll the backpack for "quest" items
   TemplateList< Unit > *backpack = ch->GetBackpack();
   if( backpack != NULL )
   {
      backpack->Lock();
      backpack->ToHead();
      while( backpack->QueryNext() )
      {
         Objects *obj = static_cast< Objects * >( backpack->Object() );

         if( obj->GetStaticReference() == __OBJ_GEM_OF_DESTINY  || obj->GetStaticReference() == theApp.m_dwMinionGemID)
         {
            continue;
         }
         else if( obj->GetStaticReference() == 42548 || 
                  obj->GetStaticReference() == 42631 || 
                  obj->GetStaticReference() == 42630 ||
                  obj->GetStaticReference() == 44180 ||
                  obj->GetStaticReference() == 44181 ||
                  obj->GetStaticReference() == 44177 ||   
                  obj->GetStaticReference() == 42782    ) //badge du gladiateur
         {
            backpack->Remove();
            obj->DeleteUnit();
         }
      }
      backpack->Unlock();
   }

   //Remove AllBoust
   ch->ClearAllSkillsAndSpells();
   ch->RemoveAllBoost();

   //////////////////////////////
   // STAT RESET
   ch->SetAirResist  ( 100 );
   ch->SetWaterResist( 100 );
   ch->SetFireResist ( 100 );
   ch->SetEarthResist( 100 );
   ch->SetLightResist( 5000 );
   ch->SetDarkResist ( 100 );

   ch->SetAirPower  ( 100 );
   ch->SetWaterPower( 100 );
   ch->SetFirePower ( 100 );
   ch->SetEarthPower( 100 );
   ch->SetLightPower( 100 );
   ch->SetDarkPower ( 100 );

   if(iNbrRemort >0)
   {
      ch->SetINT( 20 + 5 * iNbrRemort );
      ch->SetEND( 20 + 5 * iNbrRemort );
      ch->SetSTR( 20 + 5 * iNbrRemort );
      ch->SetAGI( 20 + 5 * iNbrRemort );
      ch->SetWIS( 20 + 5 * iNbrRemort );
   }
   else
   {
      ch->SetINT( 16);
      ch->SetEND( 16);
      ch->SetSTR( 16);
      ch->SetAGI( 16);
      ch->SetWIS( 16); 
   }

   ch->SetATTACK( 15 ); 
   ch->SetDODGE( 15 );    

   ch->SetMaxHP( rnd.roll( dice( 2, 5 ) ) + 48 + ch->GetTrueEND() );
   ch->SetHP( ch->GetTrueMaxHP(), false );

   ch->SetMaxMana( 10 + ch->GetTrueINT() * 2 / 3 + ch->GetTrueWIS() / 3 + rnd( 0, 5 ) );
   ch->SetMana( ch->GetMaxMana() ,FALSE);    

   ch->SetLevel( 1 );

   if(iNbrRemort == 0)
      ch->SetStatPoints(ch->GetStatPoints()+5);

   //////////////////////////////////
   // NEW POWERS
   WorldPos wlPos = { 0, 0, 0 };
   // Cast the permanent remort spell.

   if(iNbrRemortD >0)
   {
      SpellMessageHandler::ActivateSpell( 10414, ch, NULL, NULL, wlPos );//Reroll

      DWORD itemId = 44177;

      // Create seraph wings.
      Objects *obj = new Objects;
      if( obj->Create( U_OBJECT, itemId ) )
      {
         DWORD itemGlobalId = obj->GetID();

         // Add the wings to the backpack.
         ch->AddToBackpack( obj );

         // And equip them
         ch->equip_object( itemGlobalId );
      }

      WorldPos wlPosTP = { x,y,world };
      ch->Teleport( wlPosTP, 0 );

      TFCPacket sending;
      sending << (RQ_SIZE)RQ_Remort;
      ch->SendPlayerMessage( sending );
   }
   else if(iNbrRemortS >0)
   {
      SpellMessageHandler::ActivateSpell( 10696, ch, NULL, NULL, wlPos );

      /////////////////////////////////////
      // TELEPORT TO REMORT AREA

      DWORD itemId;

      if( FlagUserAlignment >= 0 )
      {
         if(iNbrRemortS >=6)
            itemId = 44181;
         else
            itemId = 42631;
      }
      else
      {
         if(iNbrRemortS >=6)
            itemId = 44180;
         else
            itemId = 42630;
      }


      // Create seraph wings.
      Objects *obj = new Objects;
      if( obj->Create( U_OBJECT, itemId ) )
      {
         DWORD itemGlobalId = obj->GetID();

         // Add the wings to the backpack.
         ch->AddToBackpack( obj );

         // And equip them
         ch->equip_object( itemGlobalId );
      }

      WorldPos wlPosTP = { x,y,world };
      ch->Teleport( wlPosTP, 0 );

      TFCPacket sending;
      sending << (RQ_SIZE)RQ_Remort;
      ch->SendPlayerMessage( sending );
   }
   else
   {
      WorldPos wlPos;
      if(theApp.dwCustomStartupSanctuaryOnOff)
      {
         wlPos.X	 = theApp.dwCustomStartupSanctuaryX;
         wlPos.Y	 = theApp.dwCustomStartupSanctuaryY;
         wlPos.world = theApp.dwCustomStartupSanctuaryW;
      }
      else
      {
         wlPos.X     = 2947;
         wlPos.Y     = 1043;
         wlPos.world = 0;
      }

      ch->Teleport( wlPos, 0 );
   }

   return 0;
}


/******************************************************************************/
// The one and only CTFCServerApp object
CTFCServerApp theApp;

/******************************************************************************/
// The cleanup function called at server exit.
void exitfunc( void )
/******************************************************************************/
{
#ifdef WIN32
	SYSTEMTIME sysTime; 
	GetLocalTime(&sysTime);

	_EXITLOG "\r\n+++[EXIT] %u/%u/%u %02u:%02u:%02u +++",
		sysTime.wMonth,
		sysTime.wDay,
        sysTime.wYear, 
        sysTime.wHour, 
        sysTime.wMinute,
        sysTime.wSecond
    EXITLOG_

	// Cleanly shutdown the Winsock2 library.
	WSACleanup();
#endif
}
/******************************************************************************/
// This short helping function simply displays how to invoke the program.
void display_usage(void)
/******************************************************************************/
{
	MessageBox(NULL, 
		_T("You must start T4C as a service or specifically add\r\n"
		"the -m command parameter if you wish to quickly test\r\n"
		"the server's settings in a standard application window."), _T("Error"),MB_OK);
	exit(INVALID_PROGRAM_ARGUMENT);
}
/*******************************************************************************
	The POSIX signals handler. It catches the ABORT and TERM signals.
	@param s The signal that were caught. 
	@remark It must remain lightweight: no IO, no system-calls.
*/
void sigfunc(int s)
/******************************************************************************/
{
	_exit(SERVER_RECEIVED_SIGNAL);
}
/******************************************************************************/
// The server main entry-point.
int main(int argc,char **argv)
/******************************************************************************/
{
#ifdef _WIN32
   if (g_SingleInstanceObj.IsAnotherInstanceRunning())
      return ANOTHER_SERVER_ALREADY_RUNNING;
#else
   theApp.csT4CKEY = "Software\\Vircom\\The 4th Coming Server\\";
   RegKeyHandler::EnsureIniLoaded();
#endif

#ifdef _WIN32
   //************** validate SVR ID *********************
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
   //************** validate SVR ID *********************
   char strAppname[128];
   if(iSvrID == 0)
   {
      theApp.csT4CKEY.Format("Software\\Vircom\\The 4th Coming Server\\");
      sprintf_s(strAppname,128,"T4C Server");
   }
   else
   {
      theApp.csT4CKEY.Format("Software\\Vircom%d\\The 4th Coming Server\\",iSvrID+1);
      sprintf_s(strAppname,128,"T4C Server%d",iSvrID+1);
   }

	// Define CrashRpt configuration parameters
	CR_INSTALL_INFO info;  
	memset(&info, 0, sizeof(CR_INSTALL_INFO));  
	info.cb = sizeof(CR_INSTALL_INFO);    
   info.pszAppName = strAppname;
	info.pszAppVersion = "";//Version::sBuildStamp().c_str();
	info.pszUrl = "";//"http://carlos.priodev.net/tcc/86159abd-ab2b-4055-957c-9481d6337391/crpt.php";
	//info.pfnCrashCallback = CrashCallback;   
	info.uPriorities[CR_HTTP] = -1;  // First try send report over HTTP 
	info.uPriorities[CR_SMTP] = -1;  // Don't try to send report over SMTP  
	info.uPriorities[CR_SMAPI]= -1; // Don't try to send report over Simple MAPI    
	// Install all available exception handlers, use HTTP binary transfer encoding (recommended).
	info.dwFlags |= CR_INST_ALL_EXCEPTION_HANDLERS;
	info.dwFlags |= CR_INST_NO_GUI;

	info.dwFlags |= CR_INST_DONT_SEND_REPORT; //We're currently just saving the reports locally
	info.pszErrorReportSaveDir = "CrashReports\\";
	info.uMiniDumpType = MiniDumpWithFullMemory; //This is a big dump 200-300MB, we can return to smaller dumps once we get more stable

	// Install exception handlers
	CrAutoInstallHelper cr_install_helper(&info);
	if(cr_install_helper.m_nInstallStatus!=0)  
	{    
		// Something goes wrong. Get error message.
		char szErrorMsg[512];    
		szErrorMsg[0]=0;    
		crGetLastErrorMsg(szErrorMsg, 512);    
		printf("CrashRpt failure: %s\n", szErrorMsg);    
		return 255;
	} 

#endif /* _WIN32 CrashRpt */




	bool startAsService = false;

	// Setup the signal handler(s).
	signal( SIGABRT, sigfunc );

#if defined(WIN32) && !defined(_DEBUG)
	// In release mode, NEVER let the default exception handler display any information..!
	SetErrorMode( 
		SEM_FAILCRITICALERRORS |
		SEM_NOGPFAULTERRORBOX  |
		SEM_NOOPENFILEERRORBOX
	);
#endif    

#ifdef WIN32
	// Check wether another instance of the server is running.
	if( boServerRunning )
	{
		return ANOTHER_SERVER_ALREADY_RUNNING;
	}
	boServerRunning = TRUE;
#endif

	theApp.InService = FALSE;
    startAsService   = true;
	// Check the program parameters.
	if (argc == 2) 
   {
		if(0 == _stricmp(argv[1],"-InstallService"))
		{
			theApp.InService = TRUE;
         startAsService   = false;
		} 
      else if(0 == _stricmp(argv[1],"-m"))
      {
         startAsService = false;
         theApp.InService = FALSE;
      } 
   }

#ifdef WIN32
	// Setup the Winsock2 library.
    WSADATA wsaData;
    if( WSAStartup( MAKEWORD( 2, 0 ), &wsaData ) != 0 )
	{
        MessageBox( NULL, "T4C Server requires a WinSock 2.0 protocol to be installed.", "Error", MB_OK );
        return WINSOCK2_NOT_FOUND;
    }
#endif

	// Install the cleanup function.
	atexit( exitfunc );

	try
	{        

#ifdef __ENABLE_LOG
		if(__LOG > 0) __LOG("Entering InitInstance");
#endif

		// Getting the machine name.
		theApp.csMachineName = System::GetMachineName().c_str();
		
      
      int 		loop  = 0;
      TCHAR 		param = 0;
      TCHAR		path[_MAX_PATH * 2];	
      char        stop_char = ' ';

#ifdef WIN32
		// Removing an unused key for the registry ... why setting it ?
        {
            RegKeyHandler regKey;
            regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY);
            regKey.DeleteValue( "SHUTDOWN" );
            regKey.Close();            
        }
#endif
		
		loop = GetModuleFileName( GetModuleHandle( NULL ), path, _MAX_PATH * 2 );		
		// Find next backslash
		do
		{
			loop--;
		} while( path[ loop ] != '\\' && path[ loop ] != '/' && loop >= 0 );
		// End string after backslash.
		path[ loop + 1 ] = 0;

#ifndef _WIN32
		RegKeyHandler::EnsureIniLoaded();
#endif
		// Load the registry
		RegKeyHandler regKey;
		// Paths.
		if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+PATHS_KEY ) )
		{
			theApp.sPaths.csLogPath =		regKey.GetProfileString( "LOG_PATH", path );
			theApp.sPaths.csBinaryPath =	regKey.GetProfileString( "BINARY_PATH", path );

         AddTrailingBackslash( theApp.sPaths.csLogPath );
         AddTrailingBackslash( theApp.sPaths.csBinaryPath );

         // Make certain that the log path exists.
         CreateDirectory( theApp.sPaths.csLogPath, NULL );

         regKey.Close();
		}
		else
		{
			theApp.sPaths.csLogPath = path;
			theApp.sPaths.csBinaryPath = path;

			AddTrailingBackslash( theApp.sPaths.csLogPath );
			AddTrailingBackslash( theApp.sPaths.csBinaryPath );
		}
        ServerPath = theApp.sPaths.csBinaryPath;

		/***********************************************************************************/
		//Add  Current day on the log path to save all klog in the start day path
     
		SYSTEMTIME sysTime; 
		GetLocalTime(&sysTime);
		CString csTimeStamp;
		csTimeStamp.Format("%04d_%02d_%02d__%02dh%02d_%02d\\",
							sysTime.wYear, 
							sysTime.wMonth,
							sysTime.wDay,
							sysTime.wHour, 
							sysTime.wMinute,
							sysTime.wSecond
							);
		theApp.sPaths.csLogPath +=csTimeStamp;
		CreateDirectory(theApp.sPaths.csLogPath,NULL);
      
		/***********************************************************************************/
        {
         RegKeyHandler regKey;
         
   		if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY ) )
			{
            theApp.dwEnableWebServer = regKey.GetProfileInt( "WEB_SERVER_ENABLE", 0 );
            theApp.dwWebServerPort   = regKey.GetProfileInt( "WEB_SERVER_PORT"  , 11677 );

            theApp.sMaxCharactersPerAccount = static_cast< DWORD >( regKey.GetProfileInt( "MAX_CHARACTERS_PER_ACCOUNT", 3 ) );
            if(theApp.sMaxCharactersPerAccount >9)
               theApp.sMaxCharactersPerAccount = 9;
            theApp.dwDebugSkillParryDisabled = regKey.GetProfileInt( "DEBUG_SKILLPARRY_DISABLED", 0 );
            theApp.dwLogXPGains = regKey.GetProfileInt( "ENABLE_XP_GAIN_LOGGING", 0 );
            theApp.dwHideUncoverEffectDisabled = regKey.GetProfileInt( "DisableHideUncoverEffect", 0 );
            theApp.dwRobWhileWalkingEnabled = regKey.GetProfileInt( "EnableRobWhileWalking", 0 );
            theApp.dwRobWhileBeingAttackedEnabled = regKey.GetProfileInt( "EnableRobWhileBeingAttacked", 0 );
            theApp.dwDisableItemInfo = regKey.GetProfileInt( "DisableItemInfo", 0 );
            theApp.dwUseGMAuth = regKey.GetProfileInt( "UseGMAuth", 0 );
            theApp.dwUseNMSDeathSystem      = regKey.GetProfileInt( "USE_NMSDeathSystem", 1);

            theApp.sColosseum = static_cast< DWORD >( regKey.GetProfileInt( "ACK_COLOSSEUM", 0 ) );
            theApp.sDoppelganger = static_cast< DWORD >( regKey.GetProfileInt( "ACK_DOPPELGANGER", 0 ) );
            theApp.sMaxRemorts = static_cast< DWORD >( regKey.GetProfileInt( "ACK_MAXREMORTS", 3 ) );


            theApp.dwAHSystemEnable  = static_cast< DWORD >( regKey.GetProfileInt( "AHSystem", 1 ) );	
            theApp.dwAHSystemMaxSold = static_cast< DWORD >( regKey.GetProfileInt( "AHMaxSoldItem", 10 ) );	
            theApp.dwGuildSystemEnable = static_cast< DWORD >( regKey.GetProfileInt( "GuildSystem", 1 ) );			
            theApp.dwReloadEnable = static_cast< DWORD >( regKey.GetProfileInt( "ReloadSystem", 1 ) );	
            theApp.dwNMSGOLDEnable = static_cast< DWORD >( regKey.GetProfileInt( "NMSGOLDSystem", 1 ) );	
            theApp.dwSendConnectEquipEnable = static_cast< DWORD >( regKey.GetProfileInt( "SendConnectEquipEnable", 1 ) );	
            theApp.dwNPCOnPopupEnable = static_cast< DWORD >( regKey.GetProfileInt( "NPCOnPopupEnable", 1 ) );	
            theApp.dwChestListEnable = static_cast< DWORD >( regKey.GetProfileInt( "ChestListeEnable", 1 ) );	

            theApp.dwEventsXPTradeItemID = static_cast< DWORD >( regKey.GetProfileInt( "EVENTS_XPTRADE_ITEM_ID", 0 ) );	

            theApp.dwShareXPDropEnable     = static_cast< DWORD >( regKey.GetProfileInt( "ShareXPDropEnable", 0 ) );	
            theApp.dwTimedUnitEnable       = static_cast< DWORD >( regKey.GetProfileInt( "TimedUnitEnable", 0 ) );	
            theApp.dwTimedUnitLockExpire   = static_cast< DWORD >( regKey.GetProfileInt( "TimedUnitLockExpire", 0 ) );	
            theApp.dwTimedUnitExpire       = static_cast< DWORD >( regKey.GetProfileInt( "TimedUnitExpire", 0 ) );	

            theApp.dwTimedUnitLockExpire = 60000*3 ; //5  min
            theApp.dwTimedUnitExpire     = 60000*10; //10 min

            #ifndef BUILD_NMS_CUSTOM_NPC
               theApp.dwShareXPDropEnable = 0;	
               theApp.dwTimedUnitEnable   = 0;	
               theApp.dwTimedUnitLockExpire = 0;
               theApp.dwTimedUnitExpire     = 0;
            #endif
            

            theApp.dwEquilibrageNewCourbeXPEnable     = static_cast< DWORD >( regKey.GetProfileInt( "EquilibrageNewCourbeXPEnable", 0 ) );	
            theApp.dwEquilibrageSkillNewFormulaEnable = static_cast< DWORD >( regKey.GetProfileInt( "EquilibrageOldSkillNewFormulaEnable", 0 ) );	
            theApp.dwEquilibrageNewSkillEnable        = static_cast< DWORD >( regKey.GetProfileInt( "EquilibrageNewSkillEnable", 0 ) );	
            

            theApp.dwUDPFilterEnable = static_cast< DWORD >( regKey.GetProfileInt( "UDPFilterSystem", 1 ) );			
            theApp.dwUDPLogAnalyseEnable = static_cast< DWORD >( regKey.GetProfileInt( "UDPLogAnalyseSystem", 0 ) );			
            theApp.dwProfessionSystemEnable = static_cast< DWORD >( regKey.GetProfileInt( "Profession", 1 ) );	
            theApp.dwSendDamageHealingSystem = static_cast< DWORD >( regKey.GetProfileInt( "DamageHealingSystem", 1 ) );
            theApp.dwManageBankInteret = static_cast< DWORD >( regKey.GetProfileInt( "ManageBankInteret", 1 ) );
            theApp.dwManageScrollXP = static_cast< DWORD >( regKey.GetProfileInt( "ManageScrollXP", 1 ) );
            theApp.dwAntiplugSystem = static_cast< DWORD >( regKey.GetProfileInt( "AntiplugSystem", 1 ) );
            theApp.dwAntiplugSystemSec = static_cast< DWORD >( regKey.GetProfileInt( "AntiplugSystemSec", 10 ) );

            
            theApp.m_dwRPSystem             = static_cast< DWORD >( regKey.GetProfileInt( "RPSystem", 1 ) );
            theApp.m_dwGMMsgSystem          = static_cast< DWORD >( regKey.GetProfileInt( "GMMsgSystem", 1 ) );
            theApp.m_dwArenaSystem1[0]      = static_cast< DWORD >( regKey.GetProfileInt( "Arena1System1", 0 ) );
            theApp.m_dwArenaSystem1[1]      = static_cast< DWORD >( regKey.GetProfileInt( "Arena2System1", 0 ) );
            theApp.m_dwArenaSystem2[0]      = static_cast< DWORD >( regKey.GetProfileInt( "Arena1System2", 0 ) );
            theApp.m_dwArenaSystem2[1]      = static_cast< DWORD >( regKey.GetProfileInt( "Arena2System2", 0 ) );
            theApp.m_dwNMS5YearItemnPod     = static_cast< DWORD >( regKey.GetProfileInt( "NMS5YearItemnPod", 0 ) );
            
            
            theApp.m_dwModeRPorHRP          = static_cast< DWORD >( regKey.GetProfileInt( "ModeRPorHRP", 1 ) );
            theApp.m_dwXPstat               = static_cast< DWORD >( regKey.GetProfileInt( "XPstat", 1 ) );
            theApp.m_dwDUELSyetemActif      = static_cast< DWORD >( regKey.GetProfileInt( "DUELSyetemActif", 1 ) );
            theApp.m_dwCCShortcut           = static_cast< DWORD >( regKey.GetProfileInt( "CCShortcut", 1 ) );
            theApp.m_dwPseudoname           = static_cast< DWORD >( regKey.GetProfileInt( "Pseudoname", 1 ) );
            theApp.m_dwPVPSyetem2Actif      = static_cast< DWORD >( regKey.GetProfileInt( "PVPSyetemActif", 1 ) );
            theApp.m_dwManagePrisonExit     = static_cast< DWORD >( regKey.GetProfileInt( "ManagePrisonExit", 1 ) );
            theApp.m_dwFriendlyBlockPJAttack= static_cast< DWORD >( regKey.GetProfileInt( "FriendlyBlockPJAttack", 0 ) );

            theApp.dwEnableCOMMMegaPack     = static_cast< DWORD >( regKey.GetProfileInt( "EnableCOMMMegaPack", 1 ) );
            theApp.dwEnableCOMMCompression  = static_cast< DWORD >( regKey.GetProfileInt( "EnableCOMMCompression", 1 ) );

            theApp.m_dwFloorDamageSpellID   = static_cast< DWORD >( regKey.GetProfileInt( "FloorType11_DamageSpellID", 0 ) );
            theApp.m_dwLocalTalkRange       = static_cast< DWORD >( regKey.GetProfileInt( "LocalTalkrange", 50 ) );
            theApp.m_dwMinionGemID          = static_cast< DWORD >( regKey.GetProfileInt( "MinionGemID", 0 ) );
            theApp.m_dwVaporizeSpellID      = static_cast< DWORD >( regKey.GetProfileInt( "VaporizeSpellID", 10210 ) );
            theApp.m_dwResetBoustEquipPos    = static_cast< DWORD >( regKey.GetProfileInt( "ResetBoustEquipPos", 0 ) );
            theApp.m_dwResetBoustEquipPosOld = static_cast< DWORD >( regKey.GetProfileInt( "OldResetBoustEquipPos", 0 ) );
            theApp.m_dwResetBoustEquipPosByGM= static_cast< DWORD >( regKey.GetProfileInt( "GMResetBoustEquipPos", 0 ) );
            if(theApp.m_dwResetBoustEquipPosByGM >0)
            {
               theApp.m_dwResetBoustEquipPosByGM = 0;
               theApp.m_dwResetBoustEquipPos++;
               theApp.m_dwResetBoustEquipPosOld  = 0;
               regKey.WriteProfileInt( "GMResetBoustEquipPos" , theApp.m_dwResetBoustEquipPosByGM );
               regKey.WriteProfileInt( "ResetBoustEquipPos"   , theApp.m_dwResetBoustEquipPos     );			
               regKey.WriteProfileInt( "OldResetBoustEquipPos", theApp.m_dwResetBoustEquipPosOld  );	
            }
            


            theApp.dwGFEnableCoco2   = static_cast< DWORD >( regKey.GetProfileInt( "NMSColiseum2", 0 ) );
            theApp.dwGFEnableCoco3   = static_cast< DWORD >( regKey.GetProfileInt( "NMSColiseum3", 0 ) );
            theApp.dwGFEnableCoco4   = static_cast< DWORD >( regKey.GetProfileInt( "NMSColiseum4", 0 ) );
            theApp.dwForceDethRecall = static_cast< DWORD >( regKey.GetProfileInt( "ForceDethRecall", 0 ) );

            if ( theApp.dwCustomStartupPositionOnOff = regKey.GetProfileInt( "StartupPosOnOff", FALSE ) ) 
            {
               theApp.dwCustomStartupPositionX = regKey.GetProfileInt( "StartupPosX", NULL );
               theApp.dwCustomStartupPositionY = regKey.GetProfileInt( "StartupPosY", NULL );
               theApp.dwCustomStartupPositionW = regKey.GetProfileInt( "StartupPosW", NULL );
            } 
            if ( theApp.dwCustomStartupSanctuaryOnOff = regKey.GetProfileInt( "StartupSanctuaryOnOff", FALSE ) ) 
            {
               theApp.dwCustomStartupSanctuaryX = regKey.GetProfileInt( "StartupSanctuaryX", NULL );
               theApp.dwCustomStartupSanctuaryY = regKey.GetProfileInt( "StartupSanctuaryY", NULL );
               theApp.dwCustomStartupSanctuaryW = regKey.GetProfileInt( "StartupSanctuaryW", NULL );
            }
            if ( theApp.dwNDeadOnOff = regKey.GetProfileInt( "StartupNDeadOnOff", FALSE ) ) 
            {
               theApp.dwNDeadX = regKey.GetProfileInt( "NDeadSanctuaryX", NULL );
               theApp.dwNDeadY = regKey.GetProfileInt( "NDeadSanctuaryY", NULL );
               theApp.dwNDeadW = regKey.GetProfileInt( "NDeadSanctuaryW", NULL );
            }
            else
            {
               theApp.dwNDeadX = 2944;
               theApp.dwNDeadY = 1059;
               theApp.dwNDeadW = 0;
            }

            theApp.dwDeadSpellID           = regKey.GetProfileInt   ( "DeadSpellID",  0x00 );        
            theApp.dwJailSpellCasted       = regKey.GetProfileInt   ( "PVPSystemJailSpell",  0x00 ); 
            theApp.dwResetProfessionRemort = regKey.GetProfileInt   ( "RESET_PROFESSION_REMORT",  0x00 ); 
			}
			else
			{
            theApp.sMaxCharactersPerAccount = 3;
            theApp.dwDebugSkillParryDisabled = 0;
            theApp.dwLogXPGains = 0;
            theApp.dwHideUncoverEffectDisabled = 0;
            theApp.dwRobWhileWalkingEnabled = 0;
            theApp.dwRobWhileBeingAttackedEnabled = 0;
            theApp.dwDisableItemInfo = 0;
            theApp.dwUseGMAuth = 0;
            theApp.dwUseNMSDeathSystem = 1;

            theApp.sColosseum = 0;
            theApp.sDoppelganger = 0;
            theApp.sMaxRemorts = 3;
            theApp.dwAHSystemEnable  = 1;
            theApp.dwAHSystemMaxSold = 10;	
            theApp.dwGuildSystemEnable = 1;
            theApp.dwReloadEnable = 1;
            theApp.dwNMSGOLDEnable = 1;
            theApp.dwChestListEnable = 1;
            theApp.dwShareXPDropEnable = 0;
            theApp.dwTimedUnitEnable = 0;
            theApp.dwTimedUnitLockExpire = 20000; //20sec
            theApp.dwTimedUnitExpire     = 40000; //40sec

            theApp.dwEquilibrageNewCourbeXPEnable     = 0;
            theApp.dwEquilibrageSkillNewFormulaEnable = 0;
            theApp.dwEquilibrageNewSkillEnable        = 0;

            theApp.dwUDPFilterEnable = 1;
            theApp.dwUDPLogAnalyseEnable = 0;
            theApp.dwProfessionSystemEnable = 1;
            theApp.dwSendDamageHealingSystem = 1;	
			   theApp.dwManageBankInteret = 1;
			   theApp.dwManageScrollXP = 1;
			   theApp.dwAntiplugSystem = 1;
			   theApp.dwAntiplugSystemSec = 10;

            theApp.m_dwRPSystem             = 1;
            theApp.m_dwGMMsgSystem          = 1;
            theApp.m_dwModeRPorHRP          = 1;
            theApp.m_dwXPstat               = 1;
            theApp.m_dwDUELSyetemActif      = 1;
            theApp.m_dwCCShortcut           = 1;
            theApp.m_dwPseudoname           = 1;
            theApp.m_dwPVPSyetem2Actif      = 1;
            theApp.m_dwManagePrisonExit     = 1;
            theApp.m_dwFriendlyBlockPJAttack= 0;
            theApp.dwEnableCOMMMegaPack     = 1;
            theApp.dwEnableCOMMCompression  = 1;
            theApp.dwSendConnectEquipEnable = 1;
            theApp.dwNPCOnPopupEnable       = 1;

            for(int i=0;i<100;i++)
            {
               theApp.m_dwArenaSystem1[0] = 0;
               theApp.m_dwArenaSystem2[0] = 0;
            }
            theApp.m_dwNMS5YearItemnPod   = 0;


            theApp.m_dwFloorDamageSpellID = 0;
            theApp.m_dwLocalTalkRange     = 50;
            theApp.m_dwMinionGemID        = 0;

            theApp.dwGFEnableCoco2 = 0;
            theApp.dwGFEnableCoco3 = 0;
            theApp.dwGFEnableCoco4 = 0;

            theApp.dwForceDethRecall = 0;

            theApp.dwCustomStartupPositionOnOff  = FALSE;
            theApp.dwCustomStartupSanctuaryOnOff = FALSE;
            theApp.dwNDeadOnOff                  = FALSE;
            theApp.dwNDeadX = 2944;
            theApp.dwNDeadY = 1059;
            theApp.dwNDeadW = 0;

            theApp.dwDeadSpellID           = 0;
            theApp.dwJailSpellCasted       = 0;
            theApp.dwResetProfessionRemort = 0;

         }
		}

		//magic world...
		//Load a list of world associated with a Spell ID and work with a selected flags...
		//you can associate world "Incante Light" to Spell LIGHT and work if flag XXXXXX=1
		{
         CString strFile;
         strFile.Format("%s\\KeyWorld.dat",ServerPath);
         
         FILE *pf = NULL;
         fopen_s(&pf,strFile.GetBuffer(0),"rt");
         if(pf)
         {
            char *pRet;
            char strLigne[2048];
            do 
            {
               pRet = fgets(strLigne,2048,pf);
               if(pRet)
               {
                  sMagicWorldSpell newWord;
                  char strTmp[2048];
                  int dwNbr = sscanf_s(strLigne,"%010d,%010d,%s",&newWord.uiFlagID,&newWord.uiSpellID,&strTmp);
                  if(dwNbr == 3)
                  {
                     newWord.strText.Format("%s",strTmp);
                     theApp.m_aSpellWorld.Add(newWord);
                  }
               }
            }while(pRet);
            fclose(pf);
         }

         //sMagicWorldSpell nWorld;
         //nWorld.strText.Format("batard");
         //nWorld.uiSpellID = 10115;
         //nWorld.uiFlagID = 0;

         //theApp.m_aSpellWorld.Add(nWorld);
      }
        
      regKey.Close();
		regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+CHARACTER_KEY );
				
      
		
		theApp.dwChestEncumbranceUpdatedLive       = regKey.GetProfileInt( "ChestEncumbranceUpdatedLive", 0);
		theApp.csChestEncumbranceBoostFormula      = regKey.GetProfileString( "ChestEncumbranceBoostFormula", "0" );
      theApp.csGUILDChestEncumbranceBoostFormula = regKey.GetProfileString( "GUILDChestEncumbranceBoostFormula", "1000000" );

      BoostFormula bfGUILDChestEncumbrance;
      bfGUILDChestEncumbrance.SetFormula( theApp.csGUILDChestEncumbranceBoostFormula );
      theApp.dwGUILDChestEncumbrance =  bfGUILDChestEncumbrance.GetBoost(NULL);
      


      //NMNMNM Still Item ID
      TFormat format;
      int i = 1;
      DWORD dwID = static_cast< DWORD >( regKey.GetProfileInt( "StillItem1", 0 ) );
      while(dwID != 0 )
      {
         theApp.m_aStillItems.Add(dwID);
         i++;
         dwID = static_cast< DWORD >( regKey.GetProfileInt( format( "StillItem%u", i ), 0 ) );
      }

      i = 1;
      dwID = static_cast< int >( regKey.GetProfileInt( "DelayItemID1", 0 ) );
      while(dwID != 0 )
      {
         sDelayItem newItem;
         newItem.uiID    =  dwID;
         newItem.uiFlag  =  static_cast< DWORD >( regKey.GetProfileInt( format( "DelayItemFlag%u", i ), 0 ) );
         newItem.uiDelay =  static_cast< DWORD >( regKey.GetProfileInt( format( "DelayItemDelay%u", i ), 0 ) );
         theApp.m_aDelayItems.Add(newItem);
         i++;
         dwID = static_cast< DWORD >( regKey.GetProfileInt( format( "DelayItemID%u", i ), 0 ) );
      }

      i = 1;
      dwID = static_cast< int >( regKey.GetProfileInt( "ArenaItemID1", 0 ) );
      while(dwID != 0 )
      {
         sArenaItem newItem;
         newItem.uiID    =  dwID;
         theApp.m_aArenaItems.Add(newItem);
         i++;
         dwID = static_cast< DWORD >( regKey.GetProfileInt( format( "ArenaItemID%u", i ), 0 ) );
      }

      i = 1;
      dwID = static_cast< int >( regKey.GetProfileInt( "XPItemID1", 0 ) );
      while(dwID != 0 )
      {
         sXPItem newItem;
         newItem.uiID    =  dwID;
         theApp.m_aXPItems.Add(newItem);
         i++;
         dwID = static_cast< DWORD >( regKey.GetProfileInt( format( "XPItemID%u", i ), 0 ) );
      }

      i = 1;
      dwID = static_cast< int >( regKey.GetProfileInt( "ORItemID1", 0 ) );
      while(dwID != 0 )
      {
         sORItem newItem;
         newItem.uiID    =  dwID;
         theApp.m_aORItems.Add(newItem);
         i++;
         dwID = static_cast< DWORD >( regKey.GetProfileInt( format( "ORItemID%u", i ), 0 ) );
      }
      i = 1;
      dwID = static_cast< int >( regKey.GetProfileInt( "InfSpellFlagID1", 0 ) );
      while(dwID != 0 )
      {
         sInfiniteSpell newSpell;
         newSpell.uiFlagID    =  dwID;
         newSpell.uiSpellID   =  static_cast< DWORD >( regKey.GetProfileInt( format( "InfSpellSpellID%u", i ), 0 ) );
         theApp.m_aInfiniteSpell.Add(newSpell);
         i++;
         dwID = static_cast< DWORD >( regKey.GetProfileInt( format( "InfSpellFlagID%u", i ), 0 ) );
      }


      i = 1;
      dwID = static_cast< int >( regKey.GetProfileInt( "GlobalFlagID1", 0 ) );
      while(dwID != 0 )
      {
         sGlobalFlag newItem;
         newItem.uiID    =  dwID;
         theApp.m_aGlobalFlag.Add(newItem);
         i++;
         dwID = static_cast< DWORD >( regKey.GetProfileInt( format( "GlobalFlagID%u", i ), 0 ) );
      }

      
      regKey.Close();

  
      //LOAD GLOBAL FLAG.....
      regKey.Create(HKEY_LOCAL_MACHINE,theApp.csT4CKEY+GLOBAL_FLAG_SAVE_KEY);
      if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GLOBAL_FLAG_SAVE_KEY ) )
      {
         for(i=0;i<theApp.m_aGlobalFlag.GetSize();i++)
         {
            CString strTmp;
            strTmp.Format("GlobalFlagID%05d",theApp.m_aGlobalFlag[i].uiID);
            DWORD dwValue = static_cast< int >( regKey.GetProfileInt( strTmp.GetBuffer(0), 0 ) );
            if(dwValue > 0)
            {
               TFCMAIN::GlobalFlagSet(theApp.m_aGlobalFlag[i].uiID,dwValue,FALSE);
            }
         }

         //essaie de lire les global flag ID des events...
         for(i=1;i<1000;i++)
         {
            //set les flag de frtiendly de 3000001 a 3001000 //max 9999 mobs different
            CString strTmp;
            strTmp.Format("GlobalFlagID%07d",3000000+i);
            DWORD dwValue = static_cast< int >( regKey.GetProfileInt( strTmp.GetBuffer(0), 0 ) );
            if(dwValue > 0)
            {
               TFCMAIN::GlobalFlagSet(3000000+i,dwValue,FALSE);
            }
         }

      }

	  // Chargement de la configuration d'authentification
	  if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+AUTH_KEY ) )
	  {
		  KEY2TXT( theApp.sAuth.csODBC_DBPwd,            "ODBC_DB_PWD",          "" );
		  KEY2TXT( theApp.sAuth.csODBC_DSN,              "ODBC_DSN",             "T4C Server Authentication" );
		  KEY2TXT( theApp.sAuth.csODBC_Where,            "ODBC_WHERE",           "" );
		  KEY2TXT( theApp.sAuth.csODBC_Pwd,              "ODBC_PWD_FLD",         "Password" );
		  KEY2TXT( theApp.sAuth.csODBC_Table,            "ODBC_TABLE",           "T4CUsers" );
		  KEY2TXT( theApp.sAuth.csODBC_Account,          "ODBC_NAME_FLD",        "Account" );
		  KEY2TXT( theApp.sAuth.csODBC_DBUser,           "ODBC_DB_USER",         "" );
		  KEY2TXT( theApp.sAuth.csODBC_DBSrcName,        "ODBC_DB_SRCNAME",      "T4C Server" );
        KEY2TXT( theApp.csDBUser,                      "AUTH_DB_USER",         "" );
        KEY2TXT( theApp.csDBPwd,                       "AUTH_DB_PWD",          "" );

		  theApp.sAuth.csODBC_Where.TrimRight();
		  theApp.sAuth.csODBC_Where.TrimLeft();

		  theApp.dwPasswordCaseSensitive  = regKey.GetProfileInt( "PASSWORD_CASE", 0 );			

		  regKey.Close();
	  }
	  else
	  {
        theApp.csDBUser = "";
        theApp.csDBPwd  = "";
		  theApp.sAuth.csODBC_DBPwd = "";
		  theApp.sAuth.csODBC_DSN = "T4C Server Authentication";
		  theApp.sAuth.csODBC_Pwd = "Password";
		  theApp.sAuth.csODBC_Account = "Account";
		  theApp.sAuth.csODBC_DBSrcName = "T4C Server";
		  theApp.dwEncryptedPassword = 0;
		  theApp.dwPasswordCaseSensitive = 0;

		  _LOG_DEBUG
			  LOG_CRIT_ERRORS,
			  "\n     Could not open registry key %s  "AUTH_KEY" required by T4C Server."
			  "\n     Run T4C Server Setup from the control panel to setup the registry keys."
			  "\n     Using default values.",theApp.csT4CKEY
			  LOG_
	  }


	  regKey.Close();




      //NMSGOLD Loading list here...
      regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+NMSGOLD_KEY );


      //Read All Upgrade achat...
      i = 1;
      CString csKey;
      CString csName;
      csName = regKey.GetProfileString( "Upgrade1Name", "$NULL$" );
      while( csName != "$NULL$" )
      {
         sUpgradeListNMG newItems;
         newItems.strName = csName;

         csKey.Format( "Upgrade%uDesc", i );
         newItems.iDesc = regKey.GetProfileInt( csKey, 0 );
         csKey.Format( "Upgrade%uMsgID", i );
         newItems.iMessageID  = regKey.GetProfileInt( csKey, 0 );
         csKey.Format( "Upgrade%uFlagIDMod", i );
         newItems.iFlagMod  = regKey.GetProfileInt( csKey, 0 );
         csKey.Format( "Upgrade%uFlagValue", i );
         newItems.iFlagValue  = regKey.GetProfileInt( csKey, 0 );
         csKey.Format( "Upgrade%uCost", i );
         newItems.iCost  = regKey.GetProfileInt( csKey, 0 );

         theApp.m_aAchatOpt1.Add(newItems);


         csKey.Format( "Upgrade%uName", ++i );
         csName = regKey.GetProfileString( csKey, "$NULL$" );
      }

      //Read all FIX Item Info...
      i = 1;
      csName = regKey.GetProfileString( "FixItem1Name", "$NULL$" );
      while( csName != "$NULL$" )
      {
         sItemListNMG pItem;
         pItem.strName = csName;

         csKey.Format( "FixItem%uID", i );
         pItem.iItemID  = regKey.GetProfileInt( csKey, 0 );

         csKey.Format( "FixItem%uQty", i );
         pItem.iNbrItem  = regKey.GetProfileInt( csKey, 0 );

         csKey.Format( "FixItem%uCost", i );
         pItem.iCost  = regKey.GetProfileInt( csKey, 0 );

         csKey.Format( "FixItem%uDesc", i );
         pItem.iDesc  = regKey.GetProfileInt( csKey, 0 );

         csKey.Format( "FixItemBonus%dName", i );
         pItem.strNameBonus = regKey.GetProfileString( csKey, "" );

         csKey.Format( "FixItemBonus%uID", i );
         pItem.iItemIDBonus  = regKey.GetProfileInt( csKey, 0 );

         csKey.Format( "FixItemBonus%uQty", i );
         pItem.iNbrItemBonus  = regKey.GetProfileInt( csKey, 0 );

         

         theApp.m_aAchatOpt2.Add(pItem);
 
         csKey.Format( "FixItem%uName", ++i );
         csName = regKey.GetProfileString( csKey, "$NULL$" );
      }

      //Read all Construction
      i = 1;
      csName = regKey.GetProfileString( "ConstItem1Name", "$NULL$" );
      while( csName != "$NULL$" )
      {
         sConstListNMG pItem;
         pItem.strName = csName;

         csKey.Format( "ConstItem%uCost", i );
         pItem.iCost  = regKey.GetProfileInt( csKey, 0 );

         csKey.Format( "ConstItem%uDesc", i );
         pItem.iDesc  = regKey.GetProfileInt( csKey, 0 );

         theApp.m_aAchatOpt3.Add(pItem);

         csKey.Format( "ConstItem%uName", ++i );
         csName = regKey.GetProfileString( csKey, "$NULL$" );
      }



      regKey.Close();

      for(int y=0;y<24;y++)
      {
         for(int x=0;x<7;x++)
         {
            theApp.m_abPaidTime[x][y] = 0;
         }
      }
      theApp.m_bHavePaidTime = FALSE;
      
      if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+"TimeGrid" ) )
      {
         CString curKey;
         DWORD   dwVal;
         for(int y=0;y<24;y++)
         {
            for(int x=0;x<7;x++)
            {
               curKey.Format("TimeGrid%02d_%02d",x,y);
               dwVal = regKey.GetProfileInt( curKey, 0xFFFFFFFF );
               if(dwVal != 0xFFFFFFFF)
               {
                  if(dwVal == 0)
                  {
                     theApp.m_abPaidTime[x][y] = 0;
                  }
                  else
                  {
                     theApp.m_abPaidTime[x][y] = 1;
                     theApp.m_bHavePaidTime = TRUE; //we have some hour not FREE.
                  }
               }
               else
               {
                  theApp.m_abPaidTime[x][y] = 0;
               }
            }
         }
      }


      regKey.Close();
      


      // ArenaLocations
      if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+"ARENADeath" ) )
      {
         theApp.arenaLocationList.clear();	
         
         TFormat format;
         
         int i = 1;
         std::string locationId = regKey.GetProfileString( "ArenaLocation1Zone", "$NULL$" );
         while( locationId != "$NULL$" )
         {
            sArenaLocation loc;
            
            loc.wlTopLeft.X         = regKey.GetProfileInt( format( "ArenaLocation%uX1", i ), 0 );
            loc.wlTopLeft.Y         = regKey.GetProfileInt( format( "ArenaLocation%uY1", i ), 0 );
            loc.wlTopLeft.world     = regKey.GetProfileInt( format( "ArenaLocation%uWorld", i ), 0 );
            loc.wlBottomRight.X     = regKey.GetProfileInt( format( "ArenaLocation%uX2", i ), 0 );
            loc.wlBottomRight.Y     = regKey.GetProfileInt( format( "ArenaLocation%uY2", i ), 0 );
            loc.wlBottomRight.world = regKey.GetProfileInt( format( "ArenaLocation%uWorld", i ), 0 );
            
            
            loc.wlRecallAttacker.X     = regKey.GetProfileInt( format( "ArenaLocation%uRecallKillX", i ), -1 );
            loc.wlRecallAttacker.Y     = regKey.GetProfileInt( format( "ArenaLocation%uRecallKillY", i ), -1 );
            loc.wlRecallAttacker.world = regKey.GetProfileInt( format( "ArenaLocation%uRecallKillW", i ), 0 );
            
            loc.wlRecallTarget.X     = regKey.GetProfileInt( format( "ArenaLocation%uRecallDieX", i ), 0 );
            loc.wlRecallTarget.Y     = regKey.GetProfileInt( format( "ArenaLocation%uRecallDieY", i ), 0 );
            loc.wlRecallTarget.world = regKey.GetProfileInt( format( "ArenaLocation%uRecallDieW", i ), 0 );

            loc.iTPMessageID         = regKey.GetProfileInt( format( "ArenaLocation%uRecallMsgID", i ), 0 );
            
            theApp.arenaLocationList.push_back( loc );
            
            ++i;
            locationId = regKey.GetProfileString( format( "ArenaLocation%uZone", i ), "$NULL$" );
         }
         
         regKey.Close();
		}

      // ArenaLocations
      if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+"CombatARENA" ) )
      {
         theApp.CombatArenaLocationList1.clear();	
         theApp.CombatArenaLocationList2.clear();	

         TFormat format;

         int i = 1;
         std::string locationId = regKey.GetProfileString( "CombatArenaLocation1Zone", "$NULL$" );
         while( locationId != "$NULL$" )
         {
            sCombatArenaLocation loc;
            loc.strZOneName.Format("%s",locationId.c_str());
            loc.wlTopLeft.X         = regKey.GetProfileInt( format( "CombatArenaLocation%uX1", i ), 0 );
            loc.wlTopLeft.Y         = regKey.GetProfileInt( format( "CombatArenaLocation%uY1", i ), 0 );
            loc.wlTopLeft.world     = regKey.GetProfileInt( format( "CombatArenaLocation%uWorld", i ), 0 );
            loc.wlBottomRight.X     = regKey.GetProfileInt( format( "CombatArenaLocation%uX2", i ), 0 );
            loc.wlBottomRight.Y     = regKey.GetProfileInt( format( "CombatArenaLocation%uY2", i ), 0 );
            loc.wlBottomRight.world = regKey.GetProfileInt( format( "CombatArenaLocation%uWorld", i ), 0 );

            loc.wlRecallBlueStart.X     = regKey.GetProfileInt( format( "CombatArenaLocation%uRecallBlueXS", i ), -1 );
            loc.wlRecallBlueStart.Y     = regKey.GetProfileInt( format( "CombatArenaLocation%uRecallBlueYS", i ), -1 );
            loc.wlRecallBlueStart.world = regKey.GetProfileInt( format( "CombatArenaLocation%uRecallBlueWS", i ), 0 );

            loc.wlRecallRedStart.X      = regKey.GetProfileInt( format( "CombatArenaLocation%uRecallRedXS", i ), 0 );
            loc.wlRecallRedStart.Y      = regKey.GetProfileInt( format( "CombatArenaLocation%uRecallRedYS", i ), 0 );
            loc.wlRecallRedStart.world  = regKey.GetProfileInt( format( "CombatArenaLocation%uRecallRedWS", i ), 0 );


            loc.wlRecallBlueDead.X     = regKey.GetProfileInt( format( "CombatArenaLocation%uRecallBlueXD", i ), -1 );
            loc.wlRecallBlueDead.Y     = regKey.GetProfileInt( format( "CombatArenaLocation%uRecallBlueYD", i ), -1 );
            loc.wlRecallBlueDead.world = regKey.GetProfileInt( format( "CombatArenaLocation%uRecallBlueWD", i ), 0 );

            loc.wlRecallRedDead.X      = regKey.GetProfileInt( format( "CombatArenaLocation%uRecallRedXD", i ), 0 );
            loc.wlRecallRedDead.Y      = regKey.GetProfileInt( format( "CombatArenaLocation%uRecallRedYD", i ), 0 );
            loc.wlRecallRedDead.world  = regKey.GetProfileInt( format( "CombatArenaLocation%uRecallRedWD", i ), 0 );

            loc.iDescMsgID = regKey.GetProfileInt( format( "CombatArenaLocation%uDescID", i ), 0 );
            loc.iItemID1   = regKey.GetProfileInt( format( "CombatArenaLocation%uItem1ID", i ), 0 );
            loc.iItemID2   = regKey.GetProfileInt( format( "CombatArenaLocation%uItem2ID", i ), 0 );

            loc.wlItemPod1.X      = regKey.GetProfileInt( format( "CombatArenaLocation%uPodX1", i ), 0 );
            loc.wlItemPod1.Y      = regKey.GetProfileInt( format( "CombatArenaLocation%uPodY1", i ), 0 );
            loc.wlItemPod1.world  = regKey.GetProfileInt( format( "CombatArenaLocation%uPodW1", i ), 0 );
            loc.wlItemPod2.X      = regKey.GetProfileInt( format( "CombatArenaLocation%uPodX2", i ), 0 );
            loc.wlItemPod2.Y      = regKey.GetProfileInt( format( "CombatArenaLocation%uPodY2", i ), 0 );
            loc.wlItemPod2.world  = regKey.GetProfileInt( format( "CombatArenaLocation%uPodW2", i ), 0 );
            loc.wlItemPod3.X      = regKey.GetProfileInt( format( "CombatArenaLocation%uPodX3", i ), 0 );
            loc.wlItemPod3.Y      = regKey.GetProfileInt( format( "CombatArenaLocation%uPodY3", i ), 0 );
            loc.wlItemPod3.world  = regKey.GetProfileInt( format( "CombatArenaLocation%uPodW3", i ), 0 );
            loc.wlItemPod4.X      = regKey.GetProfileInt( format( "CombatArenaLocation%uPodX4", i ), 0 );
            loc.wlItemPod4.Y      = regKey.GetProfileInt( format( "CombatArenaLocation%uPodY4", i ), 0 );
            loc.wlItemPod4.world  = regKey.GetProfileInt( format( "CombatArenaLocation%uPodW4", i ), 0 );
            loc.wlItemPod5.X      = regKey.GetProfileInt( format( "CombatArenaLocation%uPodX5", i ), 0 );
            loc.wlItemPod5.Y      = regKey.GetProfileInt( format( "CombatArenaLocation%uPodY5", i ), 0 );
            loc.wlItemPod5.world  = regKey.GetProfileInt( format( "CombatArenaLocation%uPodW5", i ), 0 );

            loc.iSettingsMinPlayer     = regKey.GetProfileInt( format( "CombatArenaLocation%uSettingsMinPlayer", i ), 0 );
            loc.iSettingsMaxPlayer     = regKey.GetProfileInt( format( "CombatArenaLocation%uSettingsMaxPlayer", i ), 0 );
            loc.iSettingsMaxPoint      = regKey.GetProfileInt( format( "CombatArenaLocation%uSettingsMaxPoint", i ), 0 );
            loc.iSettingsMinuteMax     = regKey.GetProfileInt( format( "CombatArenaLocation%uSettingsMinuteMax", i ), 0 );
            loc.iSettingsWaitStartSec  = regKey.GetProfileInt( format( "CombatArenaLocation%uSettingsWaitStartSec", i ), 0 );
            loc.iSettingsWaitDeathSec  = regKey.GetProfileInt( format( "CombatArenaLocation%uSettingsWaitDeathSec", i ), 0 );
            loc.iSettingsTakeItemSec   = regKey.GetProfileInt( format( "CombatArenaLocation%uSettingsTakeItemSec", i ), 0 );

            theApp.CombatArenaLocationList1.push_back( loc );

            ++i;
            locationId = regKey.GetProfileString( format( "CombatArenaLocation%uZone", i ), "$NULL$" );
         }

         i = 1;
         locationId = regKey.GetProfileString( "CombatArena2Location1Zone", "$NULL$" );
         while( locationId != "$NULL$" )
         {
            sCombatArenaLocation loc;
            loc.strZOneName.Format("%s",locationId.c_str());
            loc.wlTopLeft.X         = regKey.GetProfileInt( format( "CombatArena2Location%uX1", i ), 0 );
            loc.wlTopLeft.Y         = regKey.GetProfileInt( format( "CombatArena2Location%uY1", i ), 0 );
            loc.wlTopLeft.world     = regKey.GetProfileInt( format( "CombatArena2Location%uWorld", i ), 0 );
            loc.wlBottomRight.X     = regKey.GetProfileInt( format( "CombatArena2Location%uX2", i ), 0 );
            loc.wlBottomRight.Y     = regKey.GetProfileInt( format( "CombatArena2Location%uY2", i ), 0 );
            loc.wlBottomRight.world = regKey.GetProfileInt( format( "CombatArena2Location%uWorld", i ), 0 );

            loc.wlRecallBlueStart.X     = regKey.GetProfileInt( format( "CombatArena2Location%uRecallBlueXS", i ), -1 );
            loc.wlRecallBlueStart.Y     = regKey.GetProfileInt( format( "CombatArena2Location%uRecallBlueYS", i ), -1 );
            loc.wlRecallBlueStart.world = regKey.GetProfileInt( format( "CombatArena2Location%uRecallBlueWS", i ), 0 );

            loc.wlRecallRedStart.X      = regKey.GetProfileInt( format( "CombatArena2Location%uRecallRedXS", i ), 0 );
            loc.wlRecallRedStart.Y      = regKey.GetProfileInt( format( "CombatArena2Location%uRecallRedYS", i ), 0 );
            loc.wlRecallRedStart.world  = regKey.GetProfileInt( format( "CombatArena2Location%uRecallRedWS", i ), 0 );


            loc.wlRecallBlueDead.X     = regKey.GetProfileInt( format( "CombatArena2Location%uRecallBlueXD", i ), -1 );
            loc.wlRecallBlueDead.Y     = regKey.GetProfileInt( format( "CombatArena2Location%uRecallBlueYD", i ), -1 );
            loc.wlRecallBlueDead.world = regKey.GetProfileInt( format( "CombatArena2Location%uRecallBlueWD", i ), 0 );

            loc.wlRecallRedDead.X      = regKey.GetProfileInt( format( "CombatArena2Location%uRecallRedXD", i ), 0 );
            loc.wlRecallRedDead.Y      = regKey.GetProfileInt( format( "CombatArena2Location%uRecallRedYD", i ), 0 );
            loc.wlRecallRedDead.world  = regKey.GetProfileInt( format( "CombatArena2Location%uRecallRedWD", i ), 0 );

            loc.iDescMsgID = regKey.GetProfileInt( format( "CombatArena2Location%uDescID", i ), 0 );
            loc.iItemID1   = regKey.GetProfileInt( format( "CombatArena2Location%uItem1ID", i ), 0 );
            loc.iItemID2   = regKey.GetProfileInt( format( "CombatArena2Location%uItem2ID", i ), 0 );

            loc.wlItemPod1.X      = regKey.GetProfileInt( format( "CombatArena2Location%uPodX1", i ), 0 );
            loc.wlItemPod1.Y      = regKey.GetProfileInt( format( "CombatArena2Location%uPodY1", i ), 0 );
            loc.wlItemPod1.world  = regKey.GetProfileInt( format( "CombatArena2Location%uPodW1", i ), 0 );
            loc.wlItemPod2.X      = regKey.GetProfileInt( format( "CombatArena2Location%uPodX2", i ), 0 );
            loc.wlItemPod2.Y      = regKey.GetProfileInt( format( "CombatArena2Location%uPodY2", i ), 0 );
            loc.wlItemPod2.world  = regKey.GetProfileInt( format( "CombatArena2Location%uPodW2", i ), 0 );
            loc.wlItemPod3.X      = regKey.GetProfileInt( format( "CombatArena2Location%uPodX3", i ), 0 );
            loc.wlItemPod3.Y      = regKey.GetProfileInt( format( "CombatArena2Location%uPodY3", i ), 0 );
            loc.wlItemPod3.world  = regKey.GetProfileInt( format( "CombatArena2Location%uPodW3", i ), 0 );
            loc.wlItemPod4.X      = regKey.GetProfileInt( format( "CombatArena2Location%uPodX4", i ), 0 );
            loc.wlItemPod4.Y      = regKey.GetProfileInt( format( "CombatArena2Location%uPodY4", i ), 0 );
            loc.wlItemPod4.world  = regKey.GetProfileInt( format( "CombatArena2Location%uPodW4", i ), 0 );
            loc.wlItemPod5.X      = regKey.GetProfileInt( format( "CombatArena2Location%uPodX5", i ), 0 );
            loc.wlItemPod5.Y      = regKey.GetProfileInt( format( "CombatArena2Location%uPodY5", i ), 0 );
            loc.wlItemPod5.world  = regKey.GetProfileInt( format( "CombatArena2Location%uPodW5", i ), 0 );

            loc.iSettingsMinPlayer     = regKey.GetProfileInt( format( "CombatArena2Location%uSettingsMinPlayer", i ), 0 );
            loc.iSettingsMaxPlayer     = regKey.GetProfileInt( format( "CombatArena2Location%uSettingsMaxPlayer", i ), 0 );
            loc.iSettingsMaxPoint      = regKey.GetProfileInt( format( "CombatArena2Location%uSettingsMaxPoint", i ), 0 );
            loc.iSettingsMinuteMax     = regKey.GetProfileInt( format( "CombatArena2Location%uSettingsMinuteMax", i ), 0 );
            loc.iSettingsWaitStartSec  = regKey.GetProfileInt( format( "CombatArena2Location%uSettingsWaitStartSec", i ), 0 );
            loc.iSettingsWaitDeathSec  = regKey.GetProfileInt( format( "CombatArena2Location%uSettingsWaitDeathSec", i ), 0 );
            loc.iSettingsTakeItemSec   = regKey.GetProfileInt( format( "CombatArena2Location%uSettingsTakeItemSec", i ), 0 );

            theApp.CombatArenaLocationList2.push_back( loc );

            ++i;
            locationId = regKey.GetProfileString( format( "CombatArena2Location%uZone", i ), "$NULL$" );
         }
         

         regKey.Close();
      }

		// Mestoph (27/04/2009) : Chargement de la configuration du syst?me contre les cheats
		{
			RegKeyHandler regKey;
            if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+CHEAT_KEY ) )
			{
				// SpeedHack
				if (regKey.GetProfileInt( "SPEEDHACK_ACTIVE", 0 ) > 0) 
               theApp.bSpeedHackActive = true;
				else 
               theApp.bSpeedHackActive = false;

				theApp.iSpeedHackDelay = static_cast< int >( regKey.GetProfileInt( "SPEEDHACK_DELAY", SH_DEF_DELAY ) );
				if (theApp.iSpeedHackDelay < 5) 
               theApp.iSpeedHackDelay = SH_DEF_DELAY;
				theApp.iSpeedHackMaxMovments = static_cast< int >( regKey.GetProfileInt( "SPEEDHACK_MAX_MOVEMENTS", SH_DEF_MOVMENTS ) );
				if (theApp.iSpeedHackMaxMovments < 30) 
               theApp.iSpeedHackMaxMovments = SH_DEF_MOVMENTS;
			}
			else
			{
				theApp.bSpeedHackActive = false;
				theApp.iSpeedHackDelay = SH_DEF_DELAY;
				theApp.iSpeedHackMaxMovments = SH_DEF_MOVMENTS;
			}
		}
		// Fin du chargement de la configuration du syst?me contre les cheats

		CT4CLog::InitLogs();
				
		// End of moved lines
		_LOG_DEBUG  LOG_ALWAYS, "-+----------------+-" LOG_
		_LOG_DEBUG  LOG_ALWAYS, "Starting T4C server." LOG_
		_LOG_DEBUG  LOG_ALWAYS, Version::sBuildStamp().c_str() LOG_
		
		/*
		if ( theApp.sOracleHB.csState )
		{
			_LOG_DEBUG  LOG_DEBUG_LVL4, "Oracle Heart Beat Delay: %u",theApp.sOracleHB.csDelay LOG_
			_LOG_DEBUG  LOG_DEBUG_LVL4, "Oracle Heart Beat State: %u",theApp.sOracleHB.csState LOG_
		}*/

      _LOG_DEATH  LOG_ALWAYS, "-+----------------+-" LOG_
      _LOG_DEATH  LOG_ALWAYS, "Starting T4C server." LOG_
      _LOG_GAMEOP LOG_ALWAYS, "-+----------------+-" LOG_
      _LOG_GAMEOP LOG_ALWAYS, "Starting T4C server." LOG_
      _LOG_PC     LOG_ALWAYS, "-+----------------+-" LOG_
      _LOG_PC     LOG_ALWAYS, "Starting T4C server." LOG_
      _LOG_PAGE   LOG_ALWAYS, "-+----------------+-" LOG_
      _LOG_PAGE   LOG_ALWAYS, "Starting T4C server." LOG_
      _LOG_ITEMS  LOG_ALWAYS, "-+----------------+-" LOG_
      _LOG_ITEMS  LOG_ALWAYS, "Starting T4C server." LOG_
      _LOG_NPCS   LOG_ALWAYS, "-+----------------+-" LOG_
      _LOG_NPCS   LOG_ALWAYS, "Starting T4C server." LOG_
      _LOG_WORLD  LOG_ALWAYS, "-+----------------+-" LOG_
      _LOG_WORLD  LOG_ALWAYS, "Starting T4C server." LOG_
      _LOG_AH     LOG_ALWAYS, "-+----------------+-" LOG_
      _LOG_AH     LOG_ALWAYS, "Starting T4C server." LOG_
      _LOG_CHEAT  LOG_ALWAYS, "-+----------------+-" LOG_
      _LOG_CHEAT  LOG_ALWAYS, "Starting T4C server." LOG_
      _LOG_ACHAT_NMS  LOG_ALWAYS, "-+----------------+-" LOG_
      _LOG_ACHAT_NMS  LOG_ALWAYS, "Starting T4C server." LOG_
      _LOG_SPECIAL_ITEMS  LOG_ALWAYS, "-+----------------+-" LOG_
      _LOG_SPECIAL_ITEMS  LOG_ALWAYS, "Starting T4C server." LOG_
      _LOG_SANCTION  LOG_ALWAYS, "-+----------------+-" LOG_
      _LOG_SANCTION  LOG_ALWAYS, "Starting T4C server." LOG_
      _LOG_MONSTERS  LOG_ALWAYS, "-+----------------+-" LOG_
      _LOG_MONSTERS  LOG_ALWAYS, "Starting T4C server." LOG_
	  _LOG_GUILDHIST  LOG_ALWAYS, "-+----------------+-" LOG_
	  _LOG_GUILDHIST  LOG_ALWAYS, "Starting T4C server." LOG_
	  _LOG_GUILDCHEST  LOG_ALWAYS, "-+----------------+-" LOG_
	  _LOG_GUILDCHEST  LOG_ALWAYS, "Starting T4C server." LOG_
     _LOG_BDEXTCH  LOG_ALWAYS, "-+----------------+-" LOG_
     _LOG_BDEXTCH  LOG_ALWAYS, "Starting T4C server." LOG_
     _LOG_PROFESSION  LOG_ALWAYS, "-+----------------+-" LOG_
     _LOG_PROFESSION  LOG_ALWAYS, "Starting T4C server." LOG_
     _LOG_INTERRP  LOG_ALWAYS, "-+----------------+-" LOG_
     _LOG_INTERRP  LOG_ALWAYS, "Starting T4C server." LOG_
     _LOG_ARENA  LOG_ALWAYS, "-+----------------+-" LOG_
     _LOG_ARENA  LOG_ALWAYS, "Starting T4C server." LOG_
     _LOG_EVENTS  LOG_ALWAYS, "-+----------------+-" LOG_
     _LOG_EVENTS  LOG_ALWAYS, "Starting T4C server." LOG_
     

     

     

      
     
        // Chargement de la configuration du Network
		if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+NETWORK_KEY ) )
		{
			theApp.sNetwork.csRecvIP1  = regKey.GetProfileString( "RECV_IP", "" );
         theApp.sNetwork.csRecvIP2  = regKey.GetProfileString( "RECV2_IP", "" );
			theApp.sNetwork.csRecvIP1.TrimRight();
			theApp.sNetwork.csRecvIP1.TrimLeft();
			theApp.sNetwork.csRecvIP2.TrimRight();
			theApp.sNetwork.csRecvIP2.TrimLeft();

			theApp.sNetwork.wRecvPort1 = static_cast< WORD >( regKey.GetProfileInt( "RECV_PORT", 11677 ) );
         theApp.sNetwork.wRecvPort2 = static_cast< WORD >( regKey.GetProfileInt( "RECV_PORT", 11677 ) );
			regKey.Close();
		}
		else
		{
			theApp.sNetwork.wRecvPort1 = 11677;
			theApp.sNetwork.wRecvPort2 = 11677;

			_LOG_DEBUG
				LOG_CRIT_ERRORS,
				"\n     Could not open registry key %s " NETWORK_KEY " required by T4C Server."
				"\n     Run T4C Server Setup from the control panel to setup the registry keys."
				"\n     Using default values.",theApp.csT4CKEY
			LOG_
		}

        // Chargement de la configuration g?n?rale
		if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY ) )
		{
         theApp.sGeneral.wServerEvents = (WORD)regKey.GetProfileInt( "ServerEvents", 0 ); 
			theApp.sGeneral.wNbWarnings = (WORD)regKey.GetProfileInt( "NB_WARNINGS", 5 ); 
			theApp.sGeneral.dwTimeBeforeWarning = regKey.GetProfileInt( "TIME_BEFORE_WARNING", 2500 ); 
         theApp.sGeneral.dwServerUseAllCPU = regKey.GetProfileInt( "SERVER_USE_ALL_CPU", 0 ); 
         theApp.sGeneral.dwServerBDExtModCheck = regKey.GetProfileInt( "SERVER_BD_EXTERNAL_CHANGE_CHECK", 0 ); 
         HANDLE hProcess = GetCurrentProcess();
         DWORD processMask, systemMask;
         GetProcessAffinityMask( hProcess, &processMask, &systemMask );
         SetProcessAffinityMask( hProcess, regKey.GetProfileInt( "ProcessAffinity", 0xFFFFFFFF ) & processMask);
         TFormat format;

			int nCount = 1;
			CString csLangDB = regKey.GetProfileString( "LangDB1", "$NULL$" );
			while( csLangDB != "$NULL$"  && csLangDB != "")
			{            
				IntlText::LoadLngDB( (LPCTSTR)csLangDB );
				theApp.sGeneral.csLang = csLangDB;
				csLangDB = regKey.GetProfileString( format( "LangDB%u", ++nCount ), "$NULL$" );
			}

			// fill in the theApp.sGeneral.csLang field
			// use: if ( theApp.sGeneral.csLang.Compare("t4c_kor.elng") == 0 ) for korean specific actions			
            // If the language configuration isn't valid.
            if( !IntlText::IsLngOK() )
			{
                _LOG_DEBUG
                    LOG_CRIT_ERRORS,
                    "Could not find any valid language database. Server cannot be started."
                LOG_
                MessageBeep( -1 );
                return false;
            }

		}
		else
		{
         theApp.sGeneral.wServerEvents = 0;
			theApp.sGeneral.wNbWarnings = 5;
			theApp.sGeneral.dwTimeBeforeWarning = 2500;
         theApp.sGeneral.dwServerUseAllCPU = 0; 
         theApp.sGeneral.dwServerBDExtModCheck = 0;

			_LOG_DEBUG
				LOG_CRIT_ERRORS,
				"\n     Could not open registry key %s " GEN_CFG_KEY " required by T4C Server."
				"\n     Run T4C Server Setup from the control panel to setup the registry keys."
				"\n     Using default values.",theApp.csT4CKEY
			LOG_
		}

		{ 
			SYSTEMTIME sysTime; 
			GetLocalTime(&sysTime);

			_EXITLOG "\r\n+++[STARTUP] %u/%u/%u %02u:%02u:%02u +++",
                sysTime.wMonth,
                sysTime.wDay,
                sysTime.wYear, 
                sysTime.wHour, 
                sysTime.wMinute,
                sysTime.wSecond
            EXITLOG_
        }

        

        regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+CHARACTER_KEY );


		TRACE(_T("**%s\r\n"), (LPCTSTR)ServerPath);
		
		CString messagepath = ServerPath + "T4C Messages.dll";
			
		// If the directory specified by "ServerPath" is invalid, inform the user and use normal path
		CFileFind check;
		if(!check.FindFile(ServerPath + ".")){
			LPCTSTR param[2] = {(LPCTSTR)ServerPath, path};
			
			TRACE(_T("%s--"), param[0]);
			TRACE(_T("%s"), param[1]);

			_LOG_DEBUG LOG_ALWAYS, "Server path %s not found", (LPCTSTR)ServerPath LOG_

			messagepath = path;
			messagepath += "T4C Messages.dll";
		}

		{	
         char strKeyname[512];
         if(iSvrID == 0)
            sprintf_s(strKeyname,512,"SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\T4C Server");
         else
            sprintf_s(strKeyname,512,"SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\T4C Server%d",iSvrID+1);

         RegKeyHandler EventLog;
         if(!EventLog.Open(HKEY_LOCAL_MACHINE, strKeyname))
         {
            _LOG_DEBUG LOG_MISC_1, "Creating new NT EventViewer application entry: %s", strAppname LOG_

            EventLog.Create(HKEY_LOCAL_MACHINE, strKeyname);
            EventLog.WriteProfileString(_T("EventMessageFile"), (LPCTSTR)messagepath);
            EventLog.WriteProfileInt(_T("TypesSupported"),  EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE);
         }   
		
		}

		theApp.dwPVPDropDisabled = 0;
      theApp.dwPVMDropDisabled = 0;

#ifdef _WIN32
		//init the service with the SCM, if the program is to be started as a service
      CUService service;
		if(startAsService)
		{	
			//setup the service class callback functions
			service.SetStopTimeOut( 180000 );
			service.SetCallBacks(NULL, NULL, StopFunction);
			// Initialize the SCM.
			if(service.InitService(_T("T4C_Server"), EntryFunction) != TRUE) 
			{
				theApp.InService = FALSE;
            exit(COULDNT_START_AS_A_SERVICE);
			}
		}
		//just call the programs main entry point, if it is not to start as a service
		else
		{
			EntryFunction(NULL);
			return NORMAL_SERVER_EXIT;
		}
#else
		EntryFunction(NULL);
		return NORMAL_SERVER_EXIT;
#endif

	}
	catch( ... )
	{

#ifdef _DEBUG
		throw;
#endif
		throw;

		FILE *fDbg;
		fopen_s(&fDbg, "T4CServerGP.out", "ab" );
		if( fDbg )
		{
			time_t current_time;
			time(&current_time);
         char strTime[128];
         ctime_s(strTime,128,&current_time) ;

			fprintf( fDbg, "\r\n----------------------------------------------------------------------------");
			fprintf( fDbg, "\r\nT4C Server v%s - Build l - Main. General Protection fault report, %s\r\n\r\n", STR_REVISION, strTime);
			fprintf( fDbg, "Server crashed during InitInstance initialisation." );
			fclose( fDbg );
		}
		// Terminates main pump.
		MessageBeep( -1 );
		return FALSE;
	}
	return TRUE;
}
/******************************************************************************/
// This is the service entry-point.
void CDECL EntryFunction(void *cu)
/******************************************************************************/
{ 
	CAutoThreadMonitor tmMonitor("Console (EntryFunction)");

#ifdef __ENABLE_LOG
	if(__LOG > 0) __LOG("Entering the EntryFunction");
#endif

	try
	{
#ifdef _WIN32
      CoInitialize( NULL );
#endif

      DWORD dwVersion = SERVER_CONNECTION_HI_VERSION;
      {
         RegKeyHandler regKey;
         regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY );

         // Fetch version from registry, otherwise default to the executable's version.
         dwVersion = regKey.GetProfileInt( "Version", SERVER_CONNECTION_HI_VERSION );
         regKey.Close();
      }

      printf( "-------------------------------------------------------------------------------\n"
         "%s.\n"
         "Copyright(c) Dialsoft inc. 1998-2011+. All rights reserved.\n"
         "-------------------------------------------------------------------------------\n",
         Version::sBuildStamp().c_str()
         );

        CString csVersion;
        csVersion.Format( VERSION_STRING, dwVersion );
		
		_LOG_DEBUG
            LOG_DEBUG_LVL1,
            "Loading version %s",
            (LPCTSTR)csVersion
        LOG_
        
        try
		{
            TFCServer = new TFC_MAIN;
            _LOG_DEBUG
                LOG_CRIT_ERRORS,
                "Creating TFC_MAIN object."
            LOG_
        }
		catch(...)
		{
            _LOG_DEBUG
                LOG_CRIT_ERRORS,
                "Crashed when creating TFC_MAIN object."
            LOG_
            throw;
        }        

        try
		{
            TFCInitMaps();
        }
		catch(...)
		{
            _LOG_DEBUG
                LOG_CRIT_ERRORS,
                "Crashed when initializing worlds database."
            LOG_
            throw;
        }
		        
        _LOG_DEBUG
            LOG_DEBUG_LVL1,
            "InitThread Id=%u",
            GetCurrentThreadId()
        LOG_

        // Start the player and packet manager.
        CAutoConfig::Create( HKEY_LOCAL_MACHINE, theApp.csT4CKEY, "RegUpdate" );

        SysopCmd::Create();
        printf( "\n- Initialized System operator/GM commands" ); //BLBLBL added verbose output

        _LOG_DEBUG
            LOG_DEBUG_LVL3,
            "Initialized System operator/GM commands"
        LOG_  
		
        GAME_RULES::Create();
        printf( "\n- Initialized Game rules" );
        
		_LOG_DEBUG
            LOG_DEBUG_LVL3,
            "Initialized Game rules"
        LOG_        
        
		CDeadlockDetector::Create();
      printf( "\n- Initialized Deadlock detector" );

		_LOG_DEBUG
            LOG_DEBUG_LVL3,
            "Initialized Deadlock detector"
        LOG_

      CPlayerManager::Create(); 
      TFCMessagesHandler::Create();
      printf( "\n- Initialized Messages handler" );	

      _LOG_DEBUG
         LOG_DEBUG_LVL3,
         "Initialized Messages handler"
         LOG_        

         // Get the sole instance, this also creates the NPC thread.
         NPCMain::GetInstance();
         NPCMain::GetInstance().Create();
         printf( "\n- Initialized NPC and main instance" );

      _LOG_DEBUG
         LOG_DEBUG_LVL1,
         "Initialized NPC and main instance : NPC Thread Id=%u",
         NPCMain::GetInstance().GetThreadId()
         LOG_

         theApp.serverStarted = true;

      TFCMAIN::StartBeat();
      _LOG_DEBUG
         LOG_DEBUG_LVL3,
         "Initialized HeartBeat"
         LOG_


     
     _LOG_DEBUG
        LOG_DEBUG_LVL3,
        "Initialized Packet manager"
        LOG_   
	   CPacketManager::Create();
      printf( "\n- Initialized Packet manager" );

      
      Professions::Create();
      QuestBook::Create();
      GuildMaster::Create();
      AuctionMaster::Create();
      GMMsgMaster::Create();
      RPMaster::Create();
      Arena1Master::Create(theApp.CombatArenaLocationList1.size());
      Arena2Master::Create(theApp.CombatArenaLocationList2.size());
      NMS5YearsEvents::Create();
      EventsMaster::Create();

#ifdef _WIN32
      //Start WEB Server if is enabled./..
      if(theApp.dwEnableWebServer>0)
      {
         CPacketManager::StartWebServer();
      }

	   NMPacketManager *lpComm = CPacketManager::GetCommCenter();
	   lpComm->m_bCanStartComm = TRUE;
#endif

 	   printf( "\n- Initialized Player manager\n\n" );

        _LOG_DEBUG
            LOG_DEBUG_LVL3,
            "Initialized Player manager"
		LOG_
		
		// Sends the MSG_OnServerInitialisation
		Unit::SendGlobalUnitMessage( MSG_OnServerInitialisation, NULL, NULL, NULL );
		
        _LOG_DEBUG
            LOG_DEBUG_LVL3,
            "Initialized Global Unit Message"
		LOG_

		//SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);

        _LOG_DEBUG
            LOG_DEBUG_LVL3,
            "Initialized Thread Priority"
		LOG_
		
      

        // Let the main console take control of this thread.
        MainConsole::GetInstance().TakeControl();

        _LOG_DEBUG
            LOG_DEBUG_LVL3,
            "Initialized Main console"
		LOG_

        

	}
	catch( ... )
	{
		_LOG_DEBUG LOG_CRIT_ERRORS, "Crashed during T4C Server initialization. EntryFunction section." LOG_
      throw;
	}

    exit( TERMINATE_FAIL_INIT );
}
/******************************************************************************/
void __cdecl StopFunction(void *cus)
/******************************************************************************/
{	
    // Terminate main console.
    MainConsole::GetInstance().Terminate();
}
