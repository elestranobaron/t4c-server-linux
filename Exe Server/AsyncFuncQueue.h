#pragma once

#include "VDList.h"

/******************************************************************************/
typedef void ( *ASYNC_FUNC )( LPVOID lpData );

/******************************************************************************/
// This class handles queued function calls. Allows asynchronous calls to functions 
// without having to start a new thread each time.
class __declspec( dllexport ) AsyncFuncQueue
{
public:	
   void Destroy( void );

   void Call( ASYNC_FUNC lpFunc, LPVOID lpData );

   static AsyncFuncQueue *GetMainQueue();

private:
   AsyncFuncQueue();

   void Initialize( void );

   struct ASYNC_FUNC_DATA
   {
      ASYNC_FUNC lpFunc;
      LPVOID lpData;
   };

   HANDLE hIoCompletion;
   HANDLE hThreadID;

   static UINT CALLBACK AsyncFuncQueueThread( LPVOID lpVoid );
   void AsyncFuncQueueFunc();
};

