/******************************************************************************
Modify for vs2008 (01/05/2009)
/******************************************************************************/
#include "stdafx.h"
#include "T4CLogProcess.h"
#include "format.h"


namespace
{
   //  Specifies the string equivalent of a debug log level.
   inline const char *LogLvl2Str( WORD wLogLvl )
   {
      switch ( wLogLvl )
      {
         case LOG_CRIT_ERRORS:	return "CRITICAL";
         case LOG_GEN_ERRORS:	return "Error";
         case LOG_DEBUG_LVL1:	return "Debug1";
         case LOG_DEBUG_LVL2:	return "Debug2";
         case LOG_DEBUG_LVL3:	return "Debug3";
         case LOG_DEBUG_LVL4:	return "Debug4";
         case LOG_MEMORY:		return "Memory";
         case LOG_WARNING:		return "Warning";
         case LOG_ALWAYS:		return "Info";
         case LOG_MISC_1:		return "Misc1";
         case LOG_SYSOP:			return "Admin";
         case LOG_DEBUG_HIGH:	return "HeavyDbg";
         default:				return "Log";
      }
   }

   //  Formats a string
   inline std::string FormatLogString(WORD wLogLevels,const char *szText)
   {
      vir::TFormat format;

      std::string bsString;

      // If log levels isn't DEBUG_LVL2
      if ( wLogLevels != LOG_DEBUG_LVL2)
      {
         // Get the system time
         SYSTEMTIME sysTime; 
         GetLocalTime(&sysTime);

         // Start with the date and time 
         bsString = format( 
            "\r\n(%s),%d/%d/%d,%d:%02d:%02d,",
            LogLvl2Str( wLogLevels ), 
            sysTime.wMonth,
            sysTime.wDay,
            sysTime.wYear, 
            sysTime.wHour, 
            sysTime.wMinute,
            sysTime.wSecond
            );
      }
      else
      { 
         // Otherwise use a tab.
         bsString = "\r\n\t";
      }

      // Append the text to the string.
      bsString += szText;

      return bsString;
   }
}


CT4CLProcess::CT4CLProcess(void) : 
m_StrLogFile ( "default.log" ), 
m_wSetLogLevels( LOG_DEBUG_LIGHT )
{
   
}

CT4CLProcess::~CT4CLProcess(void)

{
}

// The new log levels.
void CT4CLProcess::SetLogLevels( WORD wNewLogLevels ) 
{
   m_wSetLogLevels = wNewLogLevels;
}

//  Sets the current log file.
void CT4CLProcess::SetLogFile( std::string bsNewFile )
{
   if( bsNewFile.size() < _MAX_PATH)
   {
      m_StrLogFile = bsNewFile;
   }
}

//  Synchronously logs the given string
void CT4CLProcess::SyncLog(WORD wLogLevels,const char *szText,...)
{
   if( wLogLevels & m_wSetLogLevels  || wLogLevels == LOG_ALWAYS )
   {
      char lpBuffer[ 4096 ];

      va_list argp;		
      int iSize;
      va_start( argp, szText );
      iSize = _vscprintf(szText, argp) + 1;
      vsprintf_s( lpBuffer, szText, argp );
      std::string bsLoggedString = FormatLogString( wLogLevels, lpBuffer );
      va_end( argp );

      // Directly write the string.
      WriteLog( bsLoggedString.c_str() );        
   }
}

void CT4CLProcess::WriteLog( const char *szString ) 
{
   FILE *fFile =NULL;
   fopen_s(&fFile, m_StrLogFile.c_str(), "ab");

   if( fFile == NULL )
      return;   

   fputs( szString, fFile );
   fclose( fFile );
}




