/******************************************************************************
Modify for vs2008 (30/04/2009)
Add Log Ah and fix Log Cheat by Nightmare (29/06/2009)
/******************************************************************************/
#include "stdafx.h"
#include "ODBCMage.h"
#include "TFC Server.h"
#include "T4CLog.h"
#include "RegKeyHandler.h"
#include "TFC Server.h"
#include "AutoConfig.h"
#include "TFC_MAIN.h"

#include <algorithm>
#include <string>

#ifdef _DEBUG
	#undef THIS_FILE
	static char THIS_FILE[]=__FILE__;
	#define new DEBUG_NEW
#endif


#define LOG_BUFFER_SIZE     (1024*8)  //8 megs

#define CREATE_LOG( __type, __key, __enablelog,  __default ) {\
    csFileName = csLogPath;\
    csFileName += regKey.GetProfileString( __key, __default );\
    if( regKey.GetProfileInt( __enablelog, 1 ) != 0 ){\
        TRACE( "\r\nLogging to file %s.", (LPCTSTR)csFileName );\
        cT4CLogs[ __type ].Create( (LPBYTE)(LPCTSTR)csFileName, wLevel );\
    }else{\
        TRACE( "\r\nNOT logging to file %s.", (LPCTSTR)csFileName );\
        cT4CLogs[ __type ].Create( NULL, 0 );\
    }\
}

#define IFLOG( __key, __level ) if( regKey.GetProfileInt( __key, 0 ) != 0 ){ wLevel |= __level; };

struct LogItemInfo {
	WORD wLogLevel;
	std::string strLog;
	CLogger *pLogProcess;
};

/******************************************************************************/
extern CTFCServerApp theApp;

CT4CLog *CT4CLog::cT4CLogs;
DWORD    CT4CLog::dwODBCErrorTimer; // Checks when the last error ocurred
WORD     CT4CLog::wSQLLogTypes; 
WORD     CT4CLog::wSQLLogLevel;
int      CT4CLog::ODBCErrorCounter; // Counts the number of errors

//thread unique
static BOOL      s_bLogThread = FALSE;
static HANDLE    s_hLogThread = NULL;
static HANDLE    s_hLogIO     = NULL;

static unsigned int WriteLogThread(LPVOID lpData);

static cODBCMage ODBCLogs;//Threader 100% manual Commit
static BOOL boIsInit = FALSE;

/******************************************************************************/
// Called when the registry is udpated.
void LogUpdate( void )
{
   RegKeyHandler regKey;

   if( !regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+LOGGING_KEY ) )
   { 
      return;
   }
}

/******************************************************************************/
// Inits all logs
void CT4CLog::InitLogs( void )
{
   RegKeyHandler regKey;
   CString csLogPath;
   CString csFileName;
   WORD wLevel = 0;

   cT4CLogs = new CT4CLog[ NB_LOG_TYPES ];

   
   UINT threadId;
   s_bLogThread     = TRUE;
   s_hLogIO         = CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL, 0, 1 );
   s_hLogThread     = (HANDLE)_beginthreadex( NULL, 0, WriteLogThread , NULL, 0, &threadId ); //OK: thread qui ecrit ASYNC les logs files

   if( !regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+LOGGING_KEY ) )
   {        
      wLevel = LOG_DEBUG_LIGHT;
   }

   CAutoConfig::AddRegUpdateCallback( LogUpdate );
   LogUpdate();

   csLogPath = theApp.sPaths.csLogPath;

   IFLOG( "LOG_CRITICAL_ERRORS",   LOG_CRIT_ERRORS );
   IFLOG( "LOG_GEN_ERRORS",        LOG_GEN_ERRORS );
   IFLOG( "LOG_WARNINGS",          LOG_WARNING );
   IFLOG( "LOG_MISC",              LOG_MISC_1 );
   IFLOG( "LOG_DEBUG_LVL1",        LOG_DEBUG_LVL1 | LOG_DEBUG_LVL2 );
   IFLOG( "LOG_DEBUG_LVL3",        LOG_DEBUG_LVL3 | LOG_DEBUG_LVL2 );
   IFLOG( "LOG_DEBUG_LVL4",        LOG_DEBUG_LVL4 | LOG_DEBUG_LVL2 );
   IFLOG( "LOG_DEBUG_HIGH",        LOG_DEBUG_HIGH | LOG_DEBUG_LVL2 );

   wLevel |= LOG_SYSOP;

   TRACE( "\r\nLog path=%s.", (LPCTSTR)csLogPath );

   CREATE_LOG( LOG_DEATH,  "DEATH_LOG",  "LOG_DEATH_LOG",  "Death.log" );
   CREATE_LOG( LOG_GAMEOP, "GAMEOP_LOG", "LOG_GAMEOP_LOG", "GameOp.log" );
   CREATE_LOG( LOG_PC,     "PC_LOG",     "LOG_PC_LOG",     "PCEdit.log" );

   CREATE_LOG( LOG_PAGE,   "PAGE_LOG",   "LOG_PAGE_LOG",   "Pages.log" );
   CREATE_LOG( LOG_SHOUTS, "SHOUTS_LOG", "LOG_SHOUTS_LOG", "Shouts.log" );
   CREATE_LOG( LOG_TEXT,   "TEXT_LOG",   "LOG_TEXT_LOG",   "OnlineText.log" );

   CREATE_LOG( LOG_ITEMS,  "ITEMS_LOG",  "LOG_ITEMS_LOG",  "Items.log" );
   CREATE_LOG( LOG_NPCS,   "NPCS_LOG",   "LOG_NPCS_LOG",   "NPCs.log" );
   CREATE_LOG( LOG_WORLD,  "WORLD_LOG",  "LOG_WORLD_LOG",  "World.log" );
   CREATE_LOG( LOG_DEBUG,  "DEBUG_LOG",  "LOG_DEBUG_LOG",  "Debug.log" );
   CREATE_LOG( LOG_CHEAT,  "CHEAT_LOG",  "LOG_CHEAT_LOG",  "Cheat.log" );
   CREATE_LOG( LOG_AH  ,   "AH_LOG",  "LOG_ITEMS_LOG",  "Ah.log" );
   CREATE_LOG( LOG_ACHAT_NMS  ,   "NMS_ACHAT_LOG",  "LOG_NPCS_LOG",  "_NMSAchat.log" );
   CREATE_LOG( LOG_SPECIAL_ITEMS  ,   "SPECIALITEMS_LOG",  "LOG_ITEMS_LOG",  "ItemsSpecial.log" );
   CREATE_LOG( LOG_SANCTION  ,   "SANCTION_LOG",  "LOG_SANCTION_LOG",  "!PJSanction.log" );
   CREATE_LOG( LOG_MONSTERS,   "MONSTERS_LOG",   "LOG_MONSTERS_LOG",   "Monsters.log" );
   CREATE_LOG( LOG_GUILDHIST,   "GUILDHIST_LOG",   "LOG_GUILDHIST_LOG",   "GuildHistorique.log" );
   CREATE_LOG( LOG_GUILDCHEST,  "GUILDCHEST_LOG",   "LOG_GUILDCHEST_LOG",   "GuildChest.log" );
   CREATE_LOG( LOG_BDEXTCH, "BDEXTCH_LOG",  "LOG_BDEXTCH_LOG",  "!BDExternalChange.log" );
   CREATE_LOG( LOG_PROFESSION,  "PROFESSION_LOG",   "LOG_PROFESSION_LOG",   "Profession.log" );
   CREATE_LOG( LOG_INTERRP,  "INTERRP_LOG",   "LOG_INTERRP_LOG",   "InteractionRP.log" );
   CREATE_LOG( LOG_ARENA,  "ARENA_LOG",   "LOG_ARENA_LOG",   "Arena.log" );
   CREATE_LOG( LOG_EVENTS,  "EVENTS_LOG",   "LOG_EVENTS_LOG",   "Events.log" );
   
   
   if (boIsInit == FALSE) 
   {

      // Starting SQL log levels and connections
      wSQLLogLevel = wSQLLogTypes = 0; // Initialize SQL log levels and types to all disabled

      // Determine which types will be logged to SQL
      if( regKey.GetProfileInt( "LOGSQL_DEATH_LOG", 0 ) != 0 ) { wSQLLogTypes |= 1 << LOG_DEATH; }
      if( regKey.GetProfileInt( "LOGSQL_GAMEOP_LOG", 0 ) != 0 ) { wSQLLogTypes |= 1 << LOG_GAMEOP; }
      if( regKey.GetProfileInt( "LOGSQL_PC_LOG", 0 ) != 0 ) { wSQLLogTypes |= 1 << LOG_PC; }
      if( regKey.GetProfileInt( "LOGSQL_PAGE_LOG", 0 ) != 0 ) { wSQLLogTypes |= 1 << LOG_PAGE; }
      if( regKey.GetProfileInt( "LOGSQL_SHOUTS_LOG", 0 ) != 0 ) { wSQLLogTypes |= 1 << LOG_SHOUTS; }
      if( regKey.GetProfileInt( "LOGSQL_TEXT_LOG", 0 ) != 0 ) { wSQLLogTypes |= 1 << LOG_TEXT; }
      if( regKey.GetProfileInt( "LOGSQL_ITEMS_LOG", 0 ) != 0 ) { wSQLLogTypes |= 1 << LOG_ITEMS; }
      if( regKey.GetProfileInt( "LOGSQL_NPCS_LOG", 0 ) != 0 ) { wSQLLogTypes |= 1 << LOG_NPCS; }
      if( regKey.GetProfileInt( "LOGSQL_WORLD_LOG", 0 ) != 0 ) { wSQLLogTypes |= 1 << LOG_WORLD; }

      // Determine which levels will be logged to SQL
      wSQLLogLevel |= LOG_SYSOP; // Sysop messages should always get logged!
      if( regKey.GetProfileInt( "LOGSQL_CRITICAL_ERRORS", 0 ) != 0 ) { wSQLLogLevel |= LOG_CRIT_ERRORS; }
      if( regKey.GetProfileInt( "LOGSQL_GEN_ERRORS",		 0 ) != 0 ) { wSQLLogLevel |= LOG_GEN_ERRORS; }
      if( regKey.GetProfileInt( "LOGSQL_WARNINGS",		 0 ) != 0 ) { wSQLLogLevel |= LOG_WARNING; }
      if( regKey.GetProfileInt( "LOGSQL_MISC",			 0 ) != 0 ) { wSQLLogLevel |= LOG_MISC_1; }
      if( regKey.GetProfileInt( "LOGSQL_DEBUG_LVL1",		 0 ) != 0 ) { wSQLLogLevel |= LOG_DEBUG_LVL1 | LOG_DEBUG_LVL2; }
      if( regKey.GetProfileInt( "LOGSQL_DEBUG_LVL3",		 0 ) != 0 ) { wSQLLogLevel |= LOG_DEBUG_LVL3 | LOG_DEBUG_LVL2; }
      if( regKey.GetProfileInt( "LOGSQL_DEBUG_LVL4",		 0 ) != 0 ) { wSQLLogLevel |= LOG_DEBUG_LVL4 | LOG_DEBUG_LVL2; }
      if( regKey.GetProfileInt( "LOGSQL_DEBUG_HIGH",		 0 ) != 0 ) { wSQLLogLevel |= LOG_DEBUG_HIGH | LOG_DEBUG_LVL2; }

      ODBCLogs.Connect ( USERS_DSN, USERS_USER, USERS_PWD );
      ODBCLogs.ConnectOption( SQL_AUTOCOMMIT, SQL_AUTOCOMMIT_OFF );
   }

   boIsInit = TRUE;
}

void CT4CLog::StopLogs()
{
   s_bLogThread = FALSE;
   if(s_hLogIO)
      CancelIo( s_hLogIO);

   if(s_hLogThread)
   {
      if( WaitForSingleObject( s_hLogThread, 500 ) == WAIT_TIMEOUT ) 
         TerminateThread( s_hLogThread, 1 ); 
      s_hLogThread = NULL;
   }

   ODBCLogs.Disconnect();

   if(cT4CLogs)
      delete []cT4CLogs;
   cT4CLogs = NULL;
}

static unsigned int WriteLogThread(LPVOID lpData)
{
 
   while( s_bLogThread )
   {

      DWORD dwFoo = 0;
      std::uintptr_t dwPacketAddr = 0;
      LPOVERLAPPED lpOverlapped = NULL;

      LogItemInfo *pNewLog = NULL;
      BOOL bRet = FALSE;
      bRet = GetQueuedCompletionStatus( s_hLogIO, &dwFoo, &dwPacketAddr, &lpOverlapped, 1000 );
      if(bRet)
      {
         pNewLog = reinterpret_cast< LogItemInfo* >( dwPacketAddr );

         if(pNewLog && pNewLog->pLogProcess)
         {
            pNewLog->pLogProcess->SyncLog(pNewLog->wLogLevel,pNewLog->strLog.c_str());
            delete pNewLog;
         }
         pNewLog = NULL;
      }
   }

   return 0;
}

/******************************************************************************/
CT4CLog::CT4CLog( void )
{   
   lpbBuffer = new BYTE[ LOG_BUFFER_SIZE + 1 ];
   boLog = FALSE;
}

/******************************************************************************/
void  CT4CLog::Destroy( void )
{

}

/******************************************************************************/
// Initializes a specific logger object.
//LPBYTE lpszFileName, // The log file name
//WORD   wLogLevels    // The log levels.
void CT4CLog::Create(LPBYTE lpszFileName,WORD   wLogLevels)
{
   if( lpszFileName != NULL )
   {
      logLog.SetLogFile( (char *)lpszFileName );
      logLog.SetLogLevels( wLogLevels );

      boLog = TRUE;
   }
}

/******************************************************************************/
// Returns a logger object.
CLogger *CT4CLog::GetLog(DWORD dwType)
{
   if( cT4CLogs == NULL || dwType >= NB_LOG_TYPES )
   {
      return NULL;
   }
   if( cT4CLogs[ dwType ].boLog )
   {
      return &cT4CLogs[ dwType ].logLog;
   }
   return NULL;
}

/******************************************************************************/
// This outputs strictly debug strings.
// WORD wLogLevels, // The log levels.
// const char *szText,     // The text to log.
//...                     // The format specifiers.
void CT4CLog::DebugLog(WORD wLogLevels,const char *szText,...)
{
   // Get the debug logger object.
   CLogger *logger = GetLog( LOG_DEBUG );
   if( logger == NULL )
   {
      return;
   }

   // If the logging level is not accepted.
   if( !( wLogLevels & logger->GetLogLevels() || wLogLevels == LOG_ALWAYS ) )
   {
      return;
   }

   static bool error = false;
   char lpBuffer[ 4096 ];

   va_list argp;		
   int iSize;
   va_start( argp, szText );
   iSize = _vscprintf(szText, argp) + 1;
   vsprintf_s( lpBuffer, iSize, szText, argp );
   va_end( argp );


   if(strstr(lpBuffer,"zzqqwxq")!= NULL)
   {
      return;
   }

   LogItemInfo *pNewEntry = new LogItemInfo();
   pNewEntry->wLogLevel   = wLogLevels;
   pNewEntry->strLog      = lpBuffer;
   pNewEntry->pLogProcess = logger;
   PostQueuedCompletionStatus( s_hLogIO, 0, reinterpret_cast< std::uintptr_t >( pNewEntry ), NULL );
}


/******************************************************************************/
// call both functions: TXT and SQL
//DWORD       dwType,      // The log type
//WORD        wLogLevels, // The log levels.
//const char *szText,     // The text to log.
//...                     // The format specifiers.
void CT4CLog::SaveToLog(DWORD dwType,WORD wLogLevels,const char *szText, ...)
{
   try
   {
      DWORD dwTypeLog = dwType;
      if( szText == NULL || szText[0] == '\0' )
         return;
      if(strlen(szText) >=4096)
         return;

      char lpBuffer[4096];
      lpBuffer[0] = '\0';
      va_list argp;		
      int iSize;
      va_start( argp, szText );
      /* DeathNMS passe csText (noms d'objets) comme format : un '%' sans arg = crash. */
      if( strchr( szText, '%' ) == NULL )
      {
         strncpy( lpBuffer, szText, sizeof(lpBuffer) - 1 );
         lpBuffer[sizeof(lpBuffer) - 1] = '\0';
         iSize = static_cast<int>( strlen( lpBuffer ) ) + 1;
      }
      else
      {
         iSize = _vscprintf(szText, argp) + 1;
         if(iSize < 4096)
            vsprintf_s( lpBuffer, iSize, szText, argp );
         else
         {
            dwTypeLog = LOG_CHEAT;
            sprintf_s( lpBuffer, 4096,"Invalid Log msg: %s",szText);
         }
      }
      va_end( argp );

      if( lpBuffer[0] == '\0' )
         return;

      /* CString::GetBuffer(0) apres MakeLower() peut crasher sur Linux (DeathNMS, csText vide). */
      std::string msg( lpBuffer );
      for( size_t pos = 0; ( pos = msg.find( '%', pos ) ) != std::string::npos; )
      {
         msg.replace( pos, 1, "%%" );
         pos += 2;
      }
      if( msg.length() > 4090 )
         msg.resize( 4090 );

      std::string lower = msg;
      std::transform( lower.begin(), lower.end(), lower.begin(),
                      []( unsigned char c ) { return static_cast<char>( std::tolower( c ) ); } );
      if( lower.find( "zzqqwxq" ) != std::string::npos )
         return;

      strncpy( lpBuffer, msg.c_str(), sizeof(lpBuffer) - 1 );
      lpBuffer[sizeof(lpBuffer) - 1] = '\0';

     

      // If it should be logged to text file..
      if (GetLog(dwTypeLog) != NULL) 
      {
         LogItemInfo *pNewEntry = new LogItemInfo();
         pNewEntry->wLogLevel   = wLogLevels;
         pNewEntry->strLog      = lpBuffer;
         pNewEntry->pLogProcess = GetLog(dwTypeLog);
         PostQueuedCompletionStatus( s_hLogIO, 0, reinterpret_cast< std::uintptr_t >( pNewEntry ), NULL );
      }

      // Then go for SQL..
      //all this log are only in text file NO Sql...
      if(dwTypeLog == LOG_AH            || 
         dwTypeLog == LOG_CHEAT         || 
         dwTypeLog == LOG_ACHAT_NMS     || 
         dwTypeLog == LOG_SPECIAL_ITEMS || 
         dwTypeLog == LOG_SANCTION      || 
         dwTypeLog == LOG_MONSTERS      ||
         dwTypeLog == LOG_GUILDHIST     ||
         dwTypeLog == LOG_GUILDCHEST    ||
         dwTypeLog == LOG_BDEXTCH       ||
         dwTypeLog == LOG_PROFESSION    ||
         dwTypeLog == LOG_INTERRP       ||
         dwTypeLog == LOG_ARENA         ||
         dwTypeLog == LOG_EVENTS
         )
      {
         return; //not save Ah log in Database for now...
      }

      if ( ((1<<dwTypeLog) & wSQLLogTypes) == 0 ) 
      {
         return; // This log type is not being logged to SQL
      }
      if ( wLogLevels != LOG_ALWAYS && (wLogLevels & wSQLLogLevel) == 0 )
      {
         return;
      }
      // This log level is not being logged to SQL

      // Get the TimeStamp
      SYSTEMTIME sysTime; 
      GetLocalTime(&sysTime);
      CString csTimeStamp;
      csTimeStamp.Format("%04d%02d%02d%02d%02d%02d",sysTime.wYear, sysTime.wMonth,sysTime.wDay,sysTime.wHour, sysTime.wMinute,sysTime.wSecond);

      CString tmp(lpBuffer);
      tmp.Replace("'", "''");
      const char *csTableName = GetLogTableName(dwTypeLog);

      TemplateList< SQL_REQUEST > *lptlSQLRequests = new TemplateList< SQL_REQUEST >;
      LPSQL_REQUEST lpSql = new SQL_REQUEST;

      CString csQuery;
      csQuery.Format( "INSERT INTO %s (%s.Level,%s.TimeStamp,%s.LogInfo) VALUES ('%s','%s','%s')", csTableName, csTableName, csTableName, csTableName, "", csTimeStamp, tmp );
      lpSql->csQuery = csQuery;

      lptlSQLRequests->AddToTail( lpSql );
      ODBCLogs.SendBatchRequest( lptlSQLRequests, CheckForODBCErrorsCallBack, &lpSql->csQuery, "ODBCLogs" );

   }
   catch (...)
   {

   }
}

/******************************************************************************/
void CT4CLog::SaveDeathLog(CString &strVictime,CString &strAssassin, int dwType, int dwArene)
{
   // Get the TimeStamp
   SYSTEMTIME sysTime; 
   GetLocalTime(&sysTime);
   CString csTimeStamp;
   csTimeStamp.Format("%04d%02d%02d%02d%02d%02d",sysTime.wYear, sysTime.wMonth,sysTime.wDay,sysTime.wHour, sysTime.wMinute,sysTime.wSecond);

   CString strTableName;
   strTableName = "LogDeath2";	

   TemplateList< SQL_REQUEST > *lptlSQLRequests = new TemplateList< SQL_REQUEST >;
   LPSQL_REQUEST lpSql = new SQL_REQUEST;

   strVictime .Replace("'"," ");
   strAssassin.Replace("'"," ");
   CString csQuery;
   csQuery.Format( "INSERT INTO LogDeath2 (LogDeath2.TimeStamp,LogDeath2.Victime,LogDeath2.Assassin,LogDeath2.Type,LogDeath2.IsArene) VALUES ('%s','%s','%s',%d,%d)", csTimeStamp, strVictime, strAssassin, dwType, dwArene);
   lpSql->csQuery = csQuery;

   lptlSQLRequests->AddToTail( lpSql );
   ODBCLogs.SendBatchRequest( lptlSQLRequests, CheckForODBCErrorsCallBack, &lpSql->csQuery, "ODBCLogs" );
}

/******************************************************************************/
// Return the name of the table where logs are saved into SQL
const char* CT4CLog::GetLogTableName(  DWORD dwType	)//The log type
{
   switch(dwType) 
   {
      case LOG_DEATH: return "LogDeath";
      case LOG_GAMEOP: return "LogGameop";
      case LOG_PC: return "LogPC";
      case LOG_ITEMS: return "LogItems";
      case LOG_NPCS: return "LogNPCS";
      case LOG_WORLD: return "LogWorld";
      case LOG_DEBUG: return "LogDebug";
      case LOG_PAGE: return "LogPage";
      case LOG_SHOUTS: return "LogShouts";
      case LOG_TEXT: return "LogText";
      case LOG_CHEAT: return "LogCHEAT";
      case LOG_AH: return "LogAH";
      case LOG_ACHAT_NMS: return "LogAchatNMS";
      case LOG_SPECIAL_ITEMS: return "LogItemsSpecial";
      case LOG_SANCTION: return "LogSANCTION";
      case LOG_MONSTERS: return "LogMONSTERS";
      case LOG_GUILDHIST: return "LogGUILDHIST";
      case LOG_GUILDCHEST: return "LogGUILDCHEST";
      case LOG_BDEXTCH: return "LogBDEXTCH";
      case LOG_PROFESSION: return "LogProfession";
      case LOG_INTERRP:   return "LogInteractionRP";
      case LOG_ARENA:   return "LogArena";
      case LOG_EVENTS:   return "LogEvsnts";
         

      default: return "LogDefault";
   }
}

/******************************************************************************/
// Get the result of the ODBC Log requests and decide what to do on fails
void CT4CLog::CheckForODBCErrorsCallBack( DWORD dwSaveStatus,LPVOID lpData)
{
   if (dwSaveStatus == BATCH_SUCCEEDED) 
   {
      return;
   }
   std::string *csQuery = reinterpret_cast<std::string *>(lpData);
   _LOG_DEBUG
      LOG_CRIT_ERRORS,
         "SQL Error (%d) while trying: %s",
         dwSaveStatus,
         csQuery->c_str()
      LOG_
   
      DWORD curRound = TFCMAIN::GetRound();
   if (dwODBCErrorTimer < curRound) 
   {
      ODBCErrorCounter = 0;
   }
   ODBCErrorCounter++;
   if (ODBCErrorCounter >= 10) 
   {
      ODBCLogs.CheckDisconnectError();
      _LOG_DEBUG
         LOG_CRIT_ERRORS,
            "Getting too many (%d) SQL ERROR responses, checking for disconnection.",
            ODBCErrorCounter
         LOG_
   }
   dwODBCErrorTimer = curRound + 12000;	
}

BOOL CT4CLog::IsInit()
{
   return boIsInit;
}

DWORD CT4CLog::GetLogID(DWORD dwType)
{
   if( cT4CLogs == NULL || dwType >= NB_LOG_TYPES )
   {
      return 0;
   }
   return cT4CLogs[ dwType ].dwLogID;
}