/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "TFC Server.h"
#include "AsyncFuncQueue.h"
#include "DeadlockDetector.h"
#ifdef _WIN32
#ifdef _WIN32
#include <process.h>
#endif
#endif
#include "ThreadMonitor.h"
 
/******************************************************************************/
AsyncFuncQueue::AsyncFuncQueue()
{}

/******************************************************************************/
// Initializes the async func queue thread.
void AsyncFuncQueue::Initialize( void )
{	
   // If thread is uninitialized
   if( hThreadID == NULL )
   {
      static CLock initialLock;
      CAutoLock autoLock( &initialLock );

      // Create the IO Completion port associated with the thread, before it starts.
      hIoCompletion = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 1);

      // Start the thread.
      UINT threadId;
      hThreadID = (HANDLE)_beginthreadex( NULL, 0, AsyncFuncQueueThread, this, CREATE_SUSPENDED, &threadId ); //OK: thread qui envoie process des callback ASYNC
      SetThreadPriority(hThreadID,THREAD_PRIORITY_ABOVE_NORMAL);

      ResumeThread( hThreadID );
   }
}
/******************************************************************************/
// Adds a function to be called by the function queue thread.
void AsyncFuncQueue::Call( ASYNC_FUNC lpFunc, // The function to call
                           LPVOID lpData		// The data to pass as parameter
                         )
{
   // Initialize thread
   Initialize();

   ASYNC_FUNC_DATA *lpFuncData = new ASYNC_FUNC_DATA;
   lpFuncData->lpFunc = lpFunc;
   lpFuncData->lpData = lpData;

   PostQueuedCompletionStatus( hIoCompletion, 0, reinterpret_cast< std::uintptr_t >( lpFuncData ), NULL );
}
/******************************************************************************/
// The async func queue thread. Calls each queued function calls
// The AsyncFuncQueue instance.
UINT AsyncFuncQueue::AsyncFuncQueueThread(LPVOID lpVoid )
{
   CAutoThreadMonitor tmMonitor("AsyncFuncQueue");
   AsyncFuncQueue *pThis = reinterpret_cast< AsyncFuncQueue * >( lpVoid );
   pThis->AsyncFuncQueueFunc();
   return 0;
}

/******************************************************************************/
void AsyncFuncQueue::AsyncFuncQueueFunc()
{
   CoInitialize( NULL );
   _LOG_DEBUG
      LOG_DEBUG_LVL1,
      "AsyncFuncQueueThread Id=%u",
      GetCurrentThreadId()
      LOG_

   DWORD dwStopMessage = 0;
   std::uintptr_t dwlpFuncKey = 0;
   ASYNC_FUNC_DATA *lpFuncData;
   LPOVERLAPPED s_oDummy;

   CDeadlockDetector cDeadlockDetector;
   cDeadlockDetector.RegisterThread( hThreadID, "AsyncFuncQueue Thread", 300000  );

   // Read until its time to terminate the thread.
   while( dwStopMessage == 0 )
   {
      ENTER_TIMEOUT
      // Get the next queued function
      if( GetQueuedCompletionStatus( hIoCompletion, &dwStopMessage, &dwlpFuncKey, &s_oDummy, INFINITE ))
      {
         LEAVE_TIMEOUT
         // If this isn't a thread termination message
         if( dwStopMessage == 0 )
         {
            lpFuncData = reinterpret_cast< ASYNC_FUNC_DATA * >( dwlpFuncKey );

            // Call the function with its parameters
            _LOG_DEBUG
               LOG_DEBUG_HIGH,
               "\r\nEntering async function. func adress %p",
               reinterpret_cast< void * >( lpFuncData->lpFunc )
               LOG_

            lpFuncData->lpFunc( lpFuncData->lpData );

            _LOG_DEBUG
               LOG_DEBUG_HIGH,
               "\r\nLeaving async function"
               LOG_

            // Delete the async data object.
            if (lpFuncData != NULL)
            {	
               delete lpFuncData;
               lpFuncData = NULL;
            }
         }
      }            
   }

   CoUninitialize();
   STOP_DEADLOCK_DETECTION
}


AsyncFuncQueue *AsyncFuncQueue::GetMainQueue()
{
    static AsyncFuncQueue instance;
    return &instance;
}

