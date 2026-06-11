/******************************************************************************
Modify for vs2008 (31/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "TFC Server.h"
#include "Players.h"
#include "TFC_MAIN.H"
#include "ODBCMage.h"
#include "TFCMessagesHandler.h"
#include "AutoConfig.h"
#include "random.h"
#include "PlayerManager.h"
#include "AsyncFuncQueue.h"
#include "format.h"
#include "RegKeyHandler.h"
#include "StatModifierFlagsListing2.h"

#ifdef _DEBUG
	#undef THIS_FILE
	static char THIS_FILE[]=__FILE__;
	#define new DEBUG_NEW
#endif

/******************************************************************************/
extern Random rnd;
extern CTFCServerApp theApp;
extern TFC_MAIN *TFCServer;
static cODBCMage ODBCUsers;//Load Direct et save Threader avec MANUAL commit
DWORD Players::dwAutoSaveFrequency = 420; //CV: A VALIDER remnis a 6 minutes car avec beaucoup de player a 3 minutes sa flood beaucoup trop la bd de save...

/******************************************************************************/
#define AUTH_WATCH_DOG_DELAY		10 MINUTES

// Defines the table rows associated with the different fields.


#define ADD_QUERY	{ \
	LPSQL_REQUEST lpSql = new SQL_REQUEST;\
	lpSql->csQuery = csQuery;\
	lptlSQLRequests->AddToTail( lpSql );\
}

#define SUPERUSER( FLAG ) SetGodFlags( GetGodFlags() | FLAG )
/******************************************************************************/
namespace
/******************************************************************************/
{
	/******************************************************************************/
    class CAutoLoad
	/******************************************************************************/
	{
    public:
        CAutoLoad(){
            CAutoConfig::AddRegInt( theApp.csT4CKEY+CHARACTER_KEY, "ShoutDelay", 10, HKEY_LOCAL_MACHINE );
        };        
    } cAutoLoad;
    const int MaxPageTime = 3 SECONDS;
    const int MaxPageCountPerMaxPageTime = 2;

    const int MaxTalkTime = 1 SECONDS;
    const int MaxTalkCountPerMaxTalkTime = 6;
};

/******************************************************************************/
struct AsyncBeginSessionData
{
    CString theAccount;
    DWORD   dwPlayerAcctID;
	sockaddr_in sockAddr;
};

/******************************************************************************/
Players::Players()
/******************************************************************************/
{	
	//InitializeCriticalSection( &csPacketLock );
	//InitializeCriticalSection( &csUsageLock );
	m_strEmpty = " ";
    loadingCount = 0;
	// IP_addr = "";
    // port_addr = 0;


    IdleChances = 0;
    m_lKeyCode  = 0;

    dwExitDecompte   = 0xFFFF;
	dwReloadDecompte = 0xFFFF;

    in_AskReloadPlayer = FALSE;
    in_game = FALSE;    // Player always starts out of the game
    registred = FALSE;//FALSE;  // Player is, by default, not registred to play!
    god_mode = FALSE;//FALSE; // By default, no one is a GOD! :)
    god_modeModo = FALSE;
    boPreInGame = FALSE;
	boCanSave   = FALSE;
    // Mestoph (31/04/2009) Anti SpeedHack
    iCountMovement = 0;
    iSpeedHackTime = time(NULL);
    bSpeedHackWarned = FALSE;
    //

    self = new Character;
    self->SetPlayer( this );

    i64GodFlags = 0;	// No god flags by default
    boRerolling = FALSE;
    boWhoInvisible = FALSE;
    boCanTalk = TRUE;
    uiTalkIndefinie = 0;//NMNMNM LOCKTMP
    boCanShout = TRUE;
    uiShoutIndefinie = 0;//NMNMNM LOCKTMP
    boCanPage = TRUE;
    uiPageIndefinie = 0;//NMNMNM LOCKTMP
    boLockedOut = FALSE;
    uiLockedComplete = 0;//NMNMNM LOCKTMP

    dwKickoutTime = 0xFFFFFFFF;
	 dwAntiPlugTime = 0;
	 dwAntiPlugTimeReload = 0;
    dwRegenTime = 0;
    dwDeathAndNoXPTime = 0;
    dwSPXPScrollTime   = 0;
    dwSPXPScrollTimeD  = 0;
    dwSPXPScrollTimeTS = 0;
    dwSPORScrollTime   = 0;
    dwSPORScrollTimeD  = 0;
    dwSPORScrollTimeTS = 0;
    
    dwInJailSystemTimeout = 60 MINUTES TDELAY;
    dwNextSave = 0;
    m_bForceSave = FALSE;
    
    boDeletionFlag = FALSE;

    dwNextShout = 0;

    boIdle = false;
    m_bSaveInProgress = false;

    time( &ttLogTime );

    static DWORD dwNewPlayerAcctID = 0;
    dwPlayerAcctID = dwNewPlayerAcctID++;

    dwNextPageTime = 0;
    dwNextPageCount = 0;

    dwNextTalkTime  = 0;
    dwNextTalkCount = 0;

    boCanPVP = true;
    boFullPVP = false;
    pages = true;

    iNMSGold       = 0;
    iNMSGoldSGLv1  = 0;
    iNMSGoldSG     = 0;
    iNMSGoldSELv1  = 0;
    iNMSGoldSE     = 0;
    iNMSGoldReroll = 0;
    iNMSGoldSDLv1  = 0;
    iNMSGoldSD     = 0;
    iNMSGoldToD    = 0;
    iNMSGoldToDLv1 = 0;
    iNMSGoldSGLv1_C= 0;
    iNMSGoldSG_C   = 0;
    iNMSGoldSELv1_C= 0;
    iNMSGoldSE_C   = 0;
    iNMSGoldSDLv1_C= 0;
    iNMSGoldSD_C   = 0;

    dwNbrSanctionA   = 0;
    dwNbrSanctionB   = 0;
    dwLastSanctionTS = 0;

    dwPaidTimeMax = 0;


    //general
    m_dwXPCurrentTick      = 0;  
    m_dwXPLastTickCounter  = 0;
    m_XPCounter = 0;

    m_dwDPSCurrentTick     = 0;  
    m_dwDPSLastTickCounter = 0;
    m_DPSCounter           = 0;

    m_dwAccessLevel = 0;
    m_dwLastTickMove = 0;
    m_dwMoveCount = 0;
    m_strPseudo = "";
    m_dwLastPlayerAttack = 0;
    m_dwLastPlayerAttackID = 0;
    m_dwAttackCount = 0;
    m_dwAutoXPCount = 0;
    m_dwShowID = 0;
    m_dwLastShowID = 0;
    m_dwSpoofedID = 0;
}
/******************************************************************************/
Players::~Players()
/******************************************************************************/
{	
	m_vAPlayerList.clear();
}
/******************************************************************************/
inline char *Players::QuotedAccount( char szBuffer[], CString &csAccount )
/******************************************************************************/
{
    int nLength = csAccount.GetLength();
    int i, j = 0;
    
    for( i = 0; i < nLength; i++ )
	{
        szBuffer[ j ] = csAccount[ i ];
        if( szBuffer[ j ] == '\'' )
		{
            j++;
            szBuffer[ j ] = '\'';
        }

        j++;
    }
    szBuffer[ j ] = 0;

    return szBuffer;
}
/******************************************************************************/
// Connects the Users database for the first time, specifying the DSN
void Players::InitializeODBC( void )
/******************************************************************************/
{
   ODBCUsers.Connect( USERS_DSN, USERS_USER, USERS_PWD );
   ODBCUsers.ConnectOption( SQL_AUTOCOMMIT, SQL_AUTOCOMMIT_OFF );	

   TFormat format;

   // Flush all online user records.
   if(ODBCUsers.SendRequest( "DELETE FROM OnlineUsers"))
      ODBCUsers.Commit();
   ODBCUsers.Cancel();
  

   if(theApp.m_dwResetBoustEquipPos >0 && theApp.m_dwResetBoustEquipPos != theApp.m_dwResetBoustEquipPosOld)
   {
      theApp.m_dwResetBoustEquipPosOld = theApp.m_dwResetBoustEquipPos;
      RegKeyHandler regKey; 
      if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY ) )
         regKey.WriteProfileInt( "OldResetBoustEquipPos", theApp.m_dwResetBoustEquipPosOld );			

      //delete la table de boust...
      if(ODBCUsers.SendRequest( "DELETE FROM Boosts"))
         ODBCUsers.Commit();
      ODBCUsers.Cancel();
   }
}
/******************************************************************************/
// Deconnects from the ODBC server.
void Players::DestroyODBC( void )
/******************************************************************************/
{
    TFormat format;

    // Flush all online user records.
    if(ODBCUsers.SendRequest( "DELETE FROM OnlineUsers"))
       ODBCUsers.Commit();
    ODBCUsers.Cancel();
	 ODBCUsers.Disconnect( );
}

void Players::SetKeyCode(long lKey)
{
   m_lKeyCode = lKey;
}

long Players::GetKeyCode()
{
   return m_lKeyCode;
}
/******************************************************************************/
CString Players::GetIP()
/******************************************************************************/
{
	return inet_ntoa(IPaddrO.sin_addr);
}
/******************************************************************************/
int Players::GetPort()
/******************************************************************************/
{
	return ntohs( IPaddrO.sin_port );
}
/******************************************************************************/
BOOL Players::IsGod(bool bValidModo)
/******************************************************************************/
{
   if(bValidModo)
   {
      if(god_modeModo || god_mode)
         return true;
      else
         return god_modeModo;
   }
   else
	   return god_mode;
}
/******************************************************************************/
void Players::SetGodMode(BOOL bvalidFlag,  unsigned __int64 i64Flags,BOOL asd)
/******************************************************************************/
{
   if(bvalidFlag)
   {
      if(asd == TRUE)
      {
         if(i64Flags != GOD_BOOST_XP                &&   //GOD_BOOST_XP
            i64Flags != GOD_CAN_RUN_CLIENT_SCRIPTS  &&   //GOD_CAN_RUN_CLIENT_SCRIPTS
            i64Flags != GOD_CAN_SQUELCH             &&   //GOD_CAN_SQUELCH
            i64Flags != GOD_CAN_REMOVE_SHOUTS       &&   //GOD_CAN_REMOVE_SHOUTS
            i64Flags != GOD_UNLIMITED_SHOUTS           ) //GOD_UNLIMITED_SHOUTS   
         {
            god_mode = asd;
         }
         else
         {
            god_modeModo = asd;
         }
      }
      else
      {
         god_mode     = asd;
         god_modeModo = asd;
      }
   }
   else
	   god_mode = asd;
}


/******************************************************************************/
void Players::SetGodFlags( unsigned __int64 i64Flags )
/******************************************************************************/
{
	i64GodFlags = i64Flags;
}
/******************************************************************************/
unsigned __int64 Players::GetGodFlags( void )
/******************************************************************************/
{
	return i64GodFlags;
}

/******************************************************************************/
// This functions saves the information of a player in the database
BOOL Players::SaveAccount()
/******************************************************************************/
{	
   if(self && self->ViewFlag(__FLAG_JUST_DO_IT) == 666)//oki
      return TRUE;

   const int BitFieldSize = 64;

   CString csQuery;
   CString csTemp;

   BOOL boInsert = FALSE;
   unsigned __int64 i;

   TemplateList< SQL_REQUEST > *lptlSQLRequests = new TemplateList< SQL_REQUEST >;

   char szAccount[ 1024 ];

   QuotedAccount( szAccount, GetAccount() );    

   // Delete the currently set user flags
   csQuery.Format( "DELETE FROM UserFlags WHERE AccountName='%s'", szAccount );

   ADD_QUERY;

   // If player is god
   if( IsGod(true) )
   {
      // Check each god flag
      for( i = 0; i < BitFieldSize; i++ )
      {
         TRACE( "%u,", i );
         // If player has bit
         if( GetGodFlags() & (unsigned __int64)( (unsigned __int64)(1) << i ) )
         {
            // Add it
            csQuery.Format( "INSERT INTO UserFlags(AccountName, FlagBitPosition, FlagExtraValue) VALUES ('%s', %u, 0)", 
               szAccount,
               i );
            ADD_QUERY;
         }
      }
   }

   if( !boCanTalk )
   {
      csQuery.Format( "INSERT INTO UserFlags(AccountName, FlagBitPosition, FlagExtraValue) VALUES('%s', 10001, %d)", szAccount,uiTalkIndefinie  );
      ADD_QUERY;
   }
   if( !boCanShout )
   {
      csQuery.Format( "INSERT INTO UserFlags(AccountName, FlagBitPosition, FlagExtraValue) VALUES('%s', 10002, %d)", szAccount,uiShoutIndefinie  );
      ADD_QUERY;
   }
   if( boLockedOut )
   {
      csQuery.Format( "INSERT INTO UserFlags(AccountName, FlagBitPosition, FlagExtraValue) VALUES('%s', 10000, %d)", szAccount, uiLockedComplete);
      ADD_QUERY;
   }
   if( boWhoInvisible )
   {
      csQuery.Format( "INSERT INTO UserFlags(AccountName, FlagBitPosition, FlagExtraValue) VALUES('%s', 10003, 0)", szAccount );
      ADD_QUERY;
   }
   if( !boCanPVP )
   {
      csQuery.Format( "INSERT INTO UserFlags(AccountName, FlagBitPosition, FlagExtraValue) VALUES('%s', 10004, 0)", szAccount );
      ADD_QUERY;
   }
   if( !boCanPage )
   {
      csQuery.Format( "INSERT INTO UserFlags(AccountName, FlagBitPosition, FlagExtraValue) VALUES('%s', 10005, %d)", szAccount,uiPageIndefinie );
      ADD_QUERY;
   }
   if( boFullPVP )
   {
      csQuery.Format( "INSERT INTO UserFlags(AccountName, FlagBitPosition, FlagExtraValue) VALUES('%s', 10006, 0)", szAccount );
      ADD_QUERY;
   }

   //Sanctions
   if(dwNbrSanctionA >0)
   {
      csQuery.Format( "INSERT INTO UserFlags(AccountName, FlagBitPosition, FlagExtraValue) VALUES('%s', 10007, %d)", szAccount,dwNbrSanctionA );
      ADD_QUERY;
   }
   if(dwNbrSanctionB>0)
   {
      csQuery.Format( "INSERT INTO UserFlags(AccountName, FlagBitPosition, FlagExtraValue) VALUES('%s', 10008, %d)", szAccount,dwNbrSanctionB );
      ADD_QUERY;
   }
   if(dwLastSanctionTS>0)
   {
      csQuery.Format( "INSERT INTO UserFlags(AccountName, FlagBitPosition, FlagExtraValue) VALUES('%s', 10009, %d)", szAccount,dwLastSanctionTS );
      ADD_QUERY;
   }

   //Paiement Time Options
   if(dwPaidTimeMax > 0)
   {
      csQuery.Format( "INSERT INTO UserFlags(AccountName, FlagBitPosition, FlagExtraValue) VALUES('%s', 11000, %d)", szAccount,dwPaidTimeMax );
      ADD_QUERY;
   }

   //NMSGold Options
   if(iNMSGold >0)
   {
      csQuery.Format( "INSERT INTO UserFlags(AccountName, FlagBitPosition, FlagExtraValue) VALUES('%s', 20000, %d)", szAccount,iNMSGold );
      ADD_QUERY;
   }

   //Write the NMS Gold Options.
   if(iNMSGoldSGLv1 >0)
   {
      csQuery.Format( "INSERT INTO UserFlags(AccountName, FlagBitPosition, FlagExtraValue) VALUES('%s', 20130, %d)", szAccount,iNMSGoldSGLv1 );
      ADD_QUERY;
   }
   if(iNMSGoldSG >0)
   {
      csQuery.Format( "INSERT INTO UserFlags(AccountName, FlagBitPosition, FlagExtraValue) VALUES('%s', 20131, %d)", szAccount,iNMSGoldSG );
      ADD_QUERY;
   }
   if(iNMSGoldSELv1 >0)
   {
      csQuery.Format( "INSERT INTO UserFlags(AccountName, FlagBitPosition, FlagExtraValue) VALUES('%s', 20132, %d)", szAccount,iNMSGoldSELv1 );
      ADD_QUERY;
   }
   if(iNMSGoldSE >0)
   {
      csQuery.Format( "INSERT INTO UserFlags(AccountName, FlagBitPosition, FlagExtraValue) VALUES('%s', 20133, %d)", szAccount,iNMSGoldSE );
      ADD_QUERY;
   }
   if(iNMSGoldReroll >0)
   {
      csQuery.Format( "INSERT INTO UserFlags(AccountName, FlagBitPosition, FlagExtraValue) VALUES('%s', 20134, %d)", szAccount,iNMSGoldReroll );
      ADD_QUERY;
   }
   if(iNMSGoldSDLv1 >0)
   {
      csQuery.Format( "INSERT INTO UserFlags(AccountName, FlagBitPosition, FlagExtraValue) VALUES('%s', 20135, %d)", szAccount,iNMSGoldSDLv1 );
      ADD_QUERY;
   }
   if(iNMSGoldSD >0)
   {
      csQuery.Format( "INSERT INTO UserFlags(AccountName, FlagBitPosition, FlagExtraValue) VALUES('%s', 20136, %d)", szAccount,iNMSGoldSD );
      ADD_QUERY;
   }
   if(iNMSGoldToD >0)
   {
      csQuery.Format( "INSERT INTO UserFlags(AccountName, FlagBitPosition, FlagExtraValue) VALUES('%s', 20137, %d)", szAccount,iNMSGoldToD );
      ADD_QUERY;
   }
   if(iNMSGoldToDLv1 >0)
   {
      csQuery.Format( "INSERT INTO UserFlags(AccountName, FlagBitPosition, FlagExtraValue) VALUES('%s', 20138, %d)", szAccount,iNMSGoldToDLv1 );
      ADD_QUERY;
   }
   if(iNMSGoldSGLv1_C >0)
   {
      csQuery.Format( "INSERT INTO UserFlags(AccountName, FlagBitPosition, FlagExtraValue) VALUES('%s', 20140, %d)", szAccount,iNMSGoldSGLv1_C );
      ADD_QUERY;
   }
   if(iNMSGoldSG_C >0)
   {
      csQuery.Format( "INSERT INTO UserFlags(AccountName, FlagBitPosition, FlagExtraValue) VALUES('%s', 20141, %d)", szAccount,iNMSGoldSG_C );
      ADD_QUERY;
   }
   if(iNMSGoldSELv1_C >0)
   {
      csQuery.Format( "INSERT INTO UserFlags(AccountName, FlagBitPosition, FlagExtraValue) VALUES('%s', 20142, %d)", szAccount,iNMSGoldSELv1_C );
      ADD_QUERY;
   }
   if(iNMSGoldSE_C >0)
   {
      csQuery.Format( "INSERT INTO UserFlags(AccountName, FlagBitPosition, FlagExtraValue) VALUES('%s', 20143, %d)", szAccount,iNMSGoldSE_C );
      ADD_QUERY;
   }
   if(iNMSGoldSDLv1_C >0)
   {
      csQuery.Format( "INSERT INTO UserFlags(AccountName, FlagBitPosition, FlagExtraValue) VALUES('%s', 20145, %d)", szAccount,iNMSGoldSDLv1_C );
      ADD_QUERY;
   }
   if(iNMSGoldSD_C >0)
   {
      csQuery.Format( "INSERT INTO UserFlags(AccountName, FlagBitPosition, FlagExtraValue) VALUES('%s', 20146, %d)", szAccount,iNMSGoldSD_C );
      ADD_QUERY;
   }

   // Save the flags
   ODBCUsers.SendBatchRequest( lptlSQLRequests, NULL, NULL, "ODBCUsers" );
   return TRUE;
}
/******************************************************************************/
// Loads information from a database about a specific user.
BOOL Players::LoadAccount( CString name )// Name of the player to load.
/******************************************************************************/
{
	
	// Desboys: Here is the 3 characters per account Limit.
	const int nMaxUsers = 3;
	int nCount = 0;
	
	CString csQuery;

   LockAPlList();
   m_vAPlayerList.clear();
   UnlockAPlList();



    SetGodFlags( 0 );
    if( _stricmp( (LPCTSTR)name, CAutoConfig::GetStringValue( theApp.csT4CKEY+GEN_CFG_KEY, "SuperUser", HKEY_LOCAL_MACHINE ).c_str() ) == 0 ){
        // SuperUser has the ability of giving god flags.
        SetGodFlags( GOD_CAN_GIVE_GOD_FLAGS );
        SetGodMode( FALSE,0,TRUE );
    }

	//NMNMNMNM pour le NMNMNM Bot...
    //#ifdef _DEBUG
    /*
       if(strstr(GetFullAccountName().GetBuffer(0),"auto") != NULL)
       {
          // SuperUser has the ability of giving god flags.
          SetGodFlags( GOD_CAN_GIVE_GOD_FLAGS );
          SetGodMode( FALSE,0,TRUE );
       }
    */
    //#endif


	ODBCUsers.Lock();

   struct AutoFindoutBug
   {
      AutoFindoutBug()
      {
         _LOG_DEBUG
            LOG_DEBUG_HIGH,
            "Entered Players::LoadAccount"
            LOG_
      }
      ~AutoFindoutBug()
      {
         _LOG_DEBUG
            LOG_DEBUG_HIGH,
            "Left Players::LoadAccount"
            LOG_
      }
   } cAutoFindoutBug;

   char szAccount[ 1024 ];

   _LOG_DEBUG
      LOG_DEBUG_HIGH,
      "Players::LoadAccount QuotedAccount( %s )",
      (LPCTSTR)GetAccount()
      LOG_

   QuotedAccount( szAccount, GetAccount() );

   _LOG_DEBUG
      LOG_DEBUG_HIGH,
      "Players::LoadAccount csQuery.Format()"
      LOG_

   // Send a query to fetch the spcified user.
   csQuery.Format( "SELECT UserID, PlayerName, Appearance, CurrentLevel FROM PlayingCharacters WHERE AccountName='%s' Order by PlayerName", szAccount );

   _LOG_DEBUG
      LOG_DEBUG_HIGH,
      "Players::LoadAccount SendRequest()"
      LOG_

   ODBCUsers.SendRequest( (LPCTSTR)csQuery );

   _LOG_DEBUG
      LOG_DEBUG_HIGH,
      "Players::LoadAccount 1"
      LOG_

   #define DB_UserID				1
   #define DB_UserPC				2
   #define DB_UserAppearance		3
   #define DB_UserLevel				4

   // If user could be fetched.
   while( ODBCUsers.Fetch() && nCount < nMaxUsers )
   {
      char lpszText[ 21 ];
      AccountPlayerInfo plAPlNew;

      //Get ID
      ODBCUsers.GetDWORD ( DB_UserID,	      &plAPlNew.dwID );
      ODBCUsers.GetString( DB_UserPC,         lpszText, 21 );
      ODBCUsers.GetWORD  ( DB_UserAppearance,&plAPlNew.wRace );
      ODBCUsers.GetWORD  ( DB_UserLevel,	   &plAPlNew.level );
      plAPlNew.name.Format("%s",lpszText);
      AddAPlList(plAPlNew.name, plAPlNew.wRace, plAPlNew.level, plAPlNew.dwID);
   }

   WORD wWord;
   DWORD dwDWORD;
   __int64 n64Flag;
   ODBCUsers.Cancel();

   //NMNMNM LOCKTMP
   csQuery.Format( "SELECT FlagBitPosition,FlagExtraValue FROM UserFlags WHERE AccountName='%s'", szAccount );
   TRACE( "SELECT FlagBitPosition,FlagExtraValue FROM UserFlags WHERE AccountName='%s'", szAccount );

   _LOG_DEBUG
      LOG_DEBUG_HIGH,
      "Players::LoadAccount 2"
      LOG_

   ODBCUsers.SendRequest( (LPCTSTR)csQuery );
   // If user could be fetched.
   while( ODBCUsers.Fetch() )
   {
      n64Flag = 0;
      wWord = 0;
      dwDWORD = 0;
      // Get the flag.
      ODBCUsers.GetWORD( 1, &wWord );		
      ODBCUsers.GetDWORD( 2, &dwDWORD );		


      // If its a god flag
      if( wWord < 64 )
      {
         TRACE( "\r\nUser has flag bit %u", wWord );
         n64Flag = wWord;
         // Add it.
         __int64 iOne = 1;
         SetGodFlags( GetGodFlags() | ( iOne << n64Flag ) );
         /*
         if(wWord == 5)
            SetGodFlags( GetGodFlags() | GOD_CAN_SQUELCH );
         if(wWord == 6)
            SetGodFlags( GetGodFlags() | GOD_CAN_REMOVE_SHOUTS );
         if(wWord == 35)
            SetGodFlags( GetGodFlags() | GOD_UNLIMITED_SHOUTS );
         */
         if(wWord != 41  &&   //GOD_BOOST_XP
            wWord != 40  &&   //GOD_CAN_RUN_CLIENT_SCRIPTS
            wWord !=  5  &&   //GOD_CAN_SQUELCH
            wWord !=  6  &&   //GOD_CAN_REMOVE_SHOUTS
            wWord != 35     ) //GOD_UNLIMITED_SHOUTS   
         {
            god_mode = TRUE;
         }
         else
         {
            god_modeModo = TRUE;
         }

         if( wWord > 10 && wWord < 20 ){
            SetGodFlags( GetGodFlags() | GOD_CAN_EDIT_USER );
         }else if( wWord > 20 && wWord < 26 ){
            SetGodFlags( GetGodFlags() | GOD_CAN_VIEW_USER );
         }
      }
      else
      {
         // Otherwise check for special case
         switch( wWord )
         {
         case LockedOut:
            boLockedOut = TRUE;
            uiLockedComplete = dwDWORD;
            break;
         case Squelched:
            boCanTalk = FALSE;
            uiTalkIndefinie = dwDWORD;
            break;
         case NoShouts:
            boCanShout = FALSE;
            uiShoutIndefinie = dwDWORD;
            break;
         case WhoInvisible:
            boWhoInvisible = true;
            break;
         case DisallowedPVP:
            boCanPVP = false;
            break;
         case CannotPage:
            boCanPage = FALSE;
            uiPageIndefinie = dwDWORD;
            break;
         case FullPVP:
            boFullPVP = true;
            break;

         case SanctionCntA:
            dwNbrSanctionA = dwDWORD;
            break;
         case SanctionCntB:
            dwNbrSanctionB = dwDWORD;
            break;
         case SanctionLastTS:
            dwLastSanctionTS = dwDWORD;
            break;


         case ciPaidFreeTime:
            dwPaidTimeMax = dwDWORD;
            break;
         case ciNMSGold:
            iNMSGold = dwDWORD;
            break;

         case ciNMSGoldSGLv1:
            iNMSGoldSGLv1 = dwDWORD;
            break;
         case ciNMSGoldSG:
            iNMSGoldSG = dwDWORD;
            break;
         case ciNMSGoldSELv1:
            iNMSGoldSELv1 = dwDWORD;
            break;
         case ciNMSGoldSE:
            iNMSGoldSE = dwDWORD;
            break;
         case ciNMSGoldReroll:
            iNMSGoldReroll = dwDWORD;
            break;
         case ciNMSGoldSDLv1:
            iNMSGoldSDLv1 = dwDWORD;
            break;
         case ciNMSGoldSD:
            iNMSGoldSD = dwDWORD;
            break;
         case ciNMSGoldToD:
            iNMSGoldToD = dwDWORD;
            break;
         case ciNMSGoldToDLv1:
            iNMSGoldToDLv1 = dwDWORD;
            break;


         case ciNMSGoldSGLv1_C:
            iNMSGoldSGLv1_C = dwDWORD;
            break;
         case ciNMSGoldSG_C:
            iNMSGoldSG_C = dwDWORD;
            break;
         case ciNMSGoldSELv1_C:
            iNMSGoldSELv1_C = dwDWORD;
            break;
         case ciNMSGoldSE_C:
            iNMSGoldSE_C = dwDWORD;
            break;
         case ciNMSGoldSDLv1_C:
            iNMSGoldSDLv1_C = dwDWORD;
            break;
         case ciNMSGoldSD_C:
            iNMSGoldSD_C = dwDWORD;
            break;
         }
      }
   }

   _LOG_DEBUG
      LOG_DEBUG_HIGH,
      "Players::LoadAccount 3"
      LOG_

   // If this is the super user!
   if( _stricmp( (LPCTSTR)GetFullAccountName(),CAutoConfig::GetStringValue( theApp.csT4CKEY+GEN_CFG_KEY, "SuperUser", HKEY_LOCAL_MACHINE ).c_str()) == 0 
      /* ||  strstr(GetFullAccountName().GetBuffer(0),"auto") != NULL*/
      )
   {
      // Give him some god flags.
      SUPERUSER( GOD_CAN_EDIT_USER );
      SUPERUSER( GOD_CAN_VIEW_USER );
      SUPERUSER( GOD_CAN_COPY_USER );
      SUPERUSER( GOD_CAN_EDIT_USER_APPEARANCE_CORPSE );
      SUPERUSER( GOD_CAN_EDIT_USER_BACKPACK );
      SUPERUSER( GOD_CAN_EDIT_USER_HP );
      SUPERUSER( GOD_CAN_EDIT_USER_MANA_FAITH );
      SUPERUSER( GOD_CAN_EDIT_USER_NAME );
      SUPERUSER( GOD_CAN_EDIT_USER_SKILLS );
      SUPERUSER( GOD_CAN_EDIT_USER_SPELLS );
      SUPERUSER( GOD_CAN_EDIT_USER_STAT );
      SUPERUSER( GOD_CAN_EDIT_USER_XP_LEVEL );
      SUPERUSER( GOD_CAN_EMULATE_MONSTER );
      SUPERUSER( GOD_CAN_GIVE_GOD_FLAGS );
      SUPERUSER( GOD_CAN_LOCKOUT_USER );
      SUPERUSER( GOD_CAN_REMOVE_SHOUTS );
      SUPERUSER( GOD_CAN_SEE_ACCOUNTS );
      SUPERUSER( GOD_CAN_SET_USER_FLAG );
      SUPERUSER( GOD_CAN_SHUTDOWN );
      SUPERUSER( GOD_CAN_SLAY_USER );
      SUPERUSER( GOD_CAN_SQUELCH );
      SUPERUSER( GOD_CAN_SUMMON_ITEMS );
      SUPERUSER( GOD_CAN_SUMMON_MONSTERS );
      SUPERUSER( GOD_CAN_TELEPORT );
      SUPERUSER( GOD_CAN_TELEPORT_USER );
      SUPERUSER( GOD_CAN_VIEW_USER_APPEARANCE_CORPSE );
      SUPERUSER( GOD_CAN_VIEW_USER_BACKPACK );
      SUPERUSER( GOD_CAN_VIEW_USER_SKILLS );
      SUPERUSER( GOD_CAN_VIEW_USER_SPELLS );
      SUPERUSER( GOD_CAN_VIEW_USER_STAT );
      SUPERUSER( GOD_CAN_ZAP );
      SUPERUSER( GOD_INVINCIBLE );
      SUPERUSER( GOD_UNLIMITED_SHOUTS );
      SUPERUSER( GOD_CAN_EMULATE_SYSTEM );
      SUPERUSER( GOD_CAN_GIVE_FLAG_TO_HIM );
      SUPERUSER( GOD_CAN_RUN_CLIENT_SCRIPTS);
      SUPERUSER( GOD_CAN_SET_WEATHER );
      SUPERUSER( GOD_SEE_ALL );
      SUPERUSER( GOD_CAN_CHANGE_SETTINGS );
   }

   _LOG_DEBUG
      LOG_DEBUG_HIGH,
      "Players::LoadAccount 4"
      LOG_


   // Cancel fetch
   ODBCUsers.Cancel();
   ODBCUsers.Unlock();
   return TRUE;
}
/******************************************************************************/
CString Players::GetPwd() const
/******************************************************************************/
{
	return m_Password;
}
/******************************************************************************/
// Ajout du système multilingue.
// Association du language utiliser par le client
inline void Players::SetLang( WORD wLanguage )
/******************************************************************************/
{
	if ( IntlText::CheckLng( wLanguage ) ) 
	{
		if (self != NULL) 
		{
			self->wLang = wLanguage; // Par défault wLang est à 0 (Langue anglaise)
		}
	} 
}
/******************************************************************************/
void Players::SetPwd(CString name)
/******************************************************************************/
{
	m_Password = name;
}
/******************************************************************************/
CString Players::GetAccount() const
/******************************************************************************/
{
	return m_Account;
}
/******************************************************************************/
void Players::SetAccount(CString name)
/******************************************************************************/
{
    // Parse the realm part of the account.
    int pos = name.ReverseFind( '@' );

    if( pos != -1 )
	{
        if( theApp.sAuth.m_StripRealmPartOfAccount )
		{
            m_Account = name.Left( pos );
        }
		else
		{
            m_Account = name;
        }
        
        m_Realm   = name.Right( name.GetLength() - pos - 1 );
    }
	else
	{
        m_Account = name;
    }
}

/******************************************************************************/
// Locks player usage. Should never be called except by Death.
void Players::UseDeathLock( void )
/******************************************************************************/
{
    _LOG_DEBUG
        LOG_DEBUG_HIGH,
        "Trying usagelock(%s).",
        (LPCTSTR)GetAccount()
    LOG_	
    
    loadingCount++;
    csUsageLock.Lock();
    
    _LOG_DEBUG
        LOG_DEBUG_HIGH,
        "Got usagelock(%s). loadingCount=%u.",
        (LPCTSTR)GetAccount(),
        loadingCount
    LOG_	
}
/******************************************************************************/
void Players::UseUnlock( const char *lpszFileName, int nLineNumber )
// Unlocks player usage.
/******************************************************************************/
{
    loadingCount--;
    _LOG_DEBUG
        LOG_DEBUG_HIGH,
        "UsageUnlock(%s) at %s(%u). loadingCount=%u",
        (LPCTSTR)GetAccount(),
        lpszFileName,
        nLineNumber,
        loadingCount
    LOG_
	
    csUsageLock.Unlock();
}
/******************************************************************************/
// Picklocks player usage.
BOOL Players::UsePicklock( const char *lpszFileName, int nLineNumber )
/******************************************************************************/
{   
    if( csUsageLock.PickLock() )
	{
        loadingCount++;
        _LOG_DEBUG
            LOG_DEBUG_HIGH,
            "UsagePicklock(%s) at %s(%u). loadingCount=%u",
            (LPCTSTR)GetAccount(),
            lpszFileName,
            nLineNumber,
            loadingCount
        LOG_
        return TRUE;
    }
    return FALSE;
}

bool Players::CanPaidPlay()
{
   if(!theApp.IsFreeTime())
   {
      //need to validate this players times...
      time_t tTimeNow =  time(NULL);
      if(GetPaidTime() > tTimeNow)
         return true;
      else
         return false;
   }
   return true;
}

void Players::GetLastGetPos(WorldPos &Wl)
{
   Wl.X     = self->ViewFlag( __FLAG_GET_RETURN_LAST_POS_X );
   Wl.Y     = self->ViewFlag( __FLAG_GET_RETURN_LAST_POS_Y );
   Wl.world = self->ViewFlag( __FLAG_GET_RETURN_LAST_POS_W );
}
void Players::SetLastGetPos(int x,int y, int w)
{
   self->SetFlag( __FLAG_GET_RETURN_LAST_POS_X ,x);
   self->SetFlag( __FLAG_GET_RETURN_LAST_POS_Y ,y);
   self->SetFlag( __FLAG_GET_RETURN_LAST_POS_W ,w);
}

bool Players::CanConnectGMOnly()
{
   BOOL bGMOnly = FALSE;
   RegKeyHandler regKey;
   if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY ) )
   {
      bGMOnly = static_cast< DWORD >( regKey.GetProfileInt( "AllowOnlyGM", 0 ) );

      regKey.Close();
   }

   if(bGMOnly && IsGod())
      return true;
   else if(bGMOnly)
      return false;
   else
      return true;
}




/******************************************************************************/
bool Players::IsLoading()
{
    return( loadingCount > 0 );
}
/******************************************************************************/
// Sets the round at which user first entered game.
void Players::BeginSession( void )
/******************************************************************************/
{
    //dwAuthWatchDogTime = AUTH_WATCH_DOG_DELAY;    
    time( &ttLogTime );
}
/******************************************************************************/
// Returns the elapsed seconds since player got online.
double Players::GetOnlineTime( void )
/******************************************************************************/
{
    time_t ttCurTime;
    time( &ttCurTime );
    
    return difftime( ttCurTime, ttLogTime );
}
/******************************************************************************/
// Queries if player should auto-save.
/******************************************************************************/

void Players::QueryNextSave( void )
{
   if( dwNextSave != 0 )
   {
      // If auto-save triggers.
      if( dwNextSave <= TFCMAIN::GetRound() || m_bForceSave == TRUE)
      {
         // Save character
         if(m_bForceSave)
            self->SaveCharacter(FALSE,"QueryNextSave-Forced",TRUE); //Query Next Save in playerMaintenance Thread.. the real Save...
         else
            self->SaveCharacter(FALSE,"QueryNextSave",TRUE); //Query Next Save in playerMaintenance Thread.. the real Save...

         // Set next auto-save time.
         dwNextSave = rnd( dwAutoSaveFrequency - 80, dwAutoSaveFrequency + 80 ) SECONDS TDELAY; //il ne faut pas condenser les save pour rien...    

         m_bForceSave = FALSE;
      }
   }
}

void Players::ForceSave()
{
   m_bForceSave = TRUE;
}

/******************************************************************************/
// Sets the next auto save.
void Players::SetNextSave( void )
/******************************************************************************/
{
    dwNextSave = 5 MINUTES + ( rnd( 0, 240 ) SECONDS ) TDELAY;
}
/******************************************************************************/
// Resets the idle time on the player structure.
void Players::ResetIdle( void )
/******************************************************************************/
{
    IdleTime    = TFCMAIN::GetRound() + TFCServer->flush_time;
    IdleChances = 0;
}
/******************************************************************************/
// Determines if player is idle or not.
BOOL Players::IsIdle( void )
/******************************************************************************/
{
   if( boIdle )
   {
      return true;
   }

   // If player is time-idle
   if( IdleTime <= TFCMAIN::GetRound() || dwKickoutTime < TFCMAIN::GetRound() )
   {
      IdleTime = TFCMAIN::GetRound() + TFCServer->flush_time;

      IdleChances++;
      // If max chances exceeded.
      if( IdleChances > TFCServer->max_chances )
      {
         _LOG_DEBUG
            LOG_DEBUG_LVL1,
            "Flushed player %s because chances ran out.",
            (LPCTSTR)GetFullAccountName()
            LOG_

            // Faith is scealed, player is IDLE! 
            boIdle = true;
         return TRUE;
      }
      if( dwKickoutTime < TFCMAIN::GetRound() )
      {
         _LOG_DEBUG
            LOG_DEBUG_LVL1,
            "Flushed player %s because kickout time expired.",
            (LPCTSTR)GetFullAccountName()
            LOG_

            boIdle = true;
         return TRUE;
      }
      // Send ack to client.
      TFCPacket sending;

      sending << (RQ_SIZE)RQ_Ack;

      //TRACE( "\r\nSending Keep-Alive" );
      self->SendPlayerMessage( sending );
   }

   return FALSE;
}
/******************************************************************************/
// Queries if a player can shout.
bool Players::CanShout( void )
/******************************************************************************/
{
    if( dwNextShout <= TFCMAIN::GetRound() )
	{
        return true;
    }
    return false;
}
/******************************************************************************/
// Called to flag the user's shout.
void Players::ToggleShout( void )
/******************************************************************************/
{           
    // If player has unlimited shouts
    if( GetGodFlags() & GOD_UNLIMITED_SHOUTS )
	{
        dwNextShout = 0;
    }
	else
	{
        // If player has a shout-delay specific flag.
        DWORD dwShoutDelay = self->ViewFlag( __FLAG_SHOUT );
        if( dwShoutDelay != 0 )
		{
            dwNextShout = dwShoutDelay SECONDS TDELAY;
        }
		else
		{
            TRACE( "\r\nShoutDelay=%u.", CAutoConfig::GetIntValue( theApp.csT4CKEY+CHARACTER_KEY, "ShoutDelay", HKEY_LOCAL_MACHINE ) );
            dwNextShout = CAutoConfig::GetIntValue( theApp.csT4CKEY+CHARACTER_KEY, "ShoutDelay", HKEY_LOCAL_MACHINE ) SECONDS TDELAY;
        }
    }
}
/******************************************************************************/
//  When a character logs on.
void Players::Logon( void )
/******************************************************************************/
{
     // Add player entry to OnlineUser table.
    char szAccount[ 1024 ];
    QuotedAccount( szAccount, GetAccount() );
    
    CString csQuery;
    TemplateList< SQL_REQUEST > *lptlSQLRequests = new TemplateList< SQL_REQUEST >;

    csQuery.Format( "UPDATE OnlineUsers SET PlayerName='%s' WHERE AccountName='%s'",(LPCTSTR)self->GetTrueName(),szAccount);
    ADD_QUERY;

    ODBCUsers.SendBatchRequest( lptlSQLRequests, NULL, NULL, "ODBCUsers" );
}
/******************************************************************************/
//  Callback called when player is logged off.
void Players::Logoff( void )
/******************************************************************************/
{    
   // Remove character from its current group.
   if( self->GetGroup() != NULL )
   {
      self->GetGroup()->Dismiss( self );
      self->SetGroup(NULL);
   }

   // Cancel any running trade
   self->GetTradeMgr2()->Cancel();

   // Remove player entry from OnlineUser table.

   // ***********************************************************
   // Accountname kann bei FakeClients SQL-Injektion enthalten !
   // possible sql-injection with fake-client!
   CString accountName;
   accountName.Format("%s", GetAccount());
   accountName.Remove(';');
   accountName.Remove(',');
   accountName.Remove('\'');
   if (accountName.GetLength() > 20)
   {
      accountName.Format("%s", accountName.Left(20));
   }
   // ***********************************************************

    // Remove player entry from OnlineUser table.
    char szAccount[ 1024 ];
    QuotedAccount( szAccount, accountName ); //QuotedAccount( szAccount, GetAccount() );

    CString csQuery;
    TemplateList< SQL_REQUEST > *lptlSQLRequests = new TemplateList< SQL_REQUEST >;
 	

    csQuery.Format( "DELETE FROM OnlineUsers WHERE AccountName='%s'", szAccount );
    ADD_QUERY;

    ODBCUsers.SendBatchRequest( lptlSQLRequests, NULL, NULL, "ODBCUsers" );
}
/******************************************************************************/
// Determines if the player can page right now.
bool Players::CanPage( void )
/******************************************************************************/
{
   // If no-paging time has expired.
   if( dwNextPageTime <= TFCMAIN::GetRound() )
   {
      // Decrease the page count.
      // Remove the quantity of 'MaxPageTime' found between the current time and the set NextPageTime.
      // +1 insures that even if the page time just expired, the page count will decrease.        
      DWORD dwCountDecrease = ( ( TFCMAIN::GetRound() - dwNextPageTime ) / MaxPageTime ) + 1;

      // If the next page count is bigger then the desired decrease.
      if( dwNextPageCount > dwCountDecrease )
      {
         // Decrease 
         dwNextPageCount -= dwCountDecrease;
      }
      else
      {
         // Otherwise the new page count is 0.
         dwNextPageCount = 0;
      }
   }

   // If the page count is smaller than the maximum allowed pages in a given time.
   if( dwNextPageCount < MaxPageCountPerMaxPageTime || GetGodFlags() & GOD_UNLIMITED_SHOUTS )
   {
      // Player can page.
      return true;
   }

   // Player cannot page.
   return false;
}
/******************************************************************************/
//  Notifies the player structure that the player just paged.
void Players::PageNotification( void )
/******************************************************************************/
{
    // Reset the next page time.
    dwNextPageTime = MaxPageTime TDELAY;

    // Augment the page count.
    dwNextPageCount++;
}


bool Players::NMCanTalk( void )
/******************************************************************************/
{
   // If no-paging time has expired.
   if( dwNextTalkTime <= TFCMAIN::GetRound() )
   {
      // Decrease the talk count.
      // Remove the quantity of 'MaxtalkTime' found between the current time and the set NexttalkTime.
      // +1 insures that even if the talk time just expired, the talk count will decrease.        
      DWORD dwCountDecrease = ( ( TFCMAIN::GetRound() - dwNextTalkTime ) / MaxTalkTime ) + 1;

      // If the next talk count is bigger then the desired decrease.
      if( dwNextTalkCount > dwCountDecrease )
      {
         // Decrease 
         dwNextTalkCount -= dwCountDecrease;
      }
      else
      {
         // Otherwise the new talk count is 0.
         dwNextTalkCount = 0;
      }
   }

   // If the talk count is smaller than the maximum allowed talks in a given time.
   if( dwNextTalkCount < MaxTalkCountPerMaxTalkTime || GetGodFlags() & GOD_UNLIMITED_SHOUTS )
   {
      // Player can talk.
      return true;
   }

   // Player cannot talk.
   return false;
}
/******************************************************************************/
//  Notifies the player structure that the player just talked.
void Players::NMTalkNotification( void )
/******************************************************************************/
{
   // Reset the next talk time.
   dwNextTalkTime = MaxTalkTime TDELAY;

   // Augment the talk count.
   dwNextTalkCount++;
}




/******************************************************************************/
//  Determines if a player is already logged on, on another server perhaps.
bool Players::AccountLogged(
 const CString &csAccountName, // The name to verify
 LPCSTR szIP                    // Textual IP adress of player.
)
/******************************************************************************/
{
   TFormat format;  
   bool boLogged = false;

   ODBCUsers.Lock();	

   // Send a request to fetch the user from the OnlineUsers table
   if( ODBCUsers.SendRequest(format(
                              "INSERT INTO OnlineUsers(MachineName,AccountName,PlayerName,IPaddr) VALUES ('%s','%s','<%s>','%s')",
                              (LPCTSTR)theApp.csMachineName,(LPCTSTR)csAccountName,_STR( 2846, _DEFAULT_LNG ),szIP), false )          )
   {
      ODBCUsers.Commit();
   }
   else
      boLogged = true;

   // Otherwise the player isn't there.
   ODBCUsers.Cancel();
   ODBCUsers.Unlock();

   return boLogged;
}
/******************************************************************************/
// How many connections from this IP?
int Players::IPLogged( LPCSTR szIP )
/******************************************************************************/
{
	CString csQuery;
	int nConnection = 0;

	csQuery.Format( "SELECT AccountName FROM OnlineUsers WHERE IPaddr='%s'", szIP );

	ODBCUsers.Lock();
	ODBCUsers.SendRequest( csQuery );
	// Count the connections
	while( ODBCUsers.Fetch() )
	{
		nConnection++;				
	}
	ODBCUsers.Cancel();
	ODBCUsers.Unlock();
	return nConnection;
}
/******************************************************************************/
//  Determines if a player name exists in the player database.
bool Players::NameExists( const string &name ) // The name of the player.
/******************************************************************************/
{
    TFormat format;
    bool exists = true;
    ODBCUsers.Lock();
    // Send a request to fetch the user name from PlayingCharacters.
    if( ODBCUsers.SendRequest(format("SELECT PlayerName FROM PlayingCharacters WHERE PlayerName='%s'",name.c_str())))
    {
       // If no field containing this name could be fetched.
       if( !ODBCUsers.Fetch() )
       {
          exists = false;            
       }
       ODBCUsers.Cancel();
    }
    ODBCUsers.Unlock();
    return exists;
}
/******************************************************************************/
//  Called to notify the database that the account failed to authenticate.
void Players::AccountLoggonFailed( const CString &csAccountName )// The account name.
/******************************************************************************/
{
    CString csQuery;
    TemplateList< SQL_REQUEST > *lptlSQLRequests = new TemplateList< SQL_REQUEST >;
 	

    csQuery.Format( "DELETE FROM OnlineUsers WHERE AccountName='%s'", csAccountName );
    ADD_QUERY;

    ODBCUsers.SendBatchRequest( lptlSQLRequests, NULL, NULL, "ODBCUsers" );
}

/******************************************************************************/
CString Players::GetRealm() const
/******************************************************************************/
{
    return m_Realm;
}
/******************************************************************************/
CString Players::GetFullAccountName() const
/******************************************************************************/
{
   if( theApp.sAuth.m_StripRealmPartOfAccount && !m_Realm.IsEmpty() )
	{
        CString fullName( m_Account );
        fullName += "@";
        fullName += m_Realm;
        return fullName;
    }
    return m_Account;
}

bool Players::SetNMSGoldFlag(int iFlag,int iVal)
{
   switch(iFlag)
   {
      case ciNMSGoldSGLv1 :      SetNMSGoldSGLv1(iVal);   return true;  break;
      case ciNMSGoldSG    :      SetNMSGoldSG(iVal);      return true;  break;
      case ciNMSGoldSELv1 :      SetNMSGoldSELv1(iVal);   return true;  break;
      case ciNMSGoldSE    :      SetNMSGoldSE(iVal);      return true;  break;
      case ciNMSGoldReroll:      SetNMSGoldReroll(iVal);  return true;  break;
      case ciNMSGoldSDLv1 :      SetNMSGoldSDLv1(iVal);   return true;  break;
      case ciNMSGoldSD    :      SetNMSGoldSD(iVal);      return true;  break;
      case ciNMSGoldToD   :      SetNMSGoldToD(iVal);     return true;  break;
      case ciNMSGoldToDLv1:      SetNMSGoldToDLv1(iVal);  return true;  break;
         

      case ciNMSGoldSGLv1_C:     SetNMSGoldSGLv1_C(iVal); return true;  break;
      case ciNMSGoldSG_C   :     SetNMSGoldSG_C(iVal);    return true;  break;
      case ciNMSGoldSELv1_C:     SetNMSGoldSELv1_C(iVal); return true;  break;
      case ciNMSGoldSE_C   :     SetNMSGoldSE_C(iVal);    return true;  break;
      case ciNMSGoldSDLv1_C:     SetNMSGoldSDLv1_C(iVal); return true;  break;
      case ciNMSGoldSD_C   :     SetNMSGoldSD_C(iVal);    return true;  break;
   }
   return false;
}

int Players::GetNMSGoldFlag(int iFlag)
{
   switch(iFlag)
   {
      case ciNMSGoldSGLv1 :      return GetNMSGoldSGLv1();  
      case ciNMSGoldSG    :      return GetNMSGoldSG();     
      case ciNMSGoldSELv1 :      return GetNMSGoldSELv1();  
      case ciNMSGoldSE    :      return GetNMSGoldSE();     
      case ciNMSGoldReroll:      return GetNMSGoldReroll(); 
      case ciNMSGoldSDLv1 :      return GetNMSGoldSDLv1();  
      case ciNMSGoldSD    :      return GetNMSGoldSD();     
      case ciNMSGoldToD   :      return GetNMSGoldToD();    
      case ciNMSGoldToDLv1:      return GetNMSGoldToDLv1();    

      case ciNMSGoldSGLv1_C:     return GetNMSGoldSGLv1_C();
      case ciNMSGoldSG_C   :     return GetNMSGoldSG_C();   
      case ciNMSGoldSELv1_C:     return GetNMSGoldSELv1_C();
      case ciNMSGoldSE_C   :     return GetNMSGoldSE_C();   
      case ciNMSGoldSDLv1_C:     return GetNMSGoldSDLv1_C();
      case ciNMSGoldSD_C   :     return GetNMSGoldSD_C();   
   }                                    
   return 0;
}






bool Players::GetAPlayerListItem(int iIndex,AccountPlayerInfo &plInfo)
{
   LockAPlList();
   if(iIndex < 0 || iIndex >=m_vAPlayerList.size())
   {
      UnlockAPlList();
      return false;
   }

   plInfo.name  = m_vAPlayerList[iIndex].name ;
   plInfo.level = m_vAPlayerList[iIndex].level;
   plInfo.wRace = m_vAPlayerList[iIndex].wRace;
   plInfo.dwID  = m_vAPlayerList[iIndex].dwID ;

   UnlockAPlList();
   return true;
}

DWORD Players::GetAPlayerListItemID(int iIndex)
{
   if(iIndex < 0 || iIndex >=m_vAPlayerList.size())
   {
      return 0;
   }

   return m_vAPlayerList[iIndex].dwID;
}



void Players::AddAPlList(CString name, WORD race, WORD level, DWORD dwID)
{
   AccountPlayerInfo newApl;
   newApl.name  = name;
   newApl.level = level;
   newApl.wRace = race;
   newApl.dwID  = dwID;

   LockAPlList();
   m_vAPlayerList.push_back(newApl);
   UnlockAPlList();
}
void Players::ModifyAPlList(CString name,WORD race,WORD level)
{
   LockAPlList();
   for(UINT i=0;i<m_vAPlayerList.size();i++)
   {
      if(m_vAPlayerList[i].name == name)
      {
         m_vAPlayerList[i].level = level;
         m_vAPlayerList[i].wRace = race;
         i = m_vAPlayerList.size();
      }
   }
   UnlockAPlList();
}

void Players::RemoveAPlList(CString name)
{
   LockAPlList();
   vector<AccountPlayerInfo>::iterator itS = m_vAPlayerList.begin();
   vector<AccountPlayerInfo>::iterator itE = m_vAPlayerList.end();
   for(itS ; itS!=itE ; itS++)
   {
      if(itS->name == name)
      {
         m_vAPlayerList.erase(itS);
         UnlockAPlList();
         return;
      }
   }
   UnlockAPlList();
}

void Players::BuildAccountPlayerList(TFCPacket &sending)
{
   short  shTmp        = 0;

   LockAPlList();

   //genere la liste des players
   sending << (RQ_SIZE)RQ_GetPersonnalPClist;
   sending << (char)m_vAPlayerList.size();

   for(UINT i=0;i<m_vAPlayerList.size();i++)
   {
      sending << (char)m_vAPlayerList[i].name.GetLength();
      for(UINT k=0; k < m_vAPlayerList[i].name.GetLength(); k++)
         sending << (char)(m_vAPlayerList[i].name[k]);
      sending << (short)m_vAPlayerList[i].wRace;
      sending << (short)m_vAPlayerList[i].level;
      sending << (short)shTmp;
   }
    
   UnlockAPlList();
}

void Players::BuildAccountPlayerListEquip(TFCPacket &sendingE)
{
   USHORT ushHairColor = 0;
   int iEquipped[ EQUIP_POSITIONS ];

   LockAPlList();

   //genere les equipement des player
   sendingE << (RQ_SIZE)RQ_GetPersonnalPClistEquitSkin;
   sendingE << (char)m_vAPlayerList.size();

   DWORD dwPlayerID;
   for(UINT pp=0;pp<m_vAPlayerList.size();pp++)
   {
      dwPlayerID = GetAPlayerListItemID(pp);
      if(dwPlayerID > 0)
      {
         self->LoadEquipedFromID(iEquipped,dwPlayerID);
         self->LoadHairColorFromID(ushHairColor,dwPlayerID);
         ///clear les equipement...
         for(int i = 0; i < EQUIP_POSITIONS; i++)
         {
            if( iEquipped[i] != -1 )
            {
               sendingE << (short)iEquipped[i];
            }
            else
            {
               sendingE << (short)0;
            }
         }
      }
      else
      {
         for(int i = 0; i < EQUIP_POSITIONS; i++)
         {
            sendingE << (short)0;
         }
      }

      sendingE << (short)ushHairColor;
   }
   UnlockAPlList();
}


void Players::BuildAccountPlayerListAsync()
{
   TFCPacket sendingList;
   Lock();
   try
   {
      BuildAccountPlayerList(sendingList);
      self->SendPlayerMessage( sendingList );
   }
   catch (...)
   {
     
   }
   Unlock();
}

void Players::BuildAccountPlayerListEquipAsync()
{
   TFCPacket sendingListEquip;
   Lock();
   try
   {
      BuildAccountPlayerListEquip(sendingListEquip);
      self->SendPlayerMessage( sendingListEquip );
   }
   catch (...)
   {

   }
   Unlock();
}


