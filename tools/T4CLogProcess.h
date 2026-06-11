/******************************************************************************
Modify for vs2008 (01/05/2009)
/******************************************************************************/
//	API Class for logging procedures
//	Copyright 1997 Dialsoft Inc.
//	Benoit Martin

#ifndef __T4CLOGGER_PROCESS_H_
#define __T4CLOGGER_PROCESS_H_

#ifndef EXPORT
   #define EXPORT	extern "C" __declspec (dllexport)
#endif

/******************************************************************************/
// Error Codes
#define LOG_CRIT_ERRORS	(1 << 0) // 1
#define LOG_GEN_ERRORS	(1 << 1) // 2
#define LOG_DEBUG_LVL1	(1 << 2) // 4
#define LOG_DEBUG_LVL2	(1 << 3) // 8: doesn't have a timestamp and starts with a \TAB field ...
#define LOG_MEMORY		(1 << 4) // 16
#define LOG_WARNING		(1 << 5) // 32
#define LOG_MISC_1		(1 << 6) // 64
#define LOG_ALWAYS		(1 << 7) // 128: only there for ID, will always be logged, whatever lvl
#define LOG_SYSOP		   (1 << 8)
#define LOG_DEBUG_LVL3  (1 << 9)
#define LOG_DEBUG_LVL4  (1 << 10)
#define LOG_DEBUG_HIGH  (1 << 11)
#define LOG_ALL			0xFFFF
#define LOG_DEBUG_LIGHT ( LOG_CRIT_ERRORS | LOG_GEN_ERRORS | LOG_DEBUG_LVL1 | LOG_DEBUG_LVL2 | LOG_MEMORY | LOG_WARNING | LOG_MISC_1 | LOG_SYSOP )
#define LOG_DEBUG_HEAVY ( LOG_CRIT_ERRORS | LOG_GEN_ERRORS | LOG_DEBUG_LVL1 | LOG_DEBUG_LVL2 | LOG_MEMORY | LOG_WARNING | LOG_MISC_1 | LOG_SYSOP | LOG_DEBUG_LVL3 | LOG_DEBUG_LVL4 )

/******************************************************************************/
class __declspec(dllexport) CT4CLProcess
/******************************************************************************/
{
public:
   CT4CLProcess();
   ~CT4CLProcess();

   void SetLogLevels( WORD wLogLevels );
   WORD GetLogLevels( void ){return m_wSetLogLevels;};
   void SetLogFile(   std::string bsLogFile );

   void SyncLog ( WORD wLogLevel, const char *szText, ... );
private:
   void WriteLog( const char * fmt );   

   std::string m_StrLogFile;
   WORD        m_wSetLogLevels;

};

#endif //__T4CLOGGER_PROCESS_H_

