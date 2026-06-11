/******************************************************************************
Modify for vs2008 (30/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "TFC Server.h"
#include "DebugLogger.h"
#include "Format.h"

#ifdef _DEBUG
	#undef THIS_FILE
	static char THIS_FILE[]=__FILE__;
	#define new DEBUG_NEW
#endif

using namespace std;

/******************************************************************************/
DebugLogger::DebugLogger()
/******************************************************************************/
{
    // 100k strings default max log size
    dwMaxLogSize   = 1000;
}
/******************************************************************************/
DebugLogger::~DebugLogger()
/******************************************************************************/
{
}
/******************************************************************************/
//  Returns the sole DebugLogger instance.
DebugLogger &DebugLogger::GetInstance( void )
/******************************************************************************/
{
    static DebugLogger cInstance;
    
    return cInstance;
}
/******************************************************************************/
// Logs a string into persistent storage.
void DebugLogger::LogString
/******************************************************************************/
(
 std::string csString,  // The string to store.
 bool boLogTime         // true if the time and date should be appended to the string.
)
/******************************************************************************/
{
    // Lock
    CAutoLock( this );

    // If time-stamp was requested.
    if( boLogTime )
	{
        TFormat format;
        
        // Get time.
        time_t ttTime;
        time( &ttTime );

        // Get time in tm structure
        tm *tmTime = NULL;
		localtime_s(tmTime, &ttTime );

        // Append time formatted text to csString.
        csString = format( 
            "%u/%u/%u %02u:%02u:%02u ",
            tmTime->tm_mday,
            tmTime->tm_mon,
            tmTime->tm_year,
            tmTime->tm_hour,
            tmTime->tm_min,
            tmTime->tm_sec
        ) + csString;
    }
}
/******************************************************************************/
//  Suggestive function. Called to suggest the DebugLogger to flush its buffers.
void DebugLogger::Flush( void )
/******************************************************************************/
{
    // This function really flushes the log.
    CAutoLock( this );

    // If debug log wasn't defined
    if( CT4CLog::GetLog( LOG_DEBUG ) == NULL )
	{
        return;
    }

    CLogger *lpLogger = CT4CLog::GetLog( LOG_DEBUG );

    // If lvl 3 debugging is enabled.
    if( lpLogger->GetLogLevels() & LOG_DEBUG_LVL3 )
	{
        _LOG_DEBUG
            LOG_DEBUG_LVL2,
            "DebugLogger information stack: %u items.",
            qStrings.size()
        LOG_

        try
		{
            // Log each and every string.
            deque< CString >::iterator i;
            for( i = qStrings.begin(); i != qStrings.end(); i++ )
			{
                lpLogger->SyncLog( 
                    LOG_DEBUG_LVL2,
                    (char *)(LPCTSTR)( (*i) )
                );
            }
        }
		catch( ... )
		{
            try
			{
                _LOG_DEBUG
                    LOG_DEBUG_LVL2,
                    "Caught exception writing DebugLogger information."
                LOG_
            }
			catch( ... )
			{
            }
        }
    }
}
/******************************************************************************/
//  Adjusts the maximum quantity of strings contained in the logger at once.
void DebugLogger::AdjustMaxLogSize( unsigned long dwNewSize )// The new size.
/******************************************************************************/
{
    CAutoLock( this );

    // Set new sizes.
    dwMaxLogSize = dwNewSize;

    // Resize queue to step back size.
    qStrings.resize( GetStepBackSize(), "" );
}
/******************************************************************************/
//  Adjusts the step back size according to the provided maximum log size.
unsigned long DebugLogger::GetStepBackSize( void )
/******************************************************************************/
{
    // Step back size is 3/4 of maximum log size.
    return( ( dwMaxLogSize / 4 ) * 3 );
}