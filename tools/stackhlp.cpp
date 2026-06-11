/******************************************************************************
Modify for vs2008 (30/04/2009)
/******************************************************************************/
#include "stdafx.h"

/******************************************************************************/
// Returns all the previous call addresses from this thread.
DWORD WalkStack(
 DWORD bufferCallAddr[],  // The buffer to hold the addresses.
 DWORD bufferSize       // The size of the buffer that holds the addresses.
)
/******************************************************************************/
{
#if _M_IX86 && !( DISABLE_RUNTIME_STACK_WALK )
	DWORD rEbp;
	__asm{
		mov rEbp, ebp
	};
   
    DWORD currentLevel = 0;
    while( rEbp != 0 )
	{
        if( currentLevel > bufferSize )
		{
            return currentLevel;
        }
        
        // Store calling address from this ebp.
        bufferCallAddr[ currentLevel ] = *(DWORD *)( rEbp + 4 );
        
        // Move to next stack frame.
        rEbp = *(DWORD *)rEbp;

        currentLevel++;
    }
    return currentLevel;
#else
    return 0;
#endif
}
/******************************************************************************/
// Walks the stack given a known Ebp, allows walking any stack.
DWORD WalkStack(
 DWORD bufferCallAddr[], // The buffer to hold the addresses.
 DWORD bufferSize, // The size of the buffer that holds the addresses.
 DWORD rEbp         // The supplied Ebp register.
)
/******************************************************************************/
{
#if _M_IX86 && !( DISABLE_RUNTIME_STACK_WALK )
    DWORD currentLevel = 0;
    while( rEbp != 0 )
	{
        if( currentLevel > bufferSize )
		{
            return currentLevel;
        }
        
        // Store calling address from this ebp.
        bufferCallAddr[ currentLevel ] = *(DWORD *)( rEbp + 4 );
        
        // Move to next stack frame.
        rEbp = *(DWORD *)rEbp;

        currentLevel++;
    }
    return currentLevel;
#else
    return 0;
#endif
}