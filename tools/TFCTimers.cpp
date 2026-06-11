/******************************************************************************
Modify for vs2008 (30/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "TFC Server.h"
#include "TFCTimers.h"
#include "TFC_MAIN.h"

#ifdef _DEBUG
	#undef THIS_FILE
	static char THIS_FILE[]=__FILE__;
	#define new DEBUG_NEW
#endif

/******************************************************************************/
TFCTimers TFCTimerManager::tGlobalTimer;

/******************************************************************************/
TFCTimers::TFCTimers( void )
/******************************************************************************/
{
	lptlteTimerList = NULL;
}
/******************************************************************************/
 TFCTimers::~TFCTimers( void )
/******************************************************************************/
{
	if( lptlteTimerList != NULL )
	{
		delete lptlteTimerList;
		lptlteTimerList = NULL;
	}
}
/******************************************************************************/
// This function adds a timer.
//	dwTriggerTime:		Round at which the timer should trigger.
//	tcbFunc:			Timer callback function which is triggered upon timer release.
//	lpParams:			Parameters to pass to the callback function.
//	boRelative:			TRUE if dwTriggerTime is relative to the current server time.
void TFCTimers::AddTimer( WORD wTimerID, DWORD dwTriggerTime, TIMERCALLBACK tcbFunc, 
						 LPVOID lpParams, BOOL boRelativeTime)
/******************************************************************************/
{
	csThreadLock.Lock();
	
	if( lptlteTimerList == NULL )
	{
		lptlteTimerList = new TemplateList< TIMER_EVENT >;
	}

	// creates and inits a new timerevent structure
	LPTIMER_EVENT newTimer	= new TIMER_EVENT;
	if(boRelativeTime)
	{
		newTimer->dwTriggerTime = dwTriggerTime + TFCMAIN::GetRound();
	}
	else
	{
		newTimer->dwTriggerTime	= dwTriggerTime;
	}
	newTimer->wTimerID		= wTimerID;
	newTimer->lpParams		= lpParams;
	newTimer->tcbFunc		= tcbFunc;
	lptlteTimerList->AddToTail(newTimer);

	csThreadLock.Unlock();
}
/******************************************************************************/
// This function removes all timers that have lpParam as parameter
//	lpParam:	Address parameter to seek and destroy
// return:		TRUE if any timer has been removed
BOOL TFCTimers::RemoveTimersByParameter(LPVOID lpSearchParam)
/******************************************************************************/
{
	LPTIMER_EVENT lpteThisTimer;
	BOOL boDestroyed = FALSE;
	
   //NMNMNM_NEW: Add le lock avant le IF au cas ou un autre delete deleterais le dernier timers 
   //en meme temps que celui ci est dans la boucle...
   csThreadLock.Lock();
	if( lptlteTimerList != NULL )
	{
		lptlteTimerList->ToHead();
		while( lptlteTimerList->QueryNext() )
		{
			lpteThisTimer = lptlteTimerList->Object();
		
			// If timer has this parameter, destroy it
			if(lpteThisTimer->lpParams == lpSearchParam)
			{
				lptlteTimerList->DeleteAbsolute();
				boDestroyed = TRUE;
			}
		}
   }
   csThreadLock.Unlock();

	return boDestroyed;
}
/******************************************************************************/
// This function removes all timers that have tcbSearchFunc as callback function
//	tcbSearchFunc:	Destroy timers associated with this callback function
// return:			TRUE if any timer has been removed.
BOOL TFCTimers::RemoveTimersByCallback(TIMERCALLBACK tcbSearchCallback)
/******************************************************************************/
{
	LPTIMER_EVENT lpteThisTimer;
	BOOL boDestroyed = FALSE;
	
   //NMNMNM_NEW: Add le lock avant le IF au cas ou un autre delete deleterais le dernier timers 
   //en meme temps que celui ci est dans la boucle...
   csThreadLock.Lock();
	if( lptlteTimerList != NULL )
	{
   	lptlteTimerList->ToHead();
		while( lptlteTimerList->QueryNext() )
		{
			lpteThisTimer = lptlteTimerList->Object();
		
			// If timer has this parameter, destroy it
			if(lpteThisTimer->tcbFunc == tcbSearchCallback)
			{
				lptlteTimerList->DeleteAbsolute();
				boDestroyed = TRUE;
			}
		}
	}
   csThreadLock.Unlock();

	return boDestroyed;
}
/******************************************************************************/
// This function removes all timers that have both the callback function, and the parameter
//	tcbSearchFunc:	Destroy timers associated with this callback function
//	lpParam:		Address parameter to seek and destroy
// return:			TRUE if any timer has been removed./
BOOL TFCTimers::RemoveTimersByCallbackAndParameter(TIMERCALLBACK tcbSearchCallback,
												   LPVOID lpSearchParams)
/******************************************************************************/
{
	LPTIMER_EVENT lpteThisTimer;
	BOOL boDestroyed = FALSE;
	
   //NMNMNM_NEW: Add le lock avant le IF au cas ou un autre delete deleterais le dernier timers 
   //en meme temps que celui ci est dans la boucle...
   csThreadLock.Lock();
	if( lptlteTimerList != NULL )
	{
		lptlteTimerList->ToHead();
		while( lptlteTimerList->QueryNext() )
		{
			lpteThisTimer = lptlteTimerList->Object();
		
			// If timer has this parameter, destroy it
			if( lpteThisTimer->tcbFunc == tcbSearchCallback && lpteThisTimer->lpParams == lpSearchParams )
			{
				lptlteTimerList->DeleteAbsolute();
				boDestroyed = TRUE;
			}
		}
	}
   csThreadLock.Unlock();

	return boDestroyed;
}
/******************************************************************************/
BOOL TFCTimers::RemoveTimersByID( WORD wID )
/******************************************************************************/
{
   LPTIMER_EVENT lpteThisTimer;

   BOOL boDestroyed = FALSE;

   //NMNMNM_NEW: Add le lock avant le IF au cas ou un autre delete deleterais le dernier timers 
   //en meme temps que celui ci est dans la boucle...
   csThreadLock.Lock();
   if( lptlteTimerList != NULL )
   {
      lptlteTimerList->ToHead();
      while( lptlteTimerList->QueryNext() )
      {
         lpteThisTimer = lptlteTimerList->Object();

         // If timer has this parameter, destroy it
         if( lpteThisTimer->wTimerID == wID )
         {
            lptlteTimerList->DeleteAbsolute();
            boDestroyed = TRUE;
         }
      }
   }
   csThreadLock.Unlock();

   return boDestroyed;
}
/******************************************************************************/
// This function checks all the timers to see if any have popped. (internal)
void TFCTimers::VerifyTimers()
/******************************************************************************/
{
   //NMNMNM_NEW: Add le lock avant le IF au cas ou un autre delete deleterais le dernier timers 
   //en meme temps que celui ci est dans la boucle...

   LPTIMER_EVENT lpteThisTimer;
   TemplateList <TIMER_EVENT> tlteCallTimer;// Makes a copy of the timers to call on the stack (to avoid thread problems)

   csThreadLock.Lock();
	if( lptlteTimerList != NULL )
	{
		lptlteTimerList->ToHead();
		while( lptlteTimerList->QueryNext())
		{
			lpteThisTimer = lptlteTimerList->Object();

			if(lpteThisTimer->dwTriggerTime <= TFCMAIN::GetRound())
			{
				tlteCallTimer.AddToTail(lpteThisTimer);
				lptlteTimerList->Remove();
			}
		}
   }
   csThreadLock.Unlock();

   // Unlocks the object before calling the timers callback function to enable
   // them to create a timer themselves (without deadlocking everything up).
   tlteCallTimer.ToHead();
   while(tlteCallTimer.QueryNext())
   {
      lpteThisTimer = tlteCallTimer.Object();
      // Call the callback function
      lpteThisTimer->tcbFunc( lpteThisTimer->wTimerID, MSG_OnTimer, lpteThisTimer->lpParams, NULL );

      tlteCallTimer.DeleteAbsolute();
   }

   
}
/******************************************************************************/
// This functions destroys all the timers.
void TFCTimers::DestroyTimers()
/******************************************************************************/
{
   //NMNMNM_NEW: Add le lock avant le IF au cas ou un autre delete deleterais le dernier timers 
   //en meme temps que celui ci est dans la boucle...

   csThreadLock.Lock();
	if( lptlteTimerList != NULL )
	{
		lptlteTimerList->ToHead();
		while( lptlteTimerList->QueryNext() )
		{
			lptlteTimerList->DeleteAbsolute();
		}
	}
   csThreadLock.Unlock();
}
/******************************************************************************/
// Adds a global timer
void TFCTimerManager::AddTimer(
 WORD wTimerID,
 DWORD dwTriggerTime,		// Elapse time after which timer should be triggered
 TIMERCALLBACK tcbFunc,		// Timer callback function
 LPVOID lpParams,			// User parameters
 BOOL boRelativeTime		// TRUE if dwTriggerTime is an absolute 'round' time.
)
/******************************************************************************/
{
	tGlobalTimer.AddTimer( wTimerID, dwTriggerTime, tcbFunc, lpParams, boRelativeTime );
}
/******************************************************************************/
// Remove all timers using a certain parameter
BOOL TFCTimerManager::RemoveTimersByParameter(
 LPVOID lpSearchParams // The parameter to seek and destroy
)
/******************************************************************************/
{
	return tGlobalTimer.RemoveTimersByParameter( lpSearchParams );
}
/******************************************************************************/
// Remove all timers using a certain parameter
BOOL TFCTimerManager::RemoveTimersByCallback(
 TIMERCALLBACK tcbSearchFunc	// Callback function to seek and destroy.
)
/******************************************************************************/
{
	return tGlobalTimer.RemoveTimersByParameter( tcbSearchFunc );
}
/******************************************************************************/
BOOL TFCTimerManager::RemoveTimersByID( WORD wID )
/******************************************************************************/
{
    return tGlobalTimer.RemoveTimersByID( wID );
}
/******************************************************************************/
// Both above functions combined
BOOL TFCTimerManager::RemoveTimersByCallbackAndParameter(
 TIMERCALLBACK tcbSearchFunc,	// "
 LPVOID lpSearchParams			// "
)
/******************************************************************************/
{
	return tGlobalTimer.RemoveTimersByCallbackAndParameter( tcbSearchFunc, lpSearchParams );
}
/******************************************************************************/
// Verifies the global timers.
void TFCTimerManager::VerifyTimers( void )
/******************************************************************************/
{
	tGlobalTimer.VerifyTimers();
}
/******************************************************************************/
// Destroy all the global timers
void TFCTimerManager::DestroyTimers( void )
/******************************************************************************/
{
	tGlobalTimer.DestroyTimers();
}