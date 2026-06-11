/******************************************************************************
Modify for vs2008 (31/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "TFC Server.h"
#include "Shutdown.h"
#include "TFCTimers.h"
#include "Broadcast.h"
#include "T4CLog.h"
#include "TimerListing.h"
#include "TFC_MAIN.h"
#include "RegKeyHandler.h"
#include "PlayerManager.h"
#include "DeadlockDetector.h"
#include "IntlText.h"
#include "PacketManager.h"
#include "MainConsole.h"
#include "GuildMaster.h"
#include "AuctionMaster.h"
#include "GMMsgMaster.h"

#ifdef _DEBUG
	#undef THIS_FILE
	static char THIS_FILE[]=__FILE__;
	#define new DEBUG_NEW
#endif

/******************************************************************************/
static CLock cLock;
extern CTFCServerApp theApp;

bool                     CShutdown::boShuttingDown = false;
vector< SHUTDOWN_FUNC >  CShutdown::vShutdownFunc;
SHUTDOWN                 CShutdown::sShutdown;
CShutdown                CShutdown::m_Instance;

/******************************************************************************/
// Shutdown structure to pass to ShutdownTimer
CShutdown::CShutdown()
/******************************************************************************/
{
}
/******************************************************************************/
CShutdown::~CShutdown()
/******************************************************************************/
{
}
/******************************************************************************/
//  Creates a shutdown handle.
bool CShutdown::CreateShutdown(
 DWORD dwTime,          // Time, in minutes, after which server should be shutdown.
 bool boEcho,           // TRUE if we should notify the users of the shutdown.
 //bool boNotifyWatchdog, // TRUE if the watchdog should wait 5 minutes before restarting the server.
 bool boOveride         // TRUE if we overide any current shutdown. 
)
/******************************************************************************/
{
    cLock.Lock();

    // If server isn't even started
    if( !theApp.serverStarted )
	{
        exit( SHUTDOWNCMD_EXIT );
    }

    // If we can't override the current shutdown.
    if( !boOveride )
	{
        MessageBeep( -1 );
        // If we're currently shutting down.
        if( boShuttingDown )
		{
            cLock.Unlock();
            return false;            
        }
    }   
    
    // Overwrite any current shutdown.
    TFCTimerManager::RemoveTimersByID( SERVER_SHUTDOWN_TIMER );        
    
    boShuttingDown = true;
    sShutdown.nMinutesLeft = dwTime;
    sShutdown.dwInitialTime = TFCMAIN::GetRound();
    sShutdown.boEcho = boEcho;
    //sShutdown.boNotifyWatchdog = boNotifyWatchdog;

    cLock.Unlock();
    return true;
}
/******************************************************************************/
// Adds a pre shutdown function.
void CShutdown::AddPreShutdownFunc(
 SHUTDOWN_FUNC lpShutdownFunc // The pre-shutdown function to call right before shutdown.
)
/******************************************************************************/
{
    cLock.Lock();
    vShutdownFunc.push_back( lpShutdownFunc );
    cLock.Unlock();
}
/******************************************************************************/
//  Starts the shutdown sequence.
void CShutdown::StartShutdown( void )
/******************************************************************************/
{
    // Add shutdown timer to trigger now.
    TFCTimerManager::AddTimer( SERVER_SHUTDOWN_TIMER, 0 SECOND TDELAY, ShutdownTimer, &sShutdown, FALSE );
}
/******************************************************************************/
// Cancels any current shutdown.
void CShutdown::CancelShutdown(
 bool boEcho    // TRUE if system should notify the users for the shutdown cancellation.
)
/******************************************************************************/
{
    cLock.Lock();
    // If a shutdown timer could be found.
    if( TFCTimerManager::RemoveTimersByID( SERVER_SHUTDOWN_TIMER ) )
	{
        if( boEcho )
		{
            // Broadcast shutdown cancel message.
            WorldPos wlPos = { 0,0,0 };
            Broadcast::BCServerMessage( wlPos, 0,
                _DEFAULT_STR( 44 ), NULL, CL_RED
            );
        }
    }    
    // Erase all shutdown functions.
    vShutdownFunc.erase( vShutdownFunc.begin(), vShutdownFunc.end() );
    
    // No more shutdown.
    boShuttingDown = false;    

    cLock.Unlock();
}
/******************************************************************************/
void WaitForAsyncSaveFunc( void );
/******************************************************************************/
// This static function is called each minutes before a shutdown starts.
// Upon final timer, it shutdowns the server.
void CShutdown::ShutdownTimer(
 WORD wTimerID,     // The timerID
 BYTE bCallReason,  // Reason of call 
 LPVOID lpData,     // Data passed at timer initialization
 LPVOID lpUserData  // NULL
)
/******************************************************************************/
{
   cLock.Lock();  

   WorldPos wlPos = {0,0,0};

   // If not currently in shut down, then we received an abnormal timer request.
   if( !boShuttingDown )
   {
      cLock.Unlock();
      return;
   }

   // If function was called because of timer expire.
   if( bCallReason == MSG_OnTimer )
   {
      // Shutdown immediatly.
      if( sShutdown.nMinutesLeft == -1 )
      {
         TFCPacket sending;
         sending << (RQ_SIZE)RQ_ExitGame;

         sockaddr_in sockDummyAddr;
         memset( &sockDummyAddr, 0, sizeof( sockaddr_in ) );

         // Send message.
         CPacketManager::SendPacket(sending,sockDummyAddr,sockDummyAddr,-1,wlPos,TRUE,NULL,false); //OK

         Sleep(5000);

         CPacketManager::StopComm();


         // Disable deadlock detection for ever.
         SUSPEND_DEADLOCK_DETECTION( INFINITE );

         GuildMaster::SaveAllGuilds(true);
         GuildMaster::SaveAllGuildChest();
         AuctionMaster::SaveAll(true);
         GMMsgMaster::SaveAllGMMsg();


         // Save players.
         CPlayerManager::SaveAll();


         Sleep( 20000 );


         _LOG_DEBUG
            LOG_DEBUG_LVL1,
            "Server is being shutdown. Waiting for character saving."
            LOG_

         // Wait until all character data has been written in the database.
         Character::WaitForAsyncSaveFuncBD();


         _LOG_DEBUG
            LOG_DEBUG_LVL1,
            "Finished saving characters."
            LOG_

         try
         {
            vector< SHUTDOWN_FUNC >::iterator i;
            for( i = vShutdownFunc.begin(); i != vShutdownFunc.end(); i++ )
            {
               (*i)( &sShutdown );
            }
         }
         catch(...)
         {
         }

         _LOG_DEBUG
            LOG_ALWAYS,
            "------- Server shutdown -------"
            LOG_

            try
         {
            // Close server.
            MainConsole::GetInstance().Terminate();
         }
         catch(...)
         {
         }
         // If everything else failed.
         exit( SHUTDOWNCMD_EXIT );
      }
      else 
         // Shutdown now! send last warning.    
         if( sShutdown.nMinutesLeft == 0 )
         {
            if( sShutdown.boEcho )
            {
               Broadcast::BCServerMessage( wlPos, 0,_DEFAULT_STR( 43 ), NULL, CL_RED);
            }

            // Next time this is called, shutdown.
            sShutdown.nMinutesLeft = -1;

            // Shutdown in 5 seconds.
            TFCTimerManager::AddTimer( SERVER_SHUTDOWN_TIMER, 5 SECONDS TDELAY, ShutdownTimer, &sShutdown, FALSE );
         }
         else
         {
            // If we should notify users.
            if( sShutdown.boEcho )
            {
               // If their is less then 5 minutes left or broadcast each 5 minutes.
               if( sShutdown.nMinutesLeft < 5 || ( sShutdown.nMinutesLeft % 5 ) == 0 )
               {
                  CString csMessage;
                  csMessage.Format( "%s %u %s.", _DEFAULT_STR( 41 ), sShutdown.nMinutesLeft, _DEFAULT_STR( 42 ) );

                  Broadcast::BCServerMessage( wlPos, 0,csMessage, NULL, CL_RED);
               }
            }
            // One minute less to play!
            sShutdown.nMinutesLeft--;

            // Setup next check in 1 minute
            TFCTimerManager::AddTimer( SERVER_SHUTDOWN_TIMER, 1 MINUTE TDELAY, ShutdownTimer, &sShutdown, FALSE );
         }
   }

   cLock.Unlock();
}
