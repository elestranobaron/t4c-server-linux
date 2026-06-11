/******************************************************************************
vs2008 (31/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "TFC Server.h"
#include "PlayerManager.h"
#include "TFC_MAIN.h"
#include "UDP/NMPacketManager.h"
#include "PacketManager.h"
#include "DeadlockDetector.h" 
#include "RegKeyHandler.h"
#include "ThreadMonitor.h"
#include "AuctionMaster.h"
#include "GuildMaster.h"
#include "RPMaster.h"
#include "Arena1Master.h"
#include "Arena2Master.h"
#include "NMS5YearsEvents.h"
#include "EventsMaster.h"
#include "GMMsgMaster.h"
#include "AsyncFuncQueue.h"
#include "DevTools/PerformUtil.h"
#include "StatModifierFlagsListing2.h"

#ifndef _WIN32
#include <cstdio>
#endif


#ifdef _DEBUG
	#undef THIS_FILE
	static char THIS_FILE[]=__FILE__;
	#define new DEBUG_NEW 
#endif
 
/******************************************************************************/
#define INITIAL_SIZE        250
#define GROW_BY             10

// Utility macro.
#ifdef _WIN32
#define SAME_IP( sock1, sock2 ) (sock1).sin_addr.S_un.S_addr == (sock2).sin_addr.S_un.S_addr &&\
                                (sock1).sin_port             == (sock2).sin_port
#else
#define SAME_IP( sock1, sock2 ) (sock1).sin_addr.s_addr == (sock2).sin_addr.s_addr &&\
                                (sock1).sin_port         == (sock2).sin_port
#endif

/******************************************************************************/
// externs
extern TFC_MAIN *TFCServer;
extern TemplateList< DEATHROW > tluDeathRow;
extern CTFCServerApp theApp;

// static variables
MultiReaderSingleWriter *CPlayerManager::pMultiRSingleW = NULL;
Players **CPlayerManager::lpRegisteredUsers =       NULL;
int       CPlayerManager::nUserCount =              0;
int       CPlayerManager::nBufferSize =             0;
CLock     CPlayerManager::cMaintenanceLock;
HANDLE    CPlayerManager::hMaintenanceThread =      NULL;
HANDLE    CPlayerManager::hSystemProcessThread =    NULL;
HANDLE    CPlayerManager::hAutoCastProcessThread =  NULL;
HANDLE    CPlayerManager::hDeletionIo =             NULL;
ChatterChannels *CPlayerManager::lpChatter = NULL;
DWORD  Round = 0;
static HANDLE hDeleteThread = NULL;
static BOOL        boMaintenance;
static BOOL   boStop = FALSE;

static DWORD m_dwManageAH              = 90  SECONDS TDELAY; //on attend 90 sec
static DWORD m_dwManageAHSave          = 90  SECONDS TDELAY; //on attend 90 sec
static DWORD m_dwManageSaveGuildChest  = 60  SECONDS TDELAY; //on attend 60 sec
static DWORD m_dwManageGMMsg           = 30  SECONDS TDELAY; //on attend 30 sec
static DWORD m_dwManageDuelClean       = 0;
static DWORD m_dwManageDuelDecompte    = 0;
static DWORD m_dwManagePVPUID          = 0;
static DWORD m_dwManageUserServerTime  = 15  MINUTES TDELAY; //on attend 15 sec
static DWORD m_dwManageInterRP         = 60  SECONDS TDELAY; //on attend 60 sec
static DWORD m_dwManageArena           = 1   SECONDS TDELAY; //on attend 1  sec
static DWORD m_dwManageEvents          = 5   SECONDS TDELAY; //on attend 5  sec
static DWORD m_dwWebServer             = 30  SECONDS TDELAY; //on attend 30  sec
static DWORD m_dwManageTimedUnit       = 10  SECONDS TDELAY; //on attend 10  sec



/******************************************************************************/
// Initializes the player manager.
void CPlayerManager::Create( void )
/******************************************************************************/
{   
   pMultiRSingleW = new MultiReaderSingleWriter;
   lpRegisteredUsers = new Players *[ INITIAL_SIZE ];

   int i;
   for( i = 0; i < INITIAL_SIZE; i++ )
      lpRegisteredUsers[ i ] = NULL;

   nUserCount = 0;
   nBufferSize = INITIAL_SIZE;

   // Begin maintenance thread.
   boMaintenance = TRUE;

   UINT threadId;
   hMaintenanceThread     = (HANDLE)_beginthreadex( NULL, 0, PlayerMaintenance , NULL, 0, &threadId ); //OK: thread qui manage tous le systeme de player en entier...
   hSystemProcessThread   = (HANDLE)_beginthreadex( NULL, 0, SystemProcess     , NULL, 0, &threadId ); //OK: thread qui magage les service comme AH, GUILD DUEL, etc etc etc
   hAutoCastProcessThread = (HANDLE)_beginthreadex( NULL, 0, AutoCastProcess   , NULL, 0, &threadId ); //OK: thread qui soccupe de recaster des spell selon un systeme baser au 100 ms
   hDeleteThread          = (HANDLE)_beginthreadex( NULL, 0, AsyncDeletePlayer , NULL, 0, &threadId ); //OK: thread qui delete les player deja virer la la liste,...

   SetThreadPriority(hMaintenanceThread,THREAD_PRIORITY_HIGHEST);

   boStop = FALSE;

   // Start async deletion thread.
   hDeletionIo = CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL, 0, 1 );    

   
}
/******************************************************************************/
// Destroys the player manager.
void CPlayerManager::Destroy( void )
/******************************************************************************/
{
    // Kill maintenance thread.
    boStop = TRUE;
    Sleep(500);
    boMaintenance   = FALSE;
    Sleep(500);


    CancelIo( hDeletionIo );
    if( WaitForSingleObject( hDeleteThread, 11000 ) == WAIT_TIMEOUT ) //CV_VALID attend le max que sa peu prendre a deleter un Player...
       TerminateThread( hDeleteThread, 1 );
    hDeleteThread = NULL;

    if( WaitForSingleObject( hSystemProcessThread, 5000 ) == WAIT_TIMEOUT )
       TerminateThread( hSystemProcessThread, 1 );
    hSystemProcessThread = NULL;

    if( WaitForSingleObject( hAutoCastProcessThread, 5000 ) == WAIT_TIMEOUT )
       TerminateThread( hAutoCastProcessThread, 1 );
    hAutoCastProcessThread = NULL;

    

    if( WaitForSingleObject( hMaintenanceThread, 5000 ) == WAIT_TIMEOUT )
       TerminateThread( hMaintenanceThread, 1 );
    hMaintenanceThread = NULL;

    if (lpRegisteredUsers != NULL)
    {
       int i;
       for( i = 0; i < nBufferSize; i++ )
       {
          if( lpRegisteredUsers[ i ] != NULL )
          {
             delete lpRegisteredUsers[ i ];
             lpRegisteredUsers[ i ] = NULL;
          }
       }
       delete []lpRegisteredUsers;
       lpRegisteredUsers = NULL;
    }

    if(pMultiRSingleW)
       delete pMultiRSingleW;
    pMultiRSingleW = NULL;
}




/******************************************************************************/
// Player maintenance thread.
UINT CPlayerManager::PlayerMaintenance( LPVOID lpData)
/******************************************************************************/
{    
   static int iiCntGlobal = 0;
   CAutoThreadMonitor tmMonitor("Player Maintenance");
   _LOG_DEBUG
      LOG_DEBUG_LVL1,
      "PlayerMaintenance Thread Id=%u",
      GetCurrentThreadId()
      LOG_
      //DC
      //DWORD delay = theApp.sOracleHB.csDelay*60;
      //time_t ttCurTime;	
      //time_t ttHitTime;
      //BOOL OracleHit=false;
      //DC

      CDeadlockDetector cDeadlockDetector;
   cDeadlockDetector.RegisterThread( hMaintenanceThread, "Player Maintenance Thread", 120000  );            

   int i;
   Players *lpPlayer;
   DWORD dwInitialTime = 0;

   TemplateList <Players> tlPlayersToDelete;

   /*
   if (theApp.sOracleHB.csState)
   {
   time( &ttHitTime );
   }
   */

   while( boMaintenance )
   { 
      KEEP_ALIVE
         cMaintenanceLock.Lock();
      try
      {
         NMPacketManager *lpComm = CPacketManager::GetCommCenter();

         pMultiRSingleW->EnterReader();
         if( lpComm != NULL && lpComm->GetServerStarted())
         {
            try
            {
               std::vector< sockaddr_in > *vLostConnections = lpComm->GetLostConnections();
               // If there are lost connections.
               if( !vLostConnections->empty() )
               {
                  _LOG_DEBUG
                     LOG_DEBUG_LVL1,
                     "PM:Found %u lost connections.",
                     vLostConnections->size()
                     LOG_

                     // Find any player who has this IP address

                     for( i = 0; i < nBufferSize; i++ )
                     {
                        lpPlayer = lpRegisteredUsers[ i ];
                        if( lpPlayer != NULL )
                        {
                           std::vector< sockaddr_in >::iterator f;
                           for( f = vLostConnections->begin(); f != vLostConnections->end(); f++ )
                           {
                              // If the IP matches that of a lost connection, delete player.
                              if( SAME_IP( (*f), lpPlayer->IPaddrO ) )
                              {
                                 _LOG_DEBUG
                                    LOG_DEBUG_LVL1,
                                    "PM:Found lost connection player."
                                    LOG_
                                    _LOG_WORLD
                                    LOG_MISC_1,
                                    "Player %s will be disconnected because T4C received a destination unreachable packet.",
                                    (LPCTSTR)lpPlayer->GetFullAccountName()
                                    LOG_
                                    lpPlayer->SetDeletePlayerFlags();
                              }
                           }                                        
                        }
                     }
               }
               lpComm->FreeLostConnections( TRUE );
            }
            catch(...)
            {
               if(lpComm)
                  lpComm->FreeLostConnections( TRUE );

               FILE *pft = NULL;
               fopen_s(&pft,"c:\\__SvrException.txt","a+");
               fprintf(pft,"EXCEPTION--> Player maintenance-->LOST Connection part\n");
               fclose(pft);

               _LOG_DEBUG
                  LOG_DEBUG_LVL1,
                  "EXCEPTION--> Player maintenance-->LOST Connection part"
                  LOG_
            }
         }
         pMultiRSingleW->LeaveReader();

         //bsccLogger::GetInstance()->Add(LOG_L1,"PM","P2");
         // Scroll through all players currently in game for maintenance.
         pMultiRSingleW->EnterReader();
         try
         {
            for( i = 0; i < nBufferSize; i++ )
            {
               KEEP_ALIVE;
               lpPlayer = lpRegisteredUsers[ i ];

               // If player 
               if( lpPlayer != NULL )
               {
                  // If the player ressource could be fetched.
                  if( lpPlayer->PickLock() )
                  {
                     try
                     {
                        // If the maintenance thread should delete this player. ( Idle or to delete ).
                        if( ( lpPlayer->IsDeleteFlags() || lpPlayer->IsIdle() || lpPlayer->dwExitDecompte == 0) && !lpPlayer->IsLoading())
                        {
                           //bsccLogger::GetInstance()->AddFmt(LOG_L1,"PM","P2 Logoff Account %s",lpPlayer->GetAccount());
                           if( lpPlayer->IsDeleteFlags() )
                           {
                              _LOG_DEBUG
                                 LOG_DEBUG_HIGH,
                                 "Flushing player %s because delete flag was true.",
                                 (LPCTSTR)lpPlayer->GetFullAccountName()
                                 LOG_
                           }

                           if(lpPlayer->dwExitDecompte == 0)
                           {
                              TFCPacket sending;
                              sending << (RQ_SIZE)RQ_ExitGameAntiPlug;
                              lpPlayer->self->SendPlayerMessage(sending);
                           }

                           // Delete player.
                           tlPlayersToDelete.AddToTail( lpPlayer );
                        }
                        else
                        {
                           // Require lock on Unit structure to do unit specific maintenance
                           //MINIONS maintenance here
                           if(lpPlayer->self->PickLock() )
                           {  
                              try
                              {
                                 if(lpPlayer->in_game)
                                 {
                                    // Can player regen?
                                    if(lpPlayer->dwRegenTime <= TFCMAIN::GetRound() )
                                    {
                                       // Regen hp and mana.
                                       lpPlayer->self->Regenerate();
                                       lpPlayer->self->ProcessYoursHealingDisplay();
                                       lpPlayer->self->SendPlayerXP(false);
                                       lpPlayer->self->CheckIFLevelUP();
                                       lpPlayer->self->NeedUpdateGold();
                                       lpPlayer->dwRegenTime = 2 SECONDS TDELAY;
                                    }

                                    if(lpPlayer->dwDeathAndNoXPTime <= TFCMAIN::GetRound() )
                                    {
                                       lpPlayer->self->ProcessNMDeath();
                                       lpPlayer->self->SendIsXP();
                                       lpPlayer->dwDeathAndNoXPTime = 5 SECONDS TDELAY;
                                    }

                                    if(theApp.dwAntiplugSystem == 1)
                                    {
                                       if(lpPlayer->dwExitDecompte != 0xFFFF  && lpPlayer->dwAntiPlugTime <= TFCMAIN::GetRound() )
                                       {
                                          CString csText;
                                          csText.Format(_STR(15235, lpPlayer->self->GetLang()),lpPlayer->dwExitDecompte);
                                          lpPlayer->self->SendSystemMessage(csText);

                                          if(lpPlayer->dwExitDecompte >0)
                                             lpPlayer->dwExitDecompte--;

                                          lpPlayer->dwAntiPlugTime = 1 SECONDS TDELAY;
                                       }

                                       if(lpPlayer->dwReloadDecompte != 0xFFFF  && lpPlayer->dwAntiPlugTimeReload <= TFCMAIN::GetRound() )
                                       {
                                          CString csText;
                                          csText.Format(_STR(15394, lpPlayer->self->GetLang()),lpPlayer->dwReloadDecompte);
                                          lpPlayer->self->SendSystemMessage(csText);

                                          if(lpPlayer->dwReloadDecompte >0)
                                             lpPlayer->dwReloadDecompte--;
                                          else
                                          {
                                             lpPlayer->dwReloadDecompte = 0xFFFF;
                                             if(lpPlayer->in_AskReloadPlayer == FALSE)
                                                lpPlayer->in_AskReloadPlayer = TRUE;
                                          }

                                          lpPlayer->dwAntiPlugTimeReload = 1 SECONDS TDELAY;
                                       }
                                    }

                                    if(theApp.dwManageBankInteret)
                                    {
                                       time_t tCurTime  =  time(NULL);

                                       //NMNMNMNM manage NMS Bank...
                                       //////////////////////////////////////////////////////////////////////////////////////////////////////
                                       if(lpPlayer->self->ViewFlag(__FLAG_NMSBANK_TYPE_CDOMPTE) !=0  && lpPlayer->self->ViewFlag(__FLAG_NMSBANK_NEXT_INTERET_TIME) == 0)
                                       {
                                          //premiere ouverture de compte...
                                          //srt le montant minimum a ce qui a dans son compte, set le jour<s des interer a +7 jours...
                                          lpPlayer->self->SetFlag(__FLAG_NMSBANK_OR_MINIMUM_WEEKS,lpPlayer->self->ViewFlag(__FLAG_NMSBANK_OR_EN_BANK));
                                          lpPlayer->self->SetFlag(__FLAG_NMSBANK_NEXT_INTERET_TIME,tCurTime + (7*86400)); //now + 7 jours
                                          //lpPlayer->self->SetFlag(__FLAG_NMSBANK_NEXT_INTERET_TIME,tCurTime + (3*60)); //3 minutes LOL
                                       }
                                       else if(lpPlayer->self->ViewFlag(__FLAG_NMSBANK_TYPE_CDOMPTE) !=0)
                                       {
                                          //oki donc le joueur a un compte on peu manager...
                                          // 1 : on verifie pour l;<or minimum dans le compte...
                                          if(lpPlayer->self->ViewFlag(__FLAG_NMSBANK_OR_EN_BANK) < lpPlayer->self->ViewFlag(__FLAG_NMSBANK_OR_MINIMUM_WEEKS))
                                          {
                                             lpPlayer->self->SetFlag(__FLAG_NMSBANK_OR_MINIMUM_WEEKS,lpPlayer->self->ViewFlag(__FLAG_NMSBANK_OR_EN_BANK));
                                          }

                                          //on verifie si c le temps des inetert ou pas...
                                          if(tCurTime > lpPlayer->self->ViewFlag(__FLAG_NMSBANK_NEXT_INTERET_TIME) && lpPlayer->self->GetTarget() == NULL)
                                          {
                                             //le temps de payer les interer... ou les interet a decouvert...
                                             if(lpPlayer->self->ViewFlag(__FLAG_NMSBANK_OR_MINIMUM_WEEKS) >0)
                                             { 
                                                //interet positif...
                                                int iInteret = (lpPlayer->self->ViewFlag(__FLAG_NMSBANK_OR_MINIMUM_WEEKS)*5)/100;
                                                //on dois envoyer a recevoir ce montant argernt la au PJ...
                                                //Character *lpCharacter = static_cast< Character * >( lpPlayer->self );
                                                theApp.AddAHRequest(NULL,NULL,NULL,AH_BANK_INTERET,lpPlayer->self->GetID(),iInteret,0,0,0,0,0,"","","",0);

                                                _LOG_SPECIAL_ITEMS
                                                   LOG_ALWAYS,
                                                   "Player %s (Account: %s) -->Bank interet Positif...   add 5 pourcent (%d golds) to AH System",
                                                   (LPCTSTR)lpPlayer->self->GetTrueName(),
                                                   (LPCTSTR)lpPlayer->GetFullAccountName(),
                                                   iInteret
                                                   LOG_
                                             }
                                             else if(lpPlayer->self->ViewFlag(__FLAG_NMSBANK_OR_MINIMUM_WEEKS) <0)
                                             {
                                                //interet negatif...
                                                int iInteret = (abs(lpPlayer->self->ViewFlag(__FLAG_NMSBANK_OR_MINIMUM_WEEKS))*10)/100;
                                                //set ce montant
                                                lpPlayer->self->SetFlag(__FLAG_NMSBANK_OR_EN_BANK,lpPlayer->self->ViewFlag(__FLAG_NMSBANK_OR_EN_BANK)-iInteret);
                                                if(lpPlayer->self->ViewFlag(__FLAG_NMSBANK_OR_EN_BANK) < -2*lpPlayer->self->ViewFlag(__FLAG_NMSBANK_MAX_EMPRUNT))  
                                                   lpPlayer->self->SetFlag(__FLAG_NMSBANK_OR_EN_BANK,-2*lpPlayer->self->ViewFlag(__FLAG_NMSBANK_MAX_EMPRUNT));

                                                _LOG_SPECIAL_ITEMS
                                                   LOG_ALWAYS,
                                                   "Player %s (Account: %s) -->Bank interet Negatif...   remove 10 pourcent (%d golds)",
                                                   (LPCTSTR)lpPlayer->self->GetTrueName(),
                                                   (LPCTSTR)lpPlayer->GetFullAccountName(),
                                                   iInteret
                                                   LOG_

                                                   CString csText = _STR( 15151, lpPlayer->self->GetLang() );
                                                lpPlayer->self->SendSystemMessage(csText);

                                             }

                                             //oki now on reset les flag interet pour la semaine dapres...
                                             //On set l<or min a l<or dans le compte et si ye sous emprunt on mets 0...
                                             lpPlayer->self->SetFlag(__FLAG_NMSBANK_OR_MINIMUM_WEEKS,lpPlayer->self->ViewFlag(__FLAG_NMSBANK_OR_EN_BANK));
                                             lpPlayer->self->SetFlag(__FLAG_NMSBANK_NEXT_INTERET_TIME,tCurTime + (7*86400)); //now + 7 jours
                                             //lpPlayer->self->SetFlag(__FLAG_NMSBANK_NEXT_INTERET_TIME,tCurTime + (3*60)); //3 minutes LOL
                                          }
                                       }
                                    }

                                    if(theApp.dwManageScrollXP)
                                    {
                                       time_t tCurTime  =  time(NULL);
                                       //NMNMNMNM manage Scroll XP...
                                       //////////////////////////////////////////////////////////////////////////////////////////////////////
                                       {
                                          int iXPManageTime = lpPlayer->self->ViewFlag(__FLAG_SCROLL_XP_MANAGEMENT);
                                          int iXPTimeStamp  = lpPlayer->self->ViewFlag(__FLAG_SCROLL_XP_TIMESTAMP);

                                          if(iXPManageTime !=0 && iXPTimeStamp !=0)
                                          {
                                             //reset the next activation... he already have a previous activation xp scroll
                                             lpPlayer->self->SetFlag(__FLAG_SCROLL_XP_MANAGEMENT,0);
                                             CString csText = _STR( 15154, lpPlayer->self->GetLang() );
                                             lpPlayer->self->SendSystemMessage(csText,CL_RED);
                                          }
                                          else if(iXPManageTime !=0)
                                          {
                                             //on dois activer le parchemin d'exp?rience...
                                             //le flag __FLAG_SCROLL_XP_MANAGEMENT contient en minutes le d?lais du parchemin et le % de gain...

                                             //icui on recupere le temps du parchemin et le pourcentage de gain...
                                             lpPlayer->dwSPXPScrollTimeTS = 0;
                                             int igain  = iXPManageTime/10000;
                                             int iDuree = iXPManageTime%10000;

                                             int iTimeDelaySec = iDuree*60;
                                             lpPlayer->self->SetFlag(__FLAG_SCROLL_XP_MANAGEMENT,0); //reset le flag de parchemin...
                                             lpPlayer->self->SetXPScrollBonnus(0);                   //reset les XP Bonnus...

                                             //set le time to stop du parchemin... apres ce set le scroll ets en marche...
                                             lpPlayer->self->SetFlag(__FLAG_SCROLL_XP_MULTIPLICATEUR,igain); //Set le %gain de ce scroll XP
                                             lpPlayer->self->SetFlag(__FLAG_SCROLL_XP_TIMESTAMP,iTimeDelaySec+100000);

                                             _LOG_SPECIAL_ITEMS
                                                LOG_ALWAYS,
                                                "Player %s (Account: %s) -->Start XP Scroll for %d min. at %d pourcent",
                                                (LPCTSTR)lpPlayer->self->GetTrueName(),
                                                (LPCTSTR)lpPlayer->GetFullAccountName(),
                                                iDuree,igain
                                                LOG_


                                                CString csText;
                                             csText.Format( _STR( 15155, IntlText::GetDefaultLng() ), igain,iDuree );
                                             lpPlayer->self->SendSystemMessage(csText,CL_ORANGE);
                                          }
                                          else if(iXPTimeStamp !=0)
                                          {
                                             char status = 0x00;
                                             if(iXPTimeStamp < 100000)
                                             {
                                                char buf[ 256 ];
                                                //get and reset the XP Bonus
                                                __int64 iXPBonnus = lpPlayer->self->GetXPScrollBonnus();

                                                //Reset all flags...
                                                lpPlayer->self->SetFlag(__FLAG_SCROLL_XP_TIMESTAMP ,0);
                                                lpPlayer->self->SetFlag(__FLAG_SCROLL_XP_MANAGEMENT,0);
                                                lpPlayer->self->SetXPScrollBonnus(0);

                                                //Set the player new XP...
                                                lpPlayer->self->SetXP(lpPlayer->self->GetXP()+iXPBonnus);

                                                _i64toa_s(iXPBonnus,buf,256,10);
                                                _LOG_SPECIAL_ITEMS
                                                   LOG_ALWAYS,
                                                   "Player %s (Account: %s) -->XP Scroll Ended for %s XP.",
                                                   (LPCTSTR)lpPlayer->self->GetTrueName(),
                                                   (LPCTSTR)lpPlayer->GetFullAccountName(),
                                                   buf
                                                   LOG_

                                                   //send message to show how many XP get with XP Scroll
                                                   CString csText;
                                                csText.Format( _STR( 15156, IntlText::GetDefaultLng() ), buf);
                                                lpPlayer->self->SendSystemMessage(csText,CL_ORANGE);
                                                status = 0x01;
                                                lpPlayer->self->InfoScrollXP(status,0);
                                             }
                                             else if( lpPlayer->dwSPXPScrollTime <= TFCMAIN::GetRound() )
                                             {
                                                lpPlayer->dwSPXPScrollTime = 15 SECONDS TDELAY;
                                                if(lpPlayer->dwSPXPScrollTimeTS ==0)
                                                   lpPlayer->self->SetFlag(__FLAG_SCROLL_XP_TIMESTAMP,iXPTimeStamp-15);
                                                else
                                                {
                                                   lpPlayer->self->SetFlag(__FLAG_SCROLL_XP_TIMESTAMP,iXPTimeStamp-(tCurTime-lpPlayer->dwSPXPScrollTimeTS));
                                                }
                                                lpPlayer->dwSPXPScrollTimeTS =  tCurTime;
                                             }
                                             else if(lpPlayer->dwSPXPScrollTimeD<= TFCMAIN::GetRound())
                                             {
                                                unsigned short ushNbrMin = 0;
                                                if((iXPTimeStamp-100015) > 0)
                                                   ushNbrMin = (iXPTimeStamp-100015)/60;
                                                ushNbrMin++;
                                                lpPlayer->self->InfoScrollXP(status,ushNbrMin);

                                                lpPlayer->dwSPXPScrollTimeD = 5 SECONDS TDELAY;
                                             }
                                          }
                                       }



                                       //NMNMNMNM manage Scroll OR...
                                       //////////////////////////////////////////////////////////////////////////////////////////////////////
                                       {
                                          int iORManageTime = lpPlayer->self->ViewFlag(__FLAG_SCROLL_OR_MANAGEMENT);
                                          int iORTimeStamp  = lpPlayer->self->ViewFlag(__FLAG_SCROLL_OR_TIMESTAMP);

                                          if(iORManageTime !=0 && iORTimeStamp !=0)
                                          {
                                             //reset the next activation... he already have a previous activation OR scroll
                                             lpPlayer->self->SetFlag(__FLAG_SCROLL_OR_MANAGEMENT,0);
                                             CString csText = _STR( 15252, lpPlayer->self->GetLang() );
                                             lpPlayer->self->SendSystemMessage(csText,CL_RED);
                                          }
                                          else if(iORManageTime !=0)
                                          {
                                             //on dois activer le parchemin de richesse...
                                             //le flag __FLAG_SCROLL_OR_MANAGEMENT contient en minutes le d?lais du parchemin et le % de gain...

                                             //icui on recupere le temps du parchemin et le pourcentage de gain...
                                             lpPlayer->dwSPORScrollTimeTS = 0;
                                             int igain  = iORManageTime/10000;
                                             int iDuree = iORManageTime%10000;

                                             int iTimeDelaySec = iDuree*60;
                                             lpPlayer->self->SetFlag(__FLAG_SCROLL_OR_MANAGEMENT,0); //reset le flag de parchemin...
                                             lpPlayer->self->SetORScrollBonnus(0);                   //reset les OR Bonnus...

                                             //set le time to stop du parchemin... apres ce set le scroll ets en marche...
                                             lpPlayer->self->SetFlag(__FLAG_SCROLL_OR_MULTIPLICATEUR,igain); //Set le %gain de ce scroll OR
                                             lpPlayer->self->SetFlag(__FLAG_SCROLL_OR_TIMESTAMP,iTimeDelaySec+100000);

                                             _LOG_SPECIAL_ITEMS
                                                LOG_ALWAYS,
                                                "Player %s (Account: %s) -->Start GOLD Scroll for %d min. at %d pourcent",
                                                (LPCTSTR)lpPlayer->self->GetTrueName(),
                                                (LPCTSTR)lpPlayer->GetFullAccountName(),
                                                iDuree,igain
                                                LOG_


                                                CString csText;
                                             csText.Format( _STR( 15253, IntlText::GetDefaultLng() ), igain,iDuree );
                                             lpPlayer->self->SendSystemMessage(csText,CL_ORANGE);
                                          }
                                          else if(iORTimeStamp !=0)
                                          {
                                             char status = 0x00;
                                             if(iORTimeStamp < 100000)
                                             {
                                                char buf[ 256 ];
                                                //get and reset the OR Bonus
                                                __int64 iORBonnus = lpPlayer->self->GetORScrollBonnus();

                                                //Reset all flags...
                                                lpPlayer->self->SetORScrollBonnus(0);
                                                lpPlayer->self->SetFlag(__FLAG_SCROLL_OR_MANAGEMENT,0);
                                                lpPlayer->self->SetFlag(__FLAG_SCROLL_OR_TIMESTAMP ,0);

                                                //Set the player new OR... EEEEEEEEEEEEEEEE
                                                lpPlayer->self->SetGold(lpPlayer->self->GetGold()+iORBonnus);

                                                _i64toa_s(iORBonnus,buf,256,10);
                                                _LOG_SPECIAL_ITEMS
                                                   LOG_ALWAYS,
                                                   "Player %s (Account: %s) -->OR Scroll Ended for %s GOLD.",
                                                   (LPCTSTR)lpPlayer->self->GetTrueName(),
                                                   (LPCTSTR)lpPlayer->GetFullAccountName(),
                                                   buf
                                                   LOG_

                                                   //send message to show how many GOLD get with OR Scroll
                                                   CString csText;
                                                csText.Format( _STR( 15254, IntlText::GetDefaultLng() ), buf );
                                                lpPlayer->self->SendSystemMessage(csText,CL_ORANGE);
                                                status = 0x01;
                                                lpPlayer->self->InfoScrollOR(status,0);
                                             }
                                             else if( lpPlayer->dwSPORScrollTime <= TFCMAIN::GetRound() )
                                             {
                                                lpPlayer->dwSPORScrollTime = 15 SECONDS TDELAY;
                                                if(lpPlayer->dwSPORScrollTimeTS ==0)
                                                   lpPlayer->self->SetFlag(__FLAG_SCROLL_OR_TIMESTAMP,iORTimeStamp-15);
                                                else
                                                {
                                                   lpPlayer->self->SetFlag(__FLAG_SCROLL_OR_TIMESTAMP,iORTimeStamp-(tCurTime-lpPlayer->dwSPORScrollTimeTS));
                                                }
                                                lpPlayer->dwSPORScrollTimeTS =  tCurTime;
                                             }
                                             else if(lpPlayer->dwSPORScrollTimeD<= TFCMAIN::GetRound())
                                             {
                                                unsigned short ushNbrMin = 0;
                                                if((iORTimeStamp-100015) > 0)
                                                   ushNbrMin = (iORTimeStamp-100015)/60;
                                                ushNbrMin++;
                                                lpPlayer->self->InfoScrollOR(status,ushNbrMin);

                                                lpPlayer->dwSPORScrollTimeD = 5 SECONDS TDELAY;
                                             }
                                          }
                                       }
                                    }

                                    //verifie if is cut for some time period if yes,
                                    //verifie if teh time is expired, if yes, we remove
                                    //the lost thing...

                                    time_t TimeMsTmp =  time(NULL);
                                    if( !lpPlayer->boCanTalk  && lpPlayer->uiTalkIndefinie >0)
                                    {
                                       if(TimeMsTmp > lpPlayer->uiTalkIndefinie)
                                       {
                                          lpPlayer->boCanTalk = TRUE;
                                          lpPlayer->uiTalkIndefinie = 0;
                                          lpPlayer->self->SendSystemMessage(_DEFAULT_STR( 15098));
                                       }
                                    }

                                    if( !lpPlayer->boCanShout  && lpPlayer->uiShoutIndefinie >0)
                                    {
                                       if(TimeMsTmp > lpPlayer->uiShoutIndefinie)
                                       {
                                          lpPlayer->boCanShout = TRUE;
                                          lpPlayer->uiShoutIndefinie = 0;
                                          lpPlayer->self->SendSystemMessage(_DEFAULT_STR( 15099));
                                       }
                                    }

                                    if( !lpPlayer->boCanPage  && lpPlayer->uiPageIndefinie >0)
                                    {
                                       if(TimeMsTmp > lpPlayer->uiPageIndefinie)
                                       {
                                          lpPlayer->boCanPage = TRUE;
                                          lpPlayer->uiPageIndefinie = 0;
                                          lpPlayer->self->SendSystemMessage(_DEFAULT_STR( 15100));
                                       }
                                    }

                                    //valide les sanctions...
                                    if( lpPlayer->GetSanctionLastTS() >0)
                                    {
                                       //ya eu une sanction..., si elle est expirer on reset le last sanction...
                                       if(TimeMsTmp > lpPlayer->GetSanctionLastTS())
                                       {
                                          lpPlayer->SetSanctionLastTS(0);
                                          int iSanctionA = lpPlayer->GetSanctionA();
                                          int iSanctionB = lpPlayer->GetSanctionB();
                                          if(iSanctionA >0)
                                          {
                                             iSanctionA--;
                                             lpPlayer->SetSanction(iSanctionA,iSanctionB);
                                             //si reste encore des sous sanction on reset le TS pour un next expire...
                                             if(iSanctionA >0)
                                             {
                                                lpPlayer->SetSanctionLastTS(TimeMsTmp+g_iDelayExpireSanction);
                                             }
                                          }
                                          lpPlayer->self->SendSystemMessage(_DEFAULT_STR( 15325));
                                       }
                                    }

                                    //valide le kill in jail system here now and not on thread...
                                    if( lpPlayer->dwInJailSystemTimeout <= TFCMAIN::GetRound() && theApp.m_dwManagePrisonExit > 0)
                                    {
                                       lpPlayer->dwInJailSystemTimeout = 60 MINUTES TDELAY;
                                       if(lpPlayer->self->ViewFlag(__FLAG_NMS_EN_PRISON) == 1)
                                       {
                                          CString csText;
                                          csText.Format("%s",_STR( 15145, lpPlayer->self->GetLang() ));

                                          TFCPacket sending;
                                          sending << (RQ_SIZE)RQ_ServerMessage;
                                          sending << (short)30;
                                          sending << (short)3;
                                          sending << csText;
                                          sending << (long) CL_RED;
                                          lpPlayer->self->SendPlayerMessage( sending );
                                          lpPlayer->dwKickoutTime = 6 SECONDS TDELAY;
                                       }
                                    }


                                    //valide la prison time si ye en prison...
                                    DWORD dwPrisonTS = lpPlayer->self->ViewFlag(__FLAG_PRISON_TIMESTAMP);
                                    if(dwPrisonTS >0)
                                    {
                                       //il doit sortir de la prison...
                                       if(TimeMsTmp > dwPrisonTS)
                                       {
                                          lpPlayer->self->SetFlag(__FLAG_PRISON_TIMESTAMP,0);
                                          lpPlayer->self->SetFlag(__FLAG_NMS_EN_PRISON,0);
                                          WorldPos wlPos = { 2967, 347, 0 };
                                          lpPlayer->self->Teleport( wlPos, 0 );

                                          lpPlayer->self->SendSystemMessage(_DEFAULT_STR( 15329));
                                       }
                                    }


                                    //RELOAD MANAGEMENT... need to confirm...

                                    //#if 0
                                    if(lpPlayer->in_AskReloadPlayer)
                                    {
                                       WorldPos ppos = lpPlayer->self->GetWL();
                                       WorldMap *wl = TFCMAIN::GetWorld(ppos.world);
                                       if(wl)
                                       {
                                          _LOG_WORLD
                                             LOG_MISC_1,
                                             "Player %s (Account %s) START RELOAD...",
                                             (LPCTSTR)lpPlayer->self->GetTrueName(),
                                             (LPCTSTR)lpPlayer->GetFullAccountName()
                                             LOG_
                                             // removes the unit associated with the PC from the world
                                             wl->remove_world_unit(ppos, lpPlayer->self->GetID());
                                          lpPlayer->self->DestroyMinion();


                                          
                                          //on detruit la phase RP de ce joueur si il en est le createur...
                                          if(lpPlayer->self->GetNMModeRPPhaseID() >=0)
                                          {
                                             BOOL bExist =  RPMaster::RpExist(lpPlayer->self->GetNMModeRPPhaseID());
                                             if(bExist)
                                             {
                                                BOOL bCreateur =  RPMaster::RpExistAndCreateur(lpPlayer->self->GetNMModeRPPhaseID(),lpPlayer->self->GetID());
                                                if(bCreateur)
                                                   RPMaster::RPInteractionTerminateLogOff(lpPlayer);
                                                else
                                                   RPMaster::RPInteractionQuitterLogOff(lpPlayer);
                                             }
                                          }

                                          if(lpPlayer->self->GetArenaID() >0)
                                          {
                                             if(lpPlayer->self->GetArenaType() == ARENE1_TYPE)
                                                Arena1Master::RemovePlayer(lpPlayer,false,lpPlayer->self->GetArenaID()-1,TRUE);
                                             else if(lpPlayer->self->GetArenaType() == ARENE2_TYPE)
                                                Arena2Master::RemovePlayer(lpPlayer,false,lpPlayer->self->GetArenaID()-1,TRUE);
                                          }
                                          
                                          if( lpPlayer->self->GetGroup() != NULL )
                                          {
                                             lpPlayer->self->GetGroup()->Dismiss( lpPlayer->self );// Leave it
                                          }

                                          

                                          lpPlayer->in_game = FALSE;
                                          lpPlayer->boPreInGame = FALSE;
                                          lpPlayer->self->SaveCharacter(FALSE,"Reload",FALSE); // Save lors d<un RELOAD, pas suppiorter pour la.... dois modifier sa
										            lpPlayer->boCanSave = FALSE;
                                          //lpPlayer->self->WaitForSaving();

                                          //destroy all CC...
                                          ChatterChannels &cChatter = CPlayerManager::GetChatter();
                                          cChatter.Remove( lpPlayer);

                                          lpPlayer->self->DestroyEffects();
                                          Sleep(100);
                                          lpPlayer->self->ClearAllSkillsAndSpells2();

                                          //ChatterChannels &cChatter = CPlayerManager::GetChatter();
                                          //cChatter.Remove(user->self->GetPlayer());
                                          lpPlayer->self->reset_character(true); // then reset it to avoid any misunderstanding.

                                          

                                          lpPlayer->dwSPXPScrollTime   = 0;
                                          lpPlayer->dwSPXPScrollTimeD  = 0;
                                          lpPlayer->dwSPXPScrollTimeTS = 0;
                                          lpPlayer->dwSPORScrollTime   = 0;
                                          lpPlayer->dwSPORScrollTimeD  = 0;
                                          lpPlayer->dwSPORScrollTimeTS = 0;


                                          Broadcast::BCObjectRemoved( ppos, _DEFAULT_RANGE_REMOVE,lpPlayer->self->GetID()); //quand player reload

                                          TFCPacket sending; 
                                          sending << (RQ_SIZE)RQ_ReturnToMenu;
                                          sending << (char)0;
                                          lpPlayer->self->SendPlayerMessage( sending );
                                       }
                                       lpPlayer->in_AskReloadPlayer = FALSE;
                                    }
                                    //#endif

                                    lpPlayer->self->ManageMinionMaintenance();
                                 }

                                 
                                 if(lpPlayer->self->GetNMModeRPPhaseID() >=0)
                                 {
                                    //valid que la phase RP est toujours active...
                                    BOOL bExist =  RPMaster::RpExist(lpPlayer->self->GetNMModeRPPhaseID());
                                    if(!bExist)
                                    {
                                       //rp existe plus on avertie et reset...
                                       CString strMsgTmp;
                                       strMsgTmp.Format(_STR(15400, lpPlayer->self->GetLang()));
                                       lpPlayer->self->SendSystemMessage(strMsgTmp,CL_ORANGE);
                                       lpPlayer->self->NMModeRPPhaseID(-1);
                                       lpPlayer->self->ResetNMModeRPPhaseCntTalk();
                                       lpPlayer->self->ResetNMModeRPPhaseCntNOTTalk();
                                       RPMaster::RPBroadcastInfo(lpPlayer);
                                    }
                                 }
                                 





                                 //DO NOT NEED to BE In game HERE....
                                 lpPlayer->self->VerifyTimers();

                                 lpPlayer->self->Unlock();

                                 // Must user auto-save?
                                 lpPlayer->QueryNextSave();

                              }
                              catch (...)
                              {
                                 lpPlayer->self->Unlock();
                                 lpPlayer->dwExitDecompte = 0;
                                 lpPlayer->dwReloadDecompte = 0xFFFF;

                                 FILE *pft = NULL;
                                 fopen_s(&pft,"c:\\__SvrException.txt","a+");
                                 fprintf(pft,"EXCEPTION--> Player maintenance-->Player:Character Lock Section\n");
                                 fclose(pft);

                                 _LOG_WORLD 
                                    LOG_MISC_1, 
                                    "EXCEPTION--> Player maintenance-->Player:Character Lock Section [%s] Kickout",lpPlayer?(LPCTSTR)lpPlayer->GetFullAccountName():"Player NULL"
                                    LOG_
                              }
                           }
                        }
                        lpPlayer->Unlock();
                     }
                     catch(...)
                     {
                        lpPlayer->Unlock();
                        lpPlayer->dwExitDecompte   = 0;
                        lpPlayer->dwReloadDecompte = 0xFFFF;

                        FILE *pft = NULL;
                        fopen_s(&pft,"c:\\__SvrException.txt","a+");
                        fprintf(pft,"EXCEPTION--> Player maintenance-->Player Lock Section\n");
                        fclose(pft);

                        _LOG_WORLD 
                           LOG_MISC_1, 
                           "EXCEPTION--> Player maintenance-->Player Lock Section [%s] Kickout",lpPlayer?(LPCTSTR)lpPlayer->GetFullAccountName():"Player NULL"
                           LOG_
                     }
                  }
               }
            }
         }
         catch(...)
         {
            FILE *pft = NULL;
            fopen_s(&pft,"c:\\__SvrException.txt","a+");
            fprintf(pft,"EXCEPTION--> Player maintenance-->Player management\n");
            fclose(pft);

            _LOG_WORLD 
               LOG_MISC_1, 
               "EXCEPTION--> Player maintenance-->Player management"
               LOG_
         }
         pMultiRSingleW->LeaveReader();


         // Lock the player resources (not resources allowed to be checked out)
         // while we verify in-game issues.
         pMultiRSingleW->EnterReader();
         try
         {
            // Scroll through all players currently in game for maintenance.
            for( i = 0; i < nBufferSize; i++ )
            {
               KEEP_ALIVE;
               lpPlayer = lpRegisteredUsers[ i ];

               // If player 
               if( lpPlayer == NULL )
               {
                  continue;
               }
               // Should always succeed since no ressources are checked out.
               if( lpPlayer->PickLock() )
               {
                  //si le pj a ete marquer pour delete lpPlayer->dwExitDecompte == 0
                  //on ne le process pas, il sera detruit a la next boucle simoplement...
                  if(lpPlayer->in_game && lpPlayer->dwExitDecompte >0 && lpPlayer->self->GetTarget() != NULL)
                  {
                     // If the character structure could be locked.
                     if( lpPlayer->self->PickLock() )
                     {
                        //Check if player cam play now...
                        if(!lpPlayer->CanPaidPlay())
                        {
                           //user credit is complete or server come on paid times...
                           if(lpPlayer->dwKickoutTime != 0xFFFFFFFF)
                           {
                              lpPlayer->dwKickoutTime = 10 SECONDS TDELAY;
                              lpPlayer->self->SendSystemMessage(_DEFAULT_STR( 15257));
                              //flush player out to game...
                           }

                        }

                        // Get the target.
                        Unit *target = lpPlayer->self->GetTarget();
                        // Get a lock on the target right now.
                        if( target && target->PickLock() )
                        {
                           // Verify auto-combat.
                           lpPlayer->self->ExecAutoCombat();

                           target->Unlock();
                        }
                        lpPlayer->self->Unlock();
                     }
                  }
                  lpPlayer->Unlock();
               }
            }
         }
         catch (...)
         {
            FILE *pft = NULL;
            fopen_s(&pft,"c:\\__SvrException.txt","a+");
            fprintf(pft,"EXCEPTION--> Player maintenance CATCH ExecAutoCombat while\n");
            fclose(pft);



            _LOG_DEBUG
               LOG_DEBUG_LVL1,
               "EXCEPTION--> Player maintenance CATCH ExecAutoCombat while"
               LOG_
         }
         pMultiRSingleW->LeaveReader();


         // Delete all players that were added for deletion
         pMultiRSingleW->EnterWriter(); //playermanager thread
         try
         {

            tlPlayersToDelete.ToHead();
            while( tlPlayersToDelete.QueryNext() )
            {
               Players *lpPlayer = tlPlayersToDelete.Object();
               if( lpPlayer->in_game && lpPlayer->self != NULL )
               {
                  RemoveTargetReferences( lpPlayer->self, true );
                  lpPlayer->self->Lock();
                  WorldMap *wlWorld = TFCMAIN::GetWorld( lpPlayer->self->GetWL().world );
                  if( wlWorld )
                  {
                     lpPlayer->self->DestroyMinion();
                     wlWorld->remove_world_unit( lpPlayer->self->GetWL(), lpPlayer->self->GetID() );
                     Broadcast::BCObjectRemoved( lpPlayer->self->GetWL(), _DEFAULT_RANGE_REMOVE, lpPlayer->self->GetID() );
                  }
                  lpPlayer->self->Unlock();
               }

               DeletePlayer( lpPlayer );
               tlPlayersToDelete.Remove();
            }

         }
         catch (...)
         {
            FILE *pft = NULL;
            fopen_s(&pft,"c:\\__SvrException.txt","a+");
            fprintf(pft,"EXCEPTION--> Player maintenance PART 2\n");
            fclose(pft);

            _LOG_DEBUG
               LOG_DEBUG_LVL1,
               "EXCEPTION--> Player maintenance PART 2"
               LOG_
         }
         pMultiRSingleW->LeaveWriter();
      }
      catch (...)
      {
         FILE *pft = NULL;
         fopen_s(&pft,"c:\\__SvrException.txt","a+");
         fprintf(pft,"EXCEPTION--> Player maintenance CATCH GLOBAL\n");
         fclose(pft);

         _LOG_DEBUG
            LOG_DEBUG_LVL1,
            "EXCEPTION--> Player maintenance CATCH GLOBAL"
            LOG_
      }

      cMaintenanceLock.Unlock();
      Sleep( 20 );
   }
   ENTER_TIMEOUT
      STOP_DEADLOCK_DETECTION

      return 0;
}

UINT CPlayerManager::SystemProcess( LPVOID lpData)
/******************************************************************************/
{    
   static int iiCntGlobal = 0;
   CAutoThreadMonitor tmMonitor("SystemProcess");
   _LOG_DEBUG
      LOG_DEBUG_LVL1,
      "SystemProcess Thread Id=%u",
      GetCurrentThreadId()
      LOG_

   CDeadlockDetector cDeadlockDetector;
   cDeadlockDetector.RegisterThread( hSystemProcessThread, "SystemProcess Thread", 120000  );            

   while( boMaintenance )
   { 
      KEEP_ALIVE

      //management general... a faire ici plutot que 50 thread separer...

      try
      {
         //if web server save the number of player into file
         if( m_dwWebServer <= TFCMAIN::GetRound() && theApp.dwEnableWebServer > 0)
         {
            CString strPathFile;
            strPathFile  = TFCMAIN::GetHomeDir();
            strPathFile +="WebServer\\";
            strPathFile += "WebServer.html";
   
            try
            {
               FILE *pfWebServer = NULL;
               fopen_s(&pfWebServer,strPathFile.GetBuffer(0),"wb");
               if(pfWebServer)
               {
                  fprintf_s(pfWebServer,"Currently %d user(s) online",CPlayerManager::GetUserCount());
                  fclose(pfWebServer);
               }
            }
            catch (...)
            {
            }
            m_dwWebServer = 30 SECONDS TDELAY;
         }
         

         if( m_dwManageTimedUnit <= TFCMAIN::GetRound() )
         {
            if(theApp.dwTimedUnitEnable)
            {
               for(int w=0;w<8;w++)
               {
                  WorldMap *wl = TFCMAIN::GetWorld(w);
                  if(wl)
                  {
                     wl->valid_link_expire();
                  }
               }
            }
            m_dwManageTimedUnit = 10 SECONDS TDELAY;
         }

         

         if( m_dwManageArena <= TFCMAIN::GetRound() )
         {
            Arena1Master::ManageArena();
            Arena2Master::ManageArena();
            NMS5YearsEvents::Manage5YearEvents();
            m_dwManageArena = 1 SECONDS TDELAY;
         }

         if( m_dwManageEvents <= TFCMAIN::GetRound() )
         {
            EventsMaster::ManageEvents();
            m_dwManageEvents = 5 SECONDS TDELAY;
         }


         if( m_dwManageInterRP <= TFCMAIN::GetRound() )
         {
            RPMaster::ManageInteractionRP();
            m_dwManageInterRP = 60 SECONDS TDELAY;
         }


         if( m_dwManageAH <= TFCMAIN::GetRound() )
         {
            AuctionMaster::ManageSoldList();
            AuctionMaster::ManageGiveList();
            m_dwManageAH = 30 SECONDS TDELAY;
         }

         if( m_dwManageAHSave <= TFCMAIN::GetRound() )
         {
            if(AuctionMaster::IsAuctionChanged())
               AuctionMaster::SaveAll(false);

            m_dwManageAHSave = 120 SECONDS TDELAY;
         }

         if( m_dwManageSaveGuildChest <= TFCMAIN::GetRound() )
         {
            if(GuildMaster::IsGuildChanged())
               GuildMaster::SaveAllGuilds(false); // Save guilds info too.

            GuildMaster::SaveAllGuildChest();
            m_dwManageSaveGuildChest = 120 SECONDS TDELAY;
         }
         if( m_dwManageGMMsg <= TFCMAIN::GetRound() )
         {
            GMMsgMaster::SaveAllGMMsg(); 
            m_dwManageGMMsg = 120 SECONDS TDELAY;
         }


         if( m_dwManageDuelClean <= TFCMAIN::GetRound() )
         {
            theApp.ManageDuelClean();
            m_dwManageDuelClean = 20 SECONDS TDELAY;
         }
         if( m_dwManageDuelDecompte <= TFCMAIN::GetRound() )
         {
            theApp.ManageDuelDecompte();
            m_dwManageDuelDecompte = 2 SECONDS TDELAY;
         }
         if( m_dwManagePVPUID <= TFCMAIN::GetRound() )
         {
            theApp.ManageThreadFctOnList();
            m_dwManagePVPUID = 5 SECONDS TDELAY;
         }
         

         if(m_dwManageUserServerTime<= TFCMAIN::GetRound())
         {
            TFCPacket sending;
            // This request sends the current time to the client,
            sending << (RQ_SIZE)RQ_GetTime;
            sending << (char )TFCTime::Second();
            sending << (char )TFCTime::Minute();
            sending << (char )TFCTime::Hour();
            sending << (char )TFCTime::Week();
            sending << (char )TFCTime::Day();
            sending << (char )TFCTime::Month();
            sending << (short)TFCTime::Year();
            WorldPos wlPos = { 0, 0, 0 };
            Broadcast::BCast( wlPos, 0, sending );

            m_dwManageUserServerTime  = 15 MINUTES TDELAY;
         }
               



         KEEP_ALIVE;

         //manage les request de AH...
         theApp.AHRequestAnalyseProcess();
         //Guild request...
         theApp.GuildRequestAnalyseProcess();
      }
      catch(...)
      {
         FILE *pft = NULL;
         fopen_s(&pft,"c:\\__SvrException.txt","a+");
         fprintf(pft,"EXCEPTION--> SystemProcess-->System management\n");
         fclose(pft);

         _LOG_WORLD 
            LOG_MISC_1, 
            "EXCEPTION--> SystemProcess-->System management"
            LOG_
      }
      Sleep( 1000 );
   }
   ENTER_TIMEOUT
   STOP_DEADLOCK_DETECTION
   return 0;
}


UINT CPlayerManager::AutoCastProcess( LPVOID lpData)
/******************************************************************************/
{    
   static int iiCntGlobal = 0;
   CAutoThreadMonitor tmMonitor("AutoCastProcess");
   _LOG_DEBUG
      LOG_DEBUG_LVL1,
      "AutoCastProcess Thread Id=%u",
      GetCurrentThreadId()
      LOG_

   CDeadlockDetector cDeadlockDetector;
   cDeadlockDetector.RegisterThread( hAutoCastProcessThread, "SystemProcess Thread", 120000  );            

   while( boMaintenance )
   { 
      KEEP_ALIVE
      try
      {
         //Loop sur la liste des event a recaster...
         TFCMAIN::ProcessCastSpellPos();
      }
      catch(...)
      {
         _LOG_WORLD 
            LOG_MISC_1, 
            "EXCEPTION--> AutoCastProcess-->System management"
            LOG_
      }
      Sleep( 100 );
   }
   ENTER_TIMEOUT
   STOP_DEADLOCK_DETECTION
   return 0;
}






/******************************************************************************/
// Grows the user buffer space.
void CPlayerManager::GrowBufferSpace( void )
{
   Players **lpNew = new Players *[ nBufferSize + GROW_BY ];

   // Copy all users from previous buffer to new one.
   int i;
   for( i = 0; i < nBufferSize; i++ )
      lpNew[ i ] = lpRegisteredUsers[ i ];

   // Fill the rest of the users to NULL.
   nBufferSize += GROW_BY;
   for(; i < nBufferSize; i++ )
   {
      lpNew[ i ] = NULL;
   }

   // Replace previous user buffer with new one.
   if (lpRegisteredUsers != NULL)
      delete lpRegisteredUsers;
   lpRegisteredUsers = lpNew;    
}

/******************************************************************************/
// Creates a new player entry.
// sockaddr_in sockAddr, // Address of remote player.
// CString csAccountName // Account name of remote player.
Players *CPlayerManager::CreatePlayer(sockaddr_in sockAddrO,sockaddr_in sockAddrI,CString csAccountName )
{
   if( boStop ) 
      return NULL;

   Players *lpPlayerCreated = NULL;   
   BOOL boFound;
   int i;

   // Lock player list (writer seul — ne pas prendre cMaintenanceLock : bloque sur maintenance longue).
   pMultiRSingleW->EnterWriter(); // Create Player
   

   // Try to find a duplicate entry for this address.
   boFound = FALSE;
   i = 0;
   while( i < nBufferSize && !boFound )
   {
      // If an non-empty spot was found.
      if( lpRegisteredUsers[ i ] != NULL )
      {
         if( SAME_IP( lpRegisteredUsers[ i ]->IPaddrO, sockAddrO ) )
         {
            if( lpRegisteredUsers[ i ]->IsDeleteFlags() )
            {
#ifndef _WIN32
               std::fprintf( stderr,
                             "[AUTH] CreatePlayer: purge ghost flagged @ IP %s:%u\n",
                             inet_ntoa( sockAddrO.sin_addr ),
                             static_cast<unsigned>( ntohs( sockAddrO.sin_port ) ) );
#endif
               DeletePlayer( lpRegisteredUsers[ i ], FALSE );
            }
            else
            {
               boFound = TRUE;
            }
         }
      }
      i++;
   }

   // If a duplicate entry wasn't found.
   if( !boFound )
   {
      // Try to find an empty user spot.
      i = 0;
      boFound = FALSE;    
      while( i < nBufferSize && !boFound )
      {
         // If an empty spot was found.
         if( lpRegisteredUsers[ i ] == NULL )
         { 
            // Reserve this space.
            boFound = TRUE;

            Players *lpPlayer = new Players;
            lpPlayer->ResetIdle();
            lpPlayer->IdleTime = 30 SECONDS TDELAY;
            lpPlayer->IPaddrO   = sockAddrO;
            lpPlayer->IPaddrI   = sockAddrI;
            lpPlayer->SetRadiusPortID( i );
            lpRegisteredUsers[ i ] = lpPlayer;

            lpPlayerCreated = lpPlayer;
         }
         i++;
      }

      // If no spot could be found
      if( !boFound )
      {
         GrowBufferSpace(); // Grow the user buffer space.

         Players *lpPlayer = new Players;
         lpPlayer->ResetIdle();
         lpPlayer->IdleTime = 30 SECONDS TDELAY;
         lpPlayer->IPaddrO   = sockAddrO;
         lpPlayer->IPaddrI   = sockAddrI;
         lpPlayer->SetRadiusPortID( i );
         lpRegisteredUsers[ i ] = lpPlayer;

         lpPlayerCreated = lpPlayer;
      }    

      _LOG_DEBUG
         LOG_DEBUG_HIGH,
         "Created new player slot!"
         LOG_

         nUserCount++;// One more user!
      if(lpPlayerCreated)
         lpPlayerCreated->Lock();// Lock the player resource.
   }

   // Unlock player list.
   pMultiRSingleW->LeaveWriter();

   return lpPlayerCreated;
}

/******************************************************************************/
// Remove a player from the player list.
//Players *lpPlayer, // The player entry to destroy.
//BOOL boIdle        // TRUE if we delete the player because he's idle.
void CPlayerManager::DeletePlayer(Players *lpPlayer,BOOL boIdle)
{
   if( boStop ) 
      return;
   
   // Find the player entry.
   BOOL boFound = FALSE;
   int i = 0;
   while( i < nBufferSize && !boFound )
   {
      // If the entry was found.
      if( lpRegisteredUsers[ i ] == lpPlayer )
      {
         // Free the entry.
         lpRegisteredUsers[ i ] = NULL;
         boFound = TRUE;
      }
      i++;
   }

   // Tell the chatter channels to delete this player.
   GetChatter().Remove( lpPlayer );

   // If unit hasn't been found, there's nothing to remove.
   if( !boFound )
   {
      return;
   }
  
   // One less user!
   nUserCount--;

   _LOG_DEBUG
      LOG_DEBUG_LVL1,
      "Logging off player %s memaddr( 0x%x ).",
      (LPCTSTR)lpPlayer->GetFullAccountName(),
      lpPlayer->self
      LOG_

   // If logoffs have not been stopped.
   PostQueuedCompletionStatus( hDeletionIo, 0, reinterpret_cast< std::uintptr_t >( lpPlayer ), NULL );    
}







/******************************************************************************/
//  Asynchronously deletes players.
UINT CPlayerManager::AsyncDeletePlayer( LPVOID lpNull) // NULL
{
   CAutoThreadMonitor tmMonitor("Async Delete Player");
   _LOG_DEBUG
      LOG_DEBUG_LVL1,
      "PlayerDeletion Thread Id=%u",
      GetCurrentThreadId()
      LOG_

   CDeadlockDetector cDeadlockDetector;
   cDeadlockDetector.RegisterThread( hDeleteThread, "Player Deletion Thread", 300000  );

   // While player manager is running
   while( boMaintenance )
   {
      DWORD dwBytes = 0;
      std::uintptr_t dwPlayerAddress = 0;
      LPOVERLAPPED lpOverlapped = NULL;

      ENTER_TIMEOUT;
      // Get next player to delete.
      if( GetQueuedCompletionStatus( hDeletionIo, &dwBytes, &dwPlayerAddress, &lpOverlapped, 60000 ) )
      {
         LEAVE_TIMEOUT;

         // Get player's object.
         Players *lpPlayer = reinterpret_cast< Players * >( dwPlayerAddress );

         if( lpPlayer == NULL || lpPlayer->self == NULL )
         {
            _LOG_DEBUG
               LOG_DEBUG_LVL1,
               "AsyncDeletePlayer: skip NULL player/self (0x%p).",
               static_cast<void *>( lpPlayer )
               LOG_
            continue;
         }

         if( !lpPlayer->in_game && !lpPlayer->registred && !lpPlayer->boPreInGame
             && !lpPlayer->boRerolling && lpPlayer->GetKeyCode() == 0 )
         {
            lpPlayer->self->SetPlayer( NULL );
            lpPlayer->self->reset_character();
            delete lpPlayer;
            continue;
         }

         if( !lpPlayer->in_game && lpPlayer->IsDeleteFlags() && lpPlayer->boPreInGame )
         {
#ifndef _WIN32
            std::fprintf( stderr,
                          "[AUTH] AsyncDeletePlayer fast preInGame compte='%s'\n",
                          (LPCTSTR)lpPlayer->GetAccount() );
#endif
            if( lpPlayer->self != NULL )
            {
               lpPlayer->self->SetPlayer( NULL );
               lpPlayer->self->reset_character();
            }
            delete lpPlayer;
            continue;
         }

         //on detruit la phase RP de ce joueur si il en est le createur...
         
         if(lpPlayer->self->GetNMModeRPPhaseID() >=0)
         {
            BOOL bExist =  RPMaster::RpExist(lpPlayer->self->GetNMModeRPPhaseID());
            if(bExist)
            {
               BOOL bCreateur =  RPMaster::RpExistAndCreateur(lpPlayer->self->GetNMModeRPPhaseID(),lpPlayer->self->GetID());
               if(bCreateur)
                  RPMaster::RPInteractionTerminateLogOff(lpPlayer);
               else
                  RPMaster::RPInteractionQuitterLogOff(lpPlayer);
            }
         }

         if(lpPlayer->self->GetArenaID() >0)
         {
            BOOL bPenalty = TRUE;
            if(lpPlayer->IsIdle())
               bPenalty = FALSE;

            if(lpPlayer->self->GetArenaType() == ARENE1_TYPE)
               Arena1Master::RemovePlayer(lpPlayer,false,lpPlayer->self->GetArenaID()-1,bPenalty);
            else if(lpPlayer->self->GetArenaType() == ARENE2_TYPE)
               Arena2Master::RemovePlayer(lpPlayer,false,lpPlayer->self->GetArenaID()-1,bPenalty);
         }
         
       
         
         lpPlayer->self->WaitForSaving(); // Wait pending saves.

         // Lock player usage ( any async functions performing on player must complete ).
         lpPlayer->UseDeathLock();
         lpPlayer->UseUnlock( __FILE__, __LINE__ );
         lpPlayer->Logoff();

         if( lpPlayer->IsIdle() )
         {
            _LOG_WORLD
               LOG_MISC_1,
               "User %s got kicked-out due to timeout.",
               (LPCTSTR)lpPlayer->GetFullAccountName()
               LOG_        
         }

         // Log player exit
         if( lpPlayer->registred )
         {
            CString csText;
            CString csOther;
            csText.Format( "User %s logged off. memaddr( 0x%x )", lpPlayer->GetFullAccountName(), lpPlayer->self );
            if( lpPlayer->in_game )
            {
               csOther.Format( "  Character %s was in game and exited from position ( %u, %u, %u ).",
                                 lpPlayer->self->GetTrueName(),lpPlayer->self->GetWL().X,lpPlayer->self->GetWL().Y,lpPlayer->self->GetWL().world);
               csText += csOther;
            }
            _LOG_WORLD
               LOG_MISC_1,
               (char *)(LPCTSTR)csText
               LOG_
         }

         // Lock User and perform Unit/World related removes.
         lpPlayer->self->Lock();

         if( lpPlayer->in_game || lpPlayer->boRerolling || lpPlayer->boPreInGame )
         {
            lpPlayer->self->SaveCharacter(FALSE,"AsyncDeletePlayer", FALSE ); //Save lors d'un delete
            lpPlayer->self->WaitForSaving();
         }



         // Player is now deleted, dereference it from the Unit.
         lpPlayer->self->SetPlayer( NULL );

         if( lpPlayer->in_game )
         {
            lpPlayer->ModifyAPlList( lpPlayer->self->GetTrueName(), (WORD)lpPlayer->self->GetAppearance(), (WORD)lpPlayer->self->GetLevel());
            // Save player.
            lpPlayer->SaveAccount();
         } 
         // Free Unit resource

		 //lpPlayer->Logoff();
         lpPlayer->self->Unlock();

         // Last of ALL, send unit to deathrow for destruction.
         LPDEATHROW lpMandate = new DEATHROW;
         lpMandate->lpuCondemned = lpPlayer->self;
         lpMandate->boDelete = TRUE;

         // Add player for deletion!
         tluDeathRow.Lock();
         tluDeathRow.AddToTail( lpMandate );
         tluDeathRow.Unlock();                            

         // Delete the player structure
         if (lpPlayer != NULL) 
         {
            delete lpPlayer;
            lpPlayer = NULL;
         }

      }
   }

   STOP_DEADLOCK_DETECTION

   return 0;
}


/******************************************************************************/
// Fetches a player resource.
Players *CPlayerManager::GetPlayerResourceFct( sockaddr_in sockAddr )// The address of the player to fetch.
{
   if( boStop ) 
      return NULL;

   Players *lpPlayer;

   // Find the player structure.
   int i;
   BOOL   boFound = FALSE;
   bool   playerLockedOrAbsent = false;

   // These two variables make sure that the player is dropped when it is locked
   // for too much time.
   const int  maxLoopCount = 5000;
   int    loopCount = 0;

   // Loop until we either find a player and successfully lock it,
   // or until the player isn't found in the list.    
   // NOTE: This is done to ensure thread safety when fetching player resources.

   while( !playerLockedOrAbsent && loopCount < maxLoopCount )
   {
      pMultiRSingleW->EnterReader();
      i = 0;
      boFound = FALSE;
      // Scroll through the array of registered users.
      while( i < nBufferSize && !boFound )
      {
         lpPlayer = lpRegisteredUsers[ i ];
         // If there is a player in this slot.
         if( lpPlayer != NULL )
         {
            // If the player was found (same IP address).
            if( SAME_IP( lpPlayer->IPaddrO, sockAddr ) )
            {
               // Break search, player was found.
               boFound = TRUE;
            }
         }
         i++;
      }

      // If the player wasn't found.
      if( !boFound )
      {
         // Player is NULL
         lpPlayer = NULL;
         // And break from main loop (player is absent).
         playerLockedOrAbsent = true;
      }
      else
      {
         // Otherwise try to lock the player.
         if( lpPlayer->PickLock() )
         {
            // If the player was successfully locked, 
            // break from main loop (player is locked)
            playerLockedOrAbsent = true;
         }
         // Otherwise, reloop and try to fetch the player again, until the player
         // is unlocked or its entry gets deleted from the list.
      }

      // Increase loop count, to make sure the function doesn't stall if the player
      // is locked for too much time.
      loopCount++;
      Sleep(0);
      pMultiRSingleW->LeaveReader();
   }
   

   // If the loop broke because the loopCount exceeded
   if( !playerLockedOrAbsent )
      lpPlayer = NULL;  // Set player to NULL, player was denied packet interpretation.

   return lpPlayer;
}

/******************************************************************************/
Players *CPlayerManager::GetPlayerResourceFctForMove( const sockaddr_in &sockAddr )
/******************************************************************************/
{
   if( boStop )
      return NULL;

   for( int attempt = 0; attempt < 32; ++attempt )
   {
      Players *lpPlayer = PeekPlayerAtEndpoint( sockAddr );
      if( lpPlayer == NULL || lpPlayer->self == NULL || lpPlayer->IsDeleteFlags() )
         return NULL;
      if( !lpPlayer->registred )
         return NULL;
      if( !lpPlayer->in_game )
         return NULL;
      if( lpPlayer->PickLock() )
         return lpPlayer;
      Sleep( 0 );
   }

   return NULL;
}

/******************************************************************************/
Players *CPlayerManager::GetPlayerResourceFctForSession( const sockaddr_in &sockAddr )
/******************************************************************************/
{
   if( boStop )
      return NULL;

   for( int attempt = 0; attempt < 128; ++attempt )
   {
      Players *lpPlayer = PeekPlayerAtEndpoint( sockAddr );
      if( lpPlayer == NULL )
      {
#ifndef _WIN32
         if( attempt == 0 )
         {
            std::fprintf( stderr,
                          "[Session] joueur inconnu IP %s:%u\n",
                          inet_ntoa( sockAddr.sin_addr ),
                          static_cast<unsigned>( ntohs( sockAddr.sin_port ) ) );
            std::fflush( stderr );
         }
#endif
         return NULL;
      }
      if( lpPlayer->self == NULL || lpPlayer->IsDeleteFlags() )
         return NULL;
      if( lpPlayer->PickLock() )
         return lpPlayer;
      Sleep( 0 );
   }

#ifndef _WIN32
   {
      Players *lpPeek = PeekPlayerAtEndpoint( sockAddr );
      std::fprintf( stderr,
                    "[Session] PickLock timeout compte='%s' preInGame=%d in_game=%d IP %s:%u\n",
                    lpPeek != NULL ? (LPCTSTR)lpPeek->GetAccount() : "?",
                    lpPeek != NULL && lpPeek->boPreInGame ? 1 : 0,
                    lpPeek != NULL && lpPeek->in_game ? 1 : 0,
                    inet_ntoa( sockAddr.sin_addr ),
                    static_cast<unsigned>( ntohs( sockAddr.sin_port ) ) );
      std::fflush( stderr );
   }
#endif
   return NULL;
}

BOOL CPlayerManager::IsPlayerResourceExist( sockaddr_in sockAddr )// The address of the player to fetch.
{
   if( boStop ) 
      return NULL;

   pMultiRSingleW->EnterReader();
   int i = 0;
   // Scroll through the array of registered users.
   while( i < nBufferSize)
   {
      // If there is a player in this slot.
      if( lpRegisteredUsers[ i ] != NULL )
      {
         // If the player was found (same IP address).
         if( SAME_IP( lpRegisteredUsers[ i ]->IPaddrO, sockAddr ) )
         {
            // Break search, player was found.
            pMultiRSingleW->LeaveReader();
            return TRUE; // is found
         }
      }
      i++;
   }

   pMultiRSingleW->LeaveReader();
   return FALSE; //player nout found
}

/******************************************************************************/
Players *CPlayerManager::PeekPlayerAtEndpoint( const sockaddr_in &sockAddr )
/******************************************************************************/
{
   if( boStop || lpRegisteredUsers == NULL || pMultiRSingleW == NULL )
      return NULL;

   Players *lpPlayer = NULL;
   pMultiRSingleW->EnterReader();
   for( int i = 0; i < nBufferSize; i++ )
   {
      if( lpRegisteredUsers[ i ] != NULL && SAME_IP( lpRegisteredUsers[ i ]->IPaddrO, sockAddr ) )
      {
         lpPlayer = lpRegisteredUsers[ i ];
         break;
      }
   }
   pMultiRSingleW->LeaveReader();
   return lpPlayer;
}

/******************************************************************************/
// Frees a player object.
void CPlayerManager::FreePlayerResourceFct( Players *lpPlayer) // The player to free
{
   if( boStop ) 
      return;

   if(lpPlayer)
      lpPlayer->Unlock();
}

/******************************************************************************/
// Frees a player object.
void CPlayerManager::FreePlayerResource( Players *lpPlayer) // The player to free
/******************************************************************************/
{
   if( boStop ) 
      return;

   if(lpPlayer)
      lpPlayer->Unlock();
}

/******************************************************************************/
// Returns the addresses of all in game players.
void CPlayerManager::GetGlobalBroadcastAddress(vector< sSockBooth > &vAddresses,SendPacketVisitor *packetVisitor,bool inGame)
{
   if( boStop ) 
      return;

   Players *lpPlayer;
   int i;    

   
   pMultiRSingleW->EnterReader();
   // Scroll the list of users.
   for( i = 0; i < nBufferSize; i++ )
   {
      lpPlayer = lpRegisteredUsers[ i ];
      if( lpPlayer != NULL )
      {
         // User must be ingame
         if( lpPlayer->in_game || !inGame )
         {
            bool addIP = true;
            if( packetVisitor != NULL )
            {
               addIP = packetVisitor->SendPacketTo( lpPlayer->self );
            }
            if( addIP )
            {
               // Create a new IP object.
               sSockBooth newSk;
               newSk.skI = lpPlayer->IPaddrI;
               newSk.skO = lpPlayer->IPaddrO;
               vAddresses.push_back( newSk );
            }
         }
      }
   }

   pMultiRSingleW->LeaveReader();
}

void CPlayerManager::GetGlobalBroadcastAddressLevelRange(vector< sSockBooth > &vAddresses,SendPacketVisitor *packetVisitor,bool inGame, int iLevelMin,int iLevelMax)
{
   if( boStop ) 
      return;

   Players *lpPlayer;
   int i;    


   pMultiRSingleW->EnterReader();
   // Scroll the list of users.
   for( i = 0; i < nBufferSize; i++ )
   {
      lpPlayer = lpRegisteredUsers[ i ];
      if( lpPlayer != NULL )
      {
         // User must be ingame
         if( (lpPlayer->in_game || !inGame )&& lpPlayer->self->GetLevel() >=iLevelMin && lpPlayer->self->GetLevel() <=iLevelMax)
         {
            bool addIP = true;
            if( packetVisitor != NULL )
            {
               addIP = packetVisitor->SendPacketTo( lpPlayer->self );
            }
            if( addIP )
            {
               // Create a new IP object.
               sSockBooth newSk;
               newSk.skI = lpPlayer->IPaddrI;
               newSk.skO = lpPlayer->IPaddrO;
               vAddresses.push_back( newSk );
            }
         }
      }
   }

   pMultiRSingleW->LeaveReader();
}
/******************************************************************************/
// Returns the addresses of all players local to a certain spot in a world.
void CPlayerManager::GetLocalBroadcastAddress (vector< sSockBooth > &vAddresses,WorldPos wlPos, int nRange, SendPacketVisitor *packetVisitor)
{
   if( boStop ) 
      return;

   Players *lpPlayer;
   int i;    
   WorldPos wlPlayerPos;

   pMultiRSingleW->EnterReader();

   // Scroll the list of users.
   for( i = 0; i < nBufferSize; i++ )
   {
      lpPlayer = lpRegisteredUsers[ i ];
      if( lpPlayer != NULL )
      {
         // User must be ingame
         if( lpPlayer->in_game )
         {
            // Fetch player position.
            wlPlayerPos = lpPlayer->self->GetWL();

            // If in same world.
            if( wlPlayerPos.world == wlPos.world )
            {

               // If within range.
               if( abs( wlPlayerPos.X - wlPos.X ) <= nRange && abs( wlPlayerPos.Y - wlPos.Y ) <= nRange )
               {

                  bool addIP = true;
                  if( packetVisitor != NULL )
                  {
                     addIP = packetVisitor->SendPacketTo( lpPlayer->self );
                  }
                  if( addIP )
                  {
                     // Create a new IP object.
                     sSockBooth newSk;
                     newSk.skI = lpPlayer->IPaddrI;
                     newSk.skO = lpPlayer->IPaddrO;
                     vAddresses.push_back( newSk );
                  }
               }
            }
         }
      }
   }

   pMultiRSingleW->LeaveReader();
}

/******************************************************************************/
// pack all player pos in game... and send to user
void CPlayerManager::PacketUserPos( Players *lpUser)
{   
   if( boStop ) 
      return;

   int i;    
   Players *lpPlayer;
   TFCPacket sending;

   sending << (RQ_SIZE)RQ_GetAllPlayerPos;

   TemplateList <Players> tlFoundUsers;

   
   pMultiRSingleW->EnterReader();
   for( i = 0; i < nBufferSize; i++ )
   {
      lpPlayer = lpRegisteredUsers[ i ];

      if( lpPlayer != NULL )
      {
         if( lpPlayer->in_game)
         {
            if(lpPlayer->self->ViewFlag(__FLAG_JUST_DO_IT) != 666)//oki
               tlFoundUsers.AddToTail( lpPlayer );
         }
      }    	
   }

   sending << (short)tlFoundUsers.NbObjects();
   tlFoundUsers.ToHead();
   while( tlFoundUsers.QueryNext() )
   {
      lpPlayer = tlFoundUsers.Object();
      WorldPos thisPos = lpPlayer->self->GetWL();

      //Send playername
      sending << lpPlayer->self->GetTrueName();
      //SendPlayer X,Y,W
      sending << (short)thisPos.X;
      sending << (short)thisPos.Y;
      sending << (short)thisPos.world;
   }
   pMultiRSingleW->LeaveReader();

   lpUser->self->SendPlayerMessage( sending );
}


/******************************************************************************/
// Makes sure player is unique (not twice). Will bump any same-player found.
BOOL CPlayerManager::VerifyPlayerUnique( Players *lpPlayer)
/******************************************************************************/
{
   if( boStop ) 
      return FALSE;

   BOOL boUnique = TRUE;

   Players *lpOtherPlayer;
   // Try to find another player.
   
   pMultiRSingleW->EnterReader();
   int i;
   for( i = 0; i < nBufferSize; i++ )
   {
      lpOtherPlayer = lpRegisteredUsers[ i ];

      if( lpOtherPlayer != NULL )
      {
         bool sameAccount = false;
         if( theApp.sAuth.m_StripRealmPartOfAccount )
         {
            sameAccount = _stricmp( (LPCTSTR)lpOtherPlayer->GetAccount(), (LPCTSTR)lpPlayer->GetAccount() ) == 0;
         }
         else
         {
            sameAccount = _stricmp( (LPCTSTR)lpOtherPlayer->GetFullAccountName(), (LPCTSTR)lpPlayer->GetFullAccountName() ) == 0;
         }

         // If player is already online but isn't on the same player resource.
         if( lpOtherPlayer != lpPlayer && sameAccount )
         {
            if( lpOtherPlayer->IsDeleteFlags() )
               continue;

            if( !lpOtherPlayer->in_game )
            {
               /* Session auth-only fantome (autre port UDP) : le nouveau login gagne. */
               continue;
            }

            /* in_game sur un autre port/socket : flag delete l'ancien, le nouveau login gagne. */
#ifndef _WIN32
            std::fprintf( stderr,
                          "[AUTH] VerifyPlayerUnique: flag delete ancien in_game compte '%s' (reconnect)\n",
                          (LPCTSTR)lpOtherPlayer->GetAccount() );
#endif
            lpOtherPlayer->SetDeletePlayerFlags();
            continue;
         }
      }    	
   }

   pMultiRSingleW->LeaveReader();

   return boUnique;
}

/******************************************************************************/
BOOL CPlayerManager::IsAccountRegistered( const CString &csAccountName )
/******************************************************************************/
{
   if( lpRegisteredUsers == NULL || pMultiRSingleW == NULL )
      return FALSE;

   BOOL boFound = FALSE;
   pMultiRSingleW->EnterReader();
   for( int i = 0; i < nBufferSize; i++ )
   {
      Players *lpPlayer = lpRegisteredUsers[ i ];
      if( lpPlayer != NULL
          && _stricmp( (LPCTSTR)lpPlayer->GetAccount(), (LPCTSTR)csAccountName ) == 0
          && lpPlayer->registred )
      {
         boFound = TRUE;
         break;
      }
   }
   pMultiRSingleW->LeaveReader();
   return boFound;
}

/******************************************************************************/
void CPlayerManager::FlagDeleteByAccount( const CString &csAccountName )
/******************************************************************************/
{
   if( lpRegisteredUsers == NULL || pMultiRSingleW == NULL )
      return;

   pMultiRSingleW->EnterReader();
   for( int i = 0; i < nBufferSize; i++ )
   {
      Players *lpPlayer = lpRegisteredUsers[ i ];
      if( lpPlayer != NULL )
      {
         if( _stricmp( (LPCTSTR)lpPlayer->GetAccount(), (LPCTSTR)csAccountName ) == 0 )
         {
#ifndef _WIN32
            std::fprintf( stderr, "[AUTH] purge session fantome compte '%s' (IP %s)\n",
                          (LPCTSTR)csAccountName, lpPlayer->GetIP() );
#endif
            lpPlayer->SetDeletePlayerFlags();
         }
      }
   }
   pMultiRSingleW->LeaveReader();
}

/******************************************************************************/
void CPlayerManager::ForceReconnectPurge( const sockaddr_in &sockAddr )
/******************************************************************************/
{
   if( boStop || lpRegisteredUsers == NULL || pMultiRSingleW == NULL )
      return;

   pMultiRSingleW->EnterWriter();

   Players *lpPlayer = NULL;
   for( int i = 0; i < nBufferSize; i++ )
   {
      if( lpRegisteredUsers[ i ] != NULL && SAME_IP( lpRegisteredUsers[ i ]->IPaddrO, sockAddr ) )
      {
         lpPlayer = lpRegisteredUsers[ i ];
         break;
      }
   }

   if( lpPlayer != NULL )
   {
      const CString csAccount = lpPlayer->GetAccount();
#ifndef _WIN32
      std::fprintf( stderr, "[AUTH] ForceReconnectPurge compte '%s' (reconnexion IP/port)\n",
                    (LPCTSTR)csAccount );
#endif
      RemoveTargetReferences( lpPlayer->self, true );
      if( lpPlayer->self != NULL )
      {
         lpPlayer->self->Lock();
         WorldMap *wlWorld = TFCMAIN::GetWorld( lpPlayer->self->GetWL().world );
         if( wlWorld != NULL && lpPlayer->in_game )
         {
            lpPlayer->self->DestroyMinion();
            wlWorld->remove_world_unit( lpPlayer->self->GetWL(), lpPlayer->self->GetID() );
            Broadcast::BCObjectRemoved( lpPlayer->self->GetWL(), _DEFAULT_RANGE_REMOVE, lpPlayer->self->GetID() );
         }
         lpPlayer->self->Unlock();
      }
      DeletePlayer( lpPlayer, FALSE );
      pMultiRSingleW->LeaveWriter();
      if( !csAccount.IsEmpty() )
         Players::AccountLoggonFailed( csAccount );
      return;
   }

   pMultiRSingleW->LeaveWriter();
}

/******************************************************************************/
void CPlayerManager::ForceReconnectPurgeByAccount( const CString &csAccountName )
/******************************************************************************/
{
   if( boStop || lpRegisteredUsers == NULL || pMultiRSingleW == NULL || csAccountName.IsEmpty() )
      return;

   cMaintenanceLock.Lock();
   pMultiRSingleW->EnterWriter();

   Players *lpPlayer = NULL;
   for( int i = 0; i < nBufferSize; i++ )
   {
      if( lpRegisteredUsers[ i ] != NULL
          && _stricmp( (LPCTSTR)lpRegisteredUsers[ i ]->GetAccount(), (LPCTSTR)csAccountName ) == 0 )
      {
         lpPlayer = lpRegisteredUsers[ i ];
         break;
      }
   }

   if( lpPlayer != NULL )
   {
#ifndef _WIN32
      std::fprintf( stderr, "[AUTH] ForceReconnectPurgeByAccount '%s' (IP %s in_game=%d)\n",
                    (LPCTSTR)csAccountName, lpPlayer->GetIP(), lpPlayer->in_game ? 1 : 0 );
#endif
      if( !lpPlayer->in_game )
      {
         DeletePlayer( lpPlayer, FALSE );
         pMultiRSingleW->LeaveWriter();
         cMaintenanceLock.Unlock();
         return;
      }
      RemoveTargetReferences( lpPlayer->self, true );
      if( lpPlayer->self != NULL )
      {
         lpPlayer->self->Lock();
         WorldMap *wlWorld = TFCMAIN::GetWorld( lpPlayer->self->GetWL().world );
         if( wlWorld != NULL && lpPlayer->in_game )
         {
            lpPlayer->self->DestroyMinion();
            wlWorld->remove_world_unit( lpPlayer->self->GetWL(), lpPlayer->self->GetID() );
            Broadcast::BCObjectRemoved( lpPlayer->self->GetWL(), _DEFAULT_RANGE_REMOVE, lpPlayer->self->GetID() );
         }
         lpPlayer->self->Unlock();
      }
      DeletePlayer( lpPlayer, FALSE );
   }

   pMultiRSingleW->LeaveWriter();
   cMaintenanceLock.Unlock();
}

/******************************************************************************/
void CPlayerManager::PurgeAccountSessionsExcept( const CString &csAccountName, const sockaddr_in &keepAddr )
/******************************************************************************/
{
   if( boStop || lpRegisteredUsers == NULL || pMultiRSingleW == NULL || csAccountName.IsEmpty() )
      return;

   Players::DeleteOnlineUserSync( csAccountName );

   for( ;; )
   {
      pMultiRSingleW->EnterWriter();

      Players *lpPlayer = NULL;
      for( int i = 0; i < nBufferSize; i++ )
      {
         if( lpRegisteredUsers[ i ] != NULL )
         {
         bool sameAccount = _stricmp( (LPCTSTR)lpRegisteredUsers[ i ]->GetAccount(),
                                      (LPCTSTR)csAccountName ) == 0;
         const bool sameEndpoint = SAME_IP( lpRegisteredUsers[ i ]->IPaddrO, keepAddr );
         const bool sameHost =
            lpRegisteredUsers[ i ]->IPaddrO.sin_addr.s_addr == keepAddr.sin_addr.s_addr;
         /* in_game (meme port ou autre) + fantomes auth sur autre port + in_game hote sans compte */
         if( ( sameAccount && ( lpRegisteredUsers[ i ]->in_game || !sameEndpoint ) )
             || ( sameHost && lpRegisteredUsers[ i ]->in_game && !sameEndpoint ) )
         {
            lpPlayer = lpRegisteredUsers[ i ];
            break;
         }
         }
      }

      if( lpPlayer == NULL )
      {
         pMultiRSingleW->LeaveWriter();
         break;
      }

#ifndef _WIN32
      std::fprintf( stderr,
                    "[AUTH] PurgeAccountSessionsExcept '%s' ancien IP %s:%u in_game=%d delete=%d\n",
                    (LPCTSTR)csAccountName,
                    lpPlayer->GetIP(),
                    static_cast<unsigned>( lpPlayer->GetPort() ),
                    lpPlayer->in_game ? 1 : 0,
                    lpPlayer->IsDeleteFlags() ? 1 : 0 );
#endif

      if( lpPlayer->in_game )
         lpPlayer->SetDeletePlayerFlags();
      lpPlayer->ClearLoadingCount();

      DeletePlayer( lpPlayer, FALSE );
      pMultiRSingleW->LeaveWriter();
   }
}


/******************************************************************************/
void CPlayerManager::DropFlaggedSessionAt( const sockaddr_in &sockAddr )
/******************************************************************************/
{
   if( boStop || lpRegisteredUsers == NULL || pMultiRSingleW == NULL )
      return;

   pMultiRSingleW->EnterWriter();
   for( int i = 0; i < nBufferSize; i++ )
   {
      Players *lpPlayer = lpRegisteredUsers[ i ];
      if( lpPlayer != NULL && SAME_IP( lpPlayer->IPaddrO, sockAddr ) && lpPlayer->IsDeleteFlags() )
      {
#ifndef _WIN32
         std::fprintf( stderr,
                       "[AUTH] DropFlaggedSessionAt IP %s:%u in_game=%d preInGame=%d\n",
                       inet_ntoa( sockAddr.sin_addr ),
                       static_cast<unsigned>( ntohs( sockAddr.sin_port ) ),
                       lpPlayer->in_game ? 1 : 0,
                       lpPlayer->boPreInGame ? 1 : 0 );
#endif
         if( !lpPlayer->in_game && lpPlayer->boPreInGame )
         {
            GetChatter().Remove( lpPlayer );
            lpRegisteredUsers[ i ] = NULL;
            nUserCount--;
#ifndef _WIN32
            std::fprintf( stderr,
                          "[AUTH] DropFlaggedSessionAt sync preInGame compte='%s'\n",
                          (LPCTSTR)lpPlayer->GetAccount() );
            std::fflush( stderr );
#endif
            delete lpPlayer;
         }
         else if( lpPlayer->IsDeleteFlags() || lpPlayer->in_game || lpPlayer->boPreInGame )
         {
            lpPlayer->ClearLoadingCount();
            DeletePlayer( lpPlayer, FALSE );
         }
         else
         {
            lpPlayer->ClearDeletePlayerFlags();
            lpPlayer->in_game = FALSE;
            lpPlayer->boPreInGame = FALSE;
            lpPlayer->registred = FALSE;
            lpPlayer->SetKeyCode( 0 );
         }
         break;
      }
   }
   pMultiRSingleW->LeaveWriter();
}


/******************************************************************************/
Players *CPlayerManager::ResetFlaggedSessionForReauth( const sockaddr_in &sockAddr )
/******************************************************************************/
{
   if( boStop || lpRegisteredUsers == NULL || pMultiRSingleW == NULL )
      return NULL;

   Players *lpReset = NULL;
   pMultiRSingleW->EnterWriter();
   for( int i = 0; i < nBufferSize; i++ )
   {
      Players *lpPlayer = lpRegisteredUsers[ i ];
      if( lpPlayer != NULL && SAME_IP( lpPlayer->IPaddrO, sockAddr )
          && ( lpPlayer->IsDeleteFlags() || lpPlayer->in_game ) )
      {
#ifndef _WIN32
         std::fprintf( stderr,
                       "[AUTH] ResetFlaggedSessionForReauth IP %s:%u (reuse slot)\n",
                       inet_ntoa( sockAddr.sin_addr ),
                       static_cast<unsigned>( ntohs( sockAddr.sin_port ) ) );
#endif
         lpPlayer->ClearDeletePlayerFlags();
         lpPlayer->in_game = FALSE;
         lpPlayer->boPreInGame = FALSE;
         lpPlayer->registred = FALSE;
         lpPlayer->SetKeyCode( 0 );
         lpPlayer->dwExitDecompte = 0xFFFF;
         lpReset = lpPlayer;
         break;
      }
   }
   pMultiRSingleW->LeaveWriter();
   return lpReset;
}


/******************************************************************************/
// Returns the player which has an in game character of a certain name.
Players *CPlayerManager::GetCharacterOld( CString csName) // The name of the character to search.
{
   if( boStop ) 
      return NULL;

   

   Players *lpPlayer;
   Players *lpFoundPlayer = NULL;
   CString csNameCpy = csName;

   int i = 0;

   pMultiRSingleW->EnterReader();
   while( i < nBufferSize && lpFoundPlayer == NULL )
   {
      lpPlayer = lpRegisteredUsers[ i ];

      if( lpPlayer != NULL )
      {
         if( lpPlayer->in_game  && lpPlayer->IdleChances <2)
         {
            CString csFoundName = lpPlayer->self->GetTrueName();
            csFoundName.MakeUpper();

            // If this player corresponds to the player
            csNameCpy.MakeUpper();            
            if( csFoundName.Find( csNameCpy ) == 0 )
            {
               // If no name was previously found or if the name exactly matches the specified name.
               if( lpFoundPlayer == NULL || csFoundName == csNameCpy )
                  lpFoundPlayer = lpPlayer;
            }
         }
      }
      i++;
   }
   pMultiRSingleW->LeaveReader();
   return lpFoundPlayer;
}

/******************************************************************************/
// Returns the player which has an in game character of a certain name.
Players *CPlayerManager::GetCharacterOldByID( DWORD dwID ) // The name of the character to search.
{
   if( boStop ) 
      return NULL;

   Players *lpPlayer;
   Players *lpFoundPlayer = NULL;

   pMultiRSingleW->EnterReader();
   int i = 0;
   while( i < nBufferSize && lpFoundPlayer == NULL )
   {
      lpPlayer = lpRegisteredUsers[ i ];

      if( lpPlayer != NULL )
      {
         if( lpPlayer->in_game  && lpPlayer->IdleChances <2)
         {
            CString csFoundName = lpPlayer->self->GetTrueName();
            csFoundName.MakeUpper();

            // If this player corresponds to the player
            if(lpPlayer->self->GetID() == dwID)
            {
               lpFoundPlayer = lpPlayer;
            }
         }
      }
      i++;
   }
   pMultiRSingleW->LeaveReader();
   return lpFoundPlayer;
}

/******************************************************************************/
// Returns the player which has an in game character of a certain name.
Players *CPlayerManager::GetCharacterRessource( CString csName) // The name of the character to search.
{
   if( boStop ) 
      return NULL;

   Players *lpPlayer;
   Players *lpFoundPlayer = NULL;
   Players *lpFoundPlayerPartial = NULL;
   // Find the player structure.
   int i;
   bool   playerLockedOrAbsent = false;
   CString csNameCpy = csName;

   // These two variables make sure that the player is dropped when it is locked
   // for too much time.
   const int  maxLoopCount = 500;
   int    loopCount = 0;

   // Loop until we either find a player and successfully lock it,
   // or until the player isn't found in the list.    
   // NOTE: This is done to ensure thread safety when fetching player resources.
   while( !playerLockedOrAbsent && loopCount < maxLoopCount )
   {
      // Lock the registered user array.
      pMultiRSingleW->EnterReader();

      //try full name
      i = 0;
      lpFoundPlayer        = NULL;
      lpFoundPlayerPartial = NULL;
      // Scroll through the array of registered users.
      while( i < nBufferSize && lpFoundPlayer == NULL )
      {
         lpPlayer = lpRegisteredUsers[ i ];
         // If there is a player in this slot.
         if( lpPlayer != NULL )
         {
            if( lpPlayer->in_game  && lpPlayer->IdleChances <2)
            {
               CString csFoundName = lpPlayer->self->GetTrueName();
               csFoundName.MakeUpper();

               // If this player corresponds to the player
               csNameCpy.MakeUpper();            
               if( csFoundName == csNameCpy)
               {
                  // If no name was previously found or if the name exactly matches the specified name.
                  if( lpFoundPlayer == NULL)
                     lpFoundPlayer = lpPlayer;
               }
               else if( csFoundName.Find( csNameCpy ) == 0 )
               {
                  if( lpFoundPlayerPartial == NULL)
                     lpFoundPlayerPartial = lpPlayer;
               }
            }
         }
         i++;
      }

      if(!lpFoundPlayer)
      {
         lpFoundPlayer = lpFoundPlayerPartial;
      }


      // If the player wasn't found.
      if( !lpFoundPlayer )
      {
         // Player is NULL
         lpPlayer = NULL;
         // And break from main loop (player is absent).
         playerLockedOrAbsent = true;
      }
      else
      {
         lpPlayer = lpFoundPlayer;
         // Otherwise try to lock the player.
         if( lpPlayer->PickLock() )
         {
            // If the player was successfully locked, 
            // break from main loop (player is locked)
            playerLockedOrAbsent = true;
         }
         // Otherwise, reloop and try to fetch the player again, until the player
         // is unlocked or its entry gets deleted from the list.
      }

      // Increase loop count, to make sure the function doesn't stall if the player
      // is locked for too much time.
      loopCount++;
      Sleep(0);
      pMultiRSingleW->LeaveReader();
   }

   // If the loop broke because the loopCount exceeded
   if( !playerLockedOrAbsent )
   {
      // Set player to NULL, player was denied packet interpretation.
      lpPlayer = NULL;        
   }
   return lpPlayer;
}


Players *CPlayerManager::GetCharacterRessourceByID(DWORD dwID)
{
   if( boStop ) 
      return NULL;

   Players *lpPlayer;
   // Find the player structure.
   int i;
   BOOL   boFound = FALSE;
   bool   playerLockedOrAbsent = false;

   // These two variables make sure that the player is dropped when it is locked
   // for too much time.
   const int  maxLoopCount = 500;
   int    loopCount = 0;

   // Loop until we either find a player and successfully lock it,
   // or until the player isn't found in the list.    
   // NOTE: This is done to ensure thread safety when fetching player resources.
   while( !playerLockedOrAbsent && loopCount < maxLoopCount )
   {
      // Lock the registered user array.
      pMultiRSingleW->EnterReader();

      i = 0;
      // Scroll through the array of registered users.
      while( i < nBufferSize && !boFound )
      {
         lpPlayer = lpRegisteredUsers[ i ];
         // If there is a player in this slot.
         if( lpPlayer != NULL )
         {
            if( lpPlayer->in_game && lpPlayer->IdleChances <2)
            {
               // If player names match.
               if(lpPlayer->self->GetID() == dwID)
               {
                  // Player was found.
                  boFound = TRUE;
               }
            }
         }
         i++;
      }

      // If the player wasn't found.
      if( !boFound )
      {
         // Player is NULL
         lpPlayer = NULL;
         // And break from main loop (player is absent).
         playerLockedOrAbsent = true;
      }
      else
      {
         // Otherwise try to lock the player.
         if( lpPlayer->PickLock() )
         {
            // If the player was successfully locked, 
            // break from main loop (player is locked)
            playerLockedOrAbsent = true;
         }
         // Otherwise, reloop and try to fetch the player again, until the player
         // is unlocked or its entry gets deleted from the list.
      }

      // Increase loop count, to make sure the function doesn't stall if the player
      // is locked for too much time.
      loopCount++;
      Sleep(0);
      pMultiRSingleW->LeaveReader();
   }

   // If the loop broke because the loopCount exceeded
   if( !playerLockedOrAbsent )
   {
      // Set player to NULL, player was denied packet interpretation.
      lpPlayer = NULL;        
   }
   return lpPlayer;
}

/******************************************************************************/
void CPlayerManager::SendNeerUnitMessage(int X, int Y, int W, CString strMsg)
{
   if( boStop) 
      return;

   int dwXMin = X-32;
   int dwXMax = X+32;
   int dwYMin = Y-32;
   int dwYMax = Y+32;
   int dwWorls= W;
   if(dwXMin < 0)
      dwXMin = 0;
   if(dwYMin < 0)
      dwYMin = 0;
   if(dwXMax > 3071)
      dwXMax = 3071;
   if(dwYMax > 3071)
      dwYMax = 3071;
   if(dwWorls <0)
      dwWorls = 0;
   if(dwWorls >7)
      dwWorls = 7;

   Players *lpPlayer;

   pMultiRSingleW->EnterReader();
   int i = 0;
   while( i < nBufferSize)
   {    
      lpPlayer = lpRegisteredUsers[ i ];

      if( lpPlayer != NULL )
      {            

         if( lpPlayer->in_game )
         {
            if(lpPlayer->self->GetWL().X > dwXMin && lpPlayer->self->GetWL().X < dwXMax && 
               lpPlayer->self->GetWL().Y > dwYMin && lpPlayer->self->GetWL().Y < dwYMax && lpPlayer->self->GetWL().world == dwWorls)
            {
               lpPlayer->self->SendSystemMessage(strMsg);
            }
         }
      }
      i++;
   }
   pMultiRSingleW->LeaveReader();
}


/******************************************************************************/
/*
int CPlayerManager::SendHLLListTo(Players *pPlayer)
{
   int iNbr = 0;
   if( boStop)   
      return iNbr;

   Players *lpPlayer;

   pMultiRSingleW->EnterReader();
   int i = 0;
   while( i < nBufferSize)
   {    
      lpPlayer = lpRegisteredUsers[ i ];

      if( lpPlayer != NULL )
      {            

         if( lpPlayer->in_game )
         {
            if (lpPlayer->self->ViewFlag(__FLAG_NMS_DEATH_HLL) == 1)
            {
               int dwVictimeKill   = lpPlayer->self->GetCrime();
               int dwNbrPrisonTime = lpPlayer->self->ViewFlag(__FLAG_NMS_DEATH_HLL_CNT);
               iNbr++;
               CString strName;
               strName.Format("%s (%d points de crime) (a ?t? %d fois en prison)",lpPlayer->self->GetTrueName(),dwVictimeKill,dwNbrPrisonTime);
               pPlayer->self->SendSystemMessage(strName.GetBuffer(0));
            }
         }
      }
      i++;
   }
   pMultiRSingleW->LeaveReader();
   return iNbr;
}
*/
/******************************************************************************/
void CPlayerManager::SendMessagetoAllGOD(CString strMsg)
{
   if( boStop )
      return;

   pMultiRSingleW->EnterReader();
   Players *lpPlayer;
   int i = 0;
   while( i < nBufferSize)
   {    
      lpPlayer = lpRegisteredUsers[ i ];

      if( lpPlayer != NULL )
      {            

         if( lpPlayer->in_game )
         {
            if(lpPlayer->IsGod())
               lpPlayer->self->SendInfoMessage(strMsg,0x0570D5);
         }
      }
      i++;
   }
   pMultiRSingleW->LeaveReader();
}

/******************************************************************************/
void CPlayerManager::SetPlayerGuildNameAndCC(CString strGN, CString strNGN)
{
   if( boStop)  
      return;

   pMultiRSingleW->EnterReader();

   Players *lpPlayer;
   int i = 0;
   while( i < nBufferSize)
   {    
      lpPlayer = lpRegisteredUsers[ i ];

      if( lpPlayer != NULL )
      {            

         if( lpPlayer->in_game )
         {
            if(lpPlayer->self->GetGuildName().CompareNoCase(strGN)==0)
            {

               lpPlayer->self->SetGuildName(strNGN.GetBuffer(0));
               ChatterChannels &cChatter = CPlayerManager::GetChatter();
               cChatter.AddCCPlayer( lpPlayer, strNGN.GetBuffer(0), "",false );
               cChatter.Remove( lpPlayer, strGN.GetBuffer(0));
               cChatter.SendRegisteredChannelList(lpPlayer);
            }
         }
      }
      i++;
   }

   pMultiRSingleW->LeaveReader();
   
}

/******************************************************************************/
void CPlayerManager::BroadcastGuildInfo(CString StrInfo, CString strGuildName,
                                        BOOL bResetGuildInfo,BOOL bUpdateGuildList,
                                        BOOL bUpdateGuildChest)
{
   if( boStop || strGuildName == "")    
      return;

   pMultiRSingleW->EnterReader();

   Players *lpPlayer;
   int i = 0;
   while( i < nBufferSize)
   {    
      lpPlayer = lpRegisteredUsers[ i ];

      if( lpPlayer != NULL )
      {            

         if( lpPlayer->in_game )
         {
            if(lpPlayer->self->GetGuildName().CompareNoCase(strGuildName) == 0)
            {
               if(bResetGuildInfo)
               {
                  lpPlayer->self->SetGuildName("");
                  lpPlayer->self->SetGuildTitle(0);
                  lpPlayer->self->SetGuildPermission(0);
                  lpPlayer->self->SetGuildNameInvited("",NULL);

                  ChatterChannels &cChatter = CPlayerManager::GetChatter();
                  cChatter.Remove(lpPlayer,strGuildName.GetBuffer(0));
                  cChatter.SendRegisteredChannelList(lpPlayer);

                  //on pourrais envoyer au client un guild info...

               }
               if(bUpdateGuildList)
                  lpPlayer->self->NMGetGuildList(0);
               if(bUpdateGuildChest)
               {
                  if(lpPlayer->self->GetIsGuildChesting())
                     lpPlayer->self->SendGuildChestPacket(FALSE);
               }

               if(StrInfo != "")
                  lpPlayer->self->SendInfoMessage(StrInfo.GetBuffer(0),0x0080FF);
            }
         }
      }
      i++;
   }

   pMultiRSingleW->LeaveReader();
}

/******************************************************************************/
int  CPlayerManager::GetNbrPlayerInGameOnThisGuild(CString strGuildName, int &iPlayerID)
/******************************************************************************/
{
   int iNbr = 0;
   if( boStop)
      return iNbr;

   pMultiRSingleW->EnterReader();

   Players *lpPlayer;
   int i = 0;
   while( i < nBufferSize)
   {    
      lpPlayer = lpRegisteredUsers[ i ];

      if( lpPlayer != NULL )
      {            

         if( lpPlayer->in_game )
         {
            if(lpPlayer->self->GetGuildName().CompareNoCase(strGuildName) == 0)
            {
               iNbr++;
               iPlayerID = lpPlayer->self->GetID();
            }
         }
      }
      i++;
   }

   pMultiRSingleW->LeaveReader();
   return iNbr;
}

/******************************************************************************/
void CPlayerManager::ForceSaveGuildMember(CString strGuildName,Character *pUser)
/******************************************************************************/
{
   if( boStop)  
      return;

   pMultiRSingleW->EnterReader();
   Players *lpPlayer;
   int i = 0;
   while( i < nBufferSize)
   {    
      lpPlayer = lpRegisteredUsers[ i ];

      if( lpPlayer != NULL )
      {            

         if( lpPlayer->in_game )
         {
            if(lpPlayer->self->GetGuildName().CompareNoCase(strGuildName)==0)
            {
               Character *pCurrentCh = lpPlayer->self;
               if(pCurrentCh != pUser)
               {
                  //reset son save time forcer un save...
                  //sa vas evider de dupliquer
                  pCurrentCh->GetPlayer()->ForceSave();//ForceSaveGuildMember (not used yet)
               }
            }
         }
      }
      i++;
   }

   pMultiRSingleW->LeaveReader();
}

//  Saves all players currently in game.
void CPlayerManager::SaveAll( void )
/******************************************************************************/
{
   Players *lpPlayer;

   pMultiRSingleW->EnterWriter(); // SaveAll
   int i;
   for( i = 0; i < nBufferSize; i++ )
   {
      lpPlayer = lpRegisteredUsers[ i ];

      if( lpPlayer != NULL )
      {
         //lpRegisteredUsers[ i ] = NULL; 
         // Logoff all users.
         lpPlayer->SetDeletePlayerFlags();
      }
   }

   pMultiRSingleW->LeaveWriter();
}

/******************************************************************************/
//  Returns the chatter channels instance.
ChatterChannels &CPlayerManager::GetChatter( void )
{
   if( lpChatter == NULL )
   {
      // Create the chatter channels.        
      lpChatter = new ChatterChannels();
   }
   return *lpChatter;
}

/******************************************************************************/
// Removes all references to a target unit from all players.
void CPlayerManager::RemoveTargetReferences( Unit *targetUnit , bool bResetContext)// The unit to dereference.
{

   pMultiRSingleW->EnterReader();
   //_old_cMaintenanceLock.Lock();

   int i;
   for( i = 0; i < nBufferSize; i++ )
   {
      Players *lpPlayer = lpRegisteredUsers[ i ];

      if( lpPlayer != NULL )
      {
         if( lpPlayer->self->GetTarget() == targetUnit )
         {
            lpPlayer->self->SetTarget( NULL );

            // If the player was robbing this player.
            if( lpPlayer->self->ViewFlag( __FLAG_ROBBING ) != 0 )
            {
               lpPlayer->self->RemoveFlag( __FLAG_ROBBING );

               TFCPacket sending;
               sending << (RQ_SIZE)RQ_DispellRob;
               lpPlayer->self->SendPlayerMessage( sending );
            }

         }
         if( lpPlayer->self->GetGameOpContext() == targetUnit  && bResetContext)
         {
            lpPlayer->self->SetGuildPermission(lpPlayer->self->GetGuildPermissionTmp());
            lpPlayer->self->SetGuildPermissionTmp(0);
            lpPlayer->self->SetGameOpContext( NULL );
            lpPlayer->self->SetGold( lpPlayer->self->GetGold() );
         }
         lpPlayer->self->RemoveReferenceTo( targetUnit );
      }
   }   

   //_old_cMaintenanceLock.Unlock();
   pMultiRSingleW->LeaveReader();
}

/******************************************************************************/
//  Refreshes the player list of all system channels.
void CPlayerManager::RefreshSystemChannels( void )
{
   pMultiRSingleW->EnterReader();
   int i;
   for( i = 0; i < nBufferSize; i++ )
   {
      Players *lpPlayer = lpRegisteredUsers[ i ];

      if( lpPlayer != NULL )
      {
         CPlayerManager::GetChatter().AddToSystemChannels( lpPlayer );
      }
   }
   pMultiRSingleW->LeaveReader();
}
