/******************************************************************************
Modify for vs2008 (06/05/2009)
/******************************************************************************/
#if !defined(AFX_PLAYERS_H__C2AF9A46_AC25_11D0_9B9E_444553540000__INCLUDED_)
#define AFX_PLAYERS_H__C2AF9A46_AC25_11D0_9B9E_444553540000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "Character.h"
#include "Objects.h"
#include "Lock.h"
#ifdef _WIN32
#include "WINSOCK.H"
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
typedef int SOCKET;
#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif
#ifndef SOCKET_ERROR
#define SOCKET_ERROR -1
#endif
#endif
#include <vector>

const int LockedOut      = 10000;
const int Squelched      = 10001;
const int NoShouts	    = 10002;
const int WhoInvisible   = 10003;
const int DisallowedPVP  = 10004;
const int CannotPage     = 10005;
const int FullPVP        = 10006;
const int SanctionCntA   = 10007;
const int SanctionCntB   = 10008;
const int SanctionLastTS = 10009;

const int ciPaidFreeTime = 11000;

//NMS GOLD SYSTEM
const int ciNMSGold          = 20000;
const int ciNMSGoldSGLv1     = 20130;
const int ciNMSGoldSG        = 20131;
const int ciNMSGoldSELv1     = 20132;
const int ciNMSGoldSE        = 20133;
const int ciNMSGoldReroll    = 20134;
const int ciNMSGoldSDLv1     = 20135;
const int ciNMSGoldSD        = 20136;
const int ciNMSGoldToD       = 20137;
const int ciNMSGoldToDLv1    = 20138;
const int ciNMSGoldSGLv1_C   = 20140;
const int ciNMSGoldSG_C      = 20141;
const int ciNMSGoldSELv1_C   = 20142;
const int ciNMSGoldSE_C      = 20143;
const int ciNMSGoldSDLv1_C   = 20145;
const int ciNMSGoldSD_C      = 20146;


const int g_iNbrSanctionBeforeLock = 4; //ne pas changer sa sa impact le client....

const int g_iDelayExpireSanction   = (182*86400); //6 mois

/******************************************************************************/
#define MAX_CHATTERS	5

/******************************************************************************/

typedef struct _AccountPlayerInfo
{
   CString name;
   WORD level;
   WORD wRace;
   DWORD dwID;
}AccountPlayerInfo;


/******************************************************************************/
struct ChatterChannel
{
   CString Pwd;
   DWORD   ID;
};

/******************************************************************************/
// Defines the different god flags.
#define GOD_NO_CLIP								   0x0000000000000001 //   0
#define GOD_NO_MONSTERS							   0x0000000000000002 //   1
#define GOD_CAN_TELEPORT						   0x0000000000000004 //   2
#define GOD_CAN_TELEPORT_USER					   0x0000000000000008 //   3
#define GOD_CAN_ZAP								   0x0000000000000010 //   4
#define GOD_CAN_SQUELCH							   0x0000000000000020 //   5
#define GOD_CAN_REMOVE_SHOUTS					   0x0000000000000040 //   6
#define GOD_CAN_SUMMON_MONSTERS				   0x0000000000000080 //   7
#define GOD_CAN_SUMMON_ITEMS					   0x0000000000000100 //   8
#define GOD_CAN_SET_USER_FLAG					   0x0000000000000200 //   9 
#define GOD_CAN_EDIT_USER						   0x0000000000000400 //   10
#define GOD_CAN_EDIT_USER_STAT				   0x0000000000000800 //   11
#define GOD_CAN_EDIT_USER_HP					   0x0000000000001000 //   12
#define GOD_CAN_EDIT_USER_MANA_FAITH		   0x0000000000002000 //   13
#define GOD_CAN_EDIT_USER_XP_LEVEL				0x0000000000004000 //   14
#define GOD_CAN_EDIT_USER_NAME					0x0000000000008000 //   15
#define GOD_CAN_EDIT_USER_APPEARANCE_CORPSE	0x0000000000010000 //   16
#define GOD_CAN_EDIT_USER_SPELLS				   0x0000000000020000 //   17
#define GOD_CAN_EDIT_USER_SKILLS				   0x0000000000040000 //   18
#define GOD_CAN_EDIT_USER_BACKPACK				0x0000000000080000 //   19
#define GOD_CAN_VIEW_USER						   0x0000000000100000 //   20
#define GOD_CAN_VIEW_USER_STAT					0x0000000000200000 //   21
#define GOD_CAN_VIEW_USER_BACKPACK				0x0000000000400000 //   22
#define GOD_CAN_VIEW_USER_SPELLS				   0x0000000000800000 //   23
#define GOD_CAN_VIEW_USER_SKILLS				   0x0000000001000000 //   24
#define GOD_CAN_VIEW_USER_APPEARANCE_CORPSE	0x0000000002000000 //   25
#define GOD_CAN_LOCKOUT_USER					   0x0000000004000000 //   26
#define GOD_CAN_SLAY_USER						   0x0000000008000000 //   27
#define GOD_CAN_COPY_USER						   0x0000000010000000 //   28
#define GOD_CAN_EMULATE_MONSTER					0x0000000020000000 //   29
#define GOD_INVINCIBLE							   0x0000000040000000 //   30
#define GOD_DEVELOPPER							   0x0000000080000000 //   31
#define GOD_CAN_SHUTDOWN                     0x0000000100000000 //   32
#define GOD_CAN_SEE_ACCOUNTS                 0x0000000200000000 //   33
#define GOD_CAN_GIVE_GOD_FLAGS               0x0000000400000000 //   34
#define GOD_UNLIMITED_SHOUTS                 0x0000000800000000 //   35
#define GOD_TRUE_INVISIBILITY                0x0000001000000000 //   36
#define GOD_CAN_EMULATE_SYSTEM               0x0000002000000000 //   37
#define GOD_CHAT_MASTER                      0x0000004000000000 //   38
#define GOD_CANNOT_DIE                       0x0000008000000000 //   39
#define GOD_CAN_RUN_CLIENT_SCRIPTS           0x0000010000000000 //   40
#define GOD_BOOST_XP                         0x0000020000000000 //   41
#define GOD_CAN_GIVE_FLAG_TO_HIM             0x0000040000000000 //   42
#define GOD_CAN_SET_WEATHER						0x0000080000000000 //   43
#define GOD_SEE_ALL        						0x0000100000000000 //   44
#define GOD_CAN_CHANGE_SETTINGS  				0x0000200000000000 //   45

#define UPDATE_GOD_CAN_RUN_CLIENT_SCRIPTS		1
#define UPDATE_GOD_CAN_SLAY_USER				2	

/******************************************************************************/
class __declspec(dllexport) Players : public CLock
   /******************************************************************************/
{
public:
   Players();
   virtual ~Players();    

   sockaddr_in     GetSockAddr();
   CString         GetIP();
   void SetKeyCode(long lKey);
   long GetKeyCode();

   int             GetPort();
   BOOL            IsGod(bool bValidModo = false);
   void            SetGodMode(BOOL bvalidFlag,  unsigned __int64 i64Flags,BOOL asd);

   BOOL            SaveAccount();
   BOOL            LoadAccount(CString name);

   static char *   QuotedAccount( char szBuffer[], const CString &csAccount );
   void            SetAccount(CString name);
   CString         GetAccount() const;
   CString         GetFullAccountName() const; // Account + realm
   CString         GetRealm() const;

   void            SetPwd(CString pwd);
   CString         GetPwd() const;

   void SetLang( WORD wLanguage ); // Ajout du syst�me multilingue.


   
   void     LockAPlList(){m_LockApl.Lock();}
   void     UnlockAPlList(){m_LockApl.Unlock();}
   int      GetNbrAPlayerList(){return m_vAPlayerList.size(); }
   bool     GetAPlayerListItem(int iIndex,AccountPlayerInfo &plInfo);
   DWORD    GetAPlayerListItemID(int iIndex);

   void     AddAPlList(CString name, WORD race, WORD level,DWORD dwID);
   void     ModifyAPlList(CString name, WORD race, WORD level);
   void     RemoveAPlList(CString name);

   void     BuildAccountPlayerList(TFCPacket &sending);
   void     BuildAccountPlayerListEquip(TFCPacket &sendingE);
   void     BuildAccountPlayerListAsync();
   void     BuildAccountPlayerListEquipAsync();
   

   

   void                SetGodFlags( unsigned __int64 i64Flags );
   unsigned __int64    GetGodFlags();

   static void     InitializeODBC( void );
   static void     DestroyODBC( void );

   void            UseDeathLock();
   void            UseUnlock( const char *lpszFileName, int nLineNumber );
   BOOL            UsePicklock( const char *lpszFileName, int nLineNumber );

   void            BeginSession( void );

   double          GetOnlineTime( void );

   void            SetNextSave( void );
   void            QueryNextSave( void );
   void            ForceSave();

   void            ResetIdle( void );
   BOOL            IsIdle( void );

   void            SetDeletePlayerFlags( void ){ boDeletionFlag = TRUE; };
   void            ClearDeletePlayerFlags( void ){ boDeletionFlag = FALSE; };
   BOOL            IsDeleteFlags( void ){ return boDeletionFlag; };

   void            Logon( void );
   void            Logoff( void );

   bool            CanPage( void );
   void            PageNotification( void );

   bool            NMCanTalk( void );
   void            NMTalkNotification( void );

   bool CanShout( void );
   void ToggleShout( void );        

   bool            CanPVP(){ return boCanPVP; };
   void            SetPVP( bool boCanUserPVP ){ boCanPVP = boCanUserPVP; };

   void            SetRadiusPortID( DWORD dwPortID ){ dwRadiusPortID = dwPortID; };

   static bool     AccountLogged( const CString &csAccountName, LPCSTR szIP );
   /** Reserve une ligne OnlineUsers apres mot de passe OK (pas avant). */
   static bool     ClaimOnlineSlot( const CString &csAccountName, LPCSTR szIP );
   /** DELETE synchrone OnlineUsers (reconnexion Linux). */
   static bool     DeleteOnlineUserSync( const CString &csAccountName );
   static BOOL     FetchAuthPassword( const CString &account, char *pwd, int pwdLen );
   static int 		IPLogged( LPCSTR szIP );
   static void     AccountLoggonFailed( const CString &csAccountName );
   static bool     NameExists( const string &name );	

   void            TogglePage( bool newState ){pages = newState;}

   bool            PageState( void )
   {
      return pages;
   }

   void SetFullPVP( bool state ){
      boFullPVP = state;
   }

   bool IsFullPVP( void ){
      return boFullPVP;
   }

   void SetSanction( int iValA, int iValB)
   {
      dwNbrSanctionA = iValA;
      dwNbrSanctionB = iValB;
   }
   bool AddSanction( BOOL bGod )
   {
      time_t TimeMsTmp =  time(NULL);

      //on set le expire for this sanction 
      dwLastSanctionTS = (DWORD)(TimeMsTmp+g_iDelayExpireSanction);
      dwNbrSanctionA++;
      if(dwNbrSanctionA == g_iNbrSanctionBeforeLock)
      {
         if(bGod)
         {
            dwNbrSanctionA = 0;
            dwNbrSanctionB++;
            return true;
         }
         else
         {
            dwNbrSanctionA--;
            return false;
         }
      }

      return true;
   }

   DWORD GetSanctionA( void ){
      return dwNbrSanctionA;
   }

   DWORD GetSanctionB( void ){
      return dwNbrSanctionB;
   }

   DWORD GetSanctionLastTS( void ){
      return dwLastSanctionTS;
   }

   void SetSanctionLastTS( DWORD dwTS ){
       dwLastSanctionTS = dwTS;
   }

   
   


   void SetNMSGold(int iVal){
      iNMSGold = iVal;
   }
   int GetNMSGold(){
      return iNMSGold;
   }

   bool SetNMSGoldFlag(int iFlag,int iVal);
   int GetNMSGoldFlag(int iFlag);


   void SetNMSGoldSGLv1(int iVal){
      iNMSGoldSGLv1 = iVal;
   }
   int GetNMSGoldSGLv1(){
      return iNMSGoldSGLv1;
   }

   void SetNMSGoldSG(int iVal){
      iNMSGoldSG = iVal;
   }
   int GetNMSGoldSG(){
      return iNMSGoldSG;
   }

   void SetNMSGoldSELv1(int iVal){
      iNMSGoldSELv1 = iVal;
   }
   int GetNMSGoldSELv1(){
      return iNMSGoldSELv1;
   }

   void SetNMSGoldSE(int iVal){
      iNMSGoldSE = iVal;
   }
   int GetNMSGoldSE(){
      return iNMSGoldSE;
   }

   void SetNMSGoldSDLv1(int iVal){
      iNMSGoldSDLv1 = iVal;
   }
   int GetNMSGoldSDLv1(){
      return iNMSGoldSDLv1;
   }

   void SetNMSGoldSD(int iVal){
      iNMSGoldSD = iVal;
   }
   int GetNMSGoldSD(){
      return iNMSGoldSD;
   }

   void SetNMSGoldReroll(int iVal){
      iNMSGoldReroll = iVal;
   }
   int GetNMSGoldReroll(){
      return iNMSGoldReroll;
   }

   void SetNMSGoldToD(int iVal){
      iNMSGoldToD = iVal;
   }
   int GetNMSGoldToD(){
      return iNMSGoldToD;
   }

   void SetNMSGoldToDLv1(int iVal){
      iNMSGoldToDLv1 = iVal;
   }
   int GetNMSGoldToDLv1(){
      return iNMSGoldToDLv1;
   }




   void SetNMSGoldSGLv1_C(int iVal){
      iNMSGoldSGLv1_C = iVal;
   }
   int GetNMSGoldSGLv1_C(){
      return iNMSGoldSGLv1_C;
   }

   void SetNMSGoldSG_C(int iVal){
      iNMSGoldSG_C = iVal;
   }
   int GetNMSGoldSG_C(){
      return iNMSGoldSG_C;
   }

   void SetNMSGoldSELv1_C(int iVal){
      iNMSGoldSELv1_C = iVal;
   }
   int GetNMSGoldSELv1_C(){
      return iNMSGoldSELv1_C;
   }

   void SetNMSGoldSE_C(int iVal){
      iNMSGoldSE_C = iVal;
   }
   int GetNMSGoldSE_C(){
      return iNMSGoldSE_C;
   }

   void SetNMSGoldSDLv1_C(int iVal){
      iNMSGoldSDLv1_C = iVal;
   }
   int GetNMSGoldSDLv1_C(){
      return iNMSGoldSDLv1_C;
   }

   void SetNMSGoldSD_C(int iVal){
      iNMSGoldSD_C = iVal;
   }
   int GetNMSGoldSD_C(){
      return iNMSGoldSD_C;
   }

   void GetLastGetPos(WorldPos &Wl);
   void SetLastGetPos(int x,int y, int w);


   void SetPaidTime(DWORD dwVal){
      dwPaidTimeMax = dwVal;
   }
   DWORD GetPaidTime(){
      return dwPaidTimeMax;
   }

   bool CanPaidPlay();
   bool CanConnectGMOnly();

   

   bool IsLoading();
   void ClearLoadingCount( void ){ loadingCount = 0; }

   // Personnal structures. Old artifacts (aren't accessed through functions ).
   Character      *self;
   sockaddr_in     IPaddrO;
   sockaddr_in     IPaddrI;
   DWORD           dwAntiPlugTime;
   DWORD           dwAntiPlugTimeReload;
   DWORD           dwRegenTime;
   DWORD           dwDeathAndNoXPTime;
   DWORD           dwSPXPScrollTime;
   DWORD           dwSPXPScrollTimeD;
   DWORD           dwSPXPScrollTimeTS;
   DWORD           dwSPORScrollTime;
   DWORD           dwSPORScrollTimeD;
   DWORD           dwSPORScrollTimeTS;
   DWORD           dwKickoutTime;
   DWORD           dwInJailSystemTimeout;

   //ANtiplug Exit Game 
   DWORD dwExitDecompte;
   DWORD dwReloadDecompte;
   /** Dernier paquet recu (time_t) — antiplug ExitGame (>= 13 s). */
   unsigned long ulLastPacketTime;
   // InGame flags
   BOOL in_game;
   BOOL in_AskReloadPlayer;
   BOOL boCanSave;
   /* ExitGame en jeu : demande la sauvegarde dans AsyncDeletePlayer
      (les flags in_game/boPreInGame sont effaces avant le thread de suppression). */
   BOOL boSaveOnDelete;
   BOOL boPreInGame;	  
   BOOL registred;
   BOOL boRerolling;

   // General player flags
   BOOL boCanTalk;
   BOOL boCanShout;
   BOOL boLockedOut;
   BOOL boCanPage;

   BOOL god_mode;
   BOOL god_modeModo;
   BOOL boWhoInvisible;

   //NMNMNM LOCKTMP
   unsigned int uiLockedComplete;
   unsigned int uiTalkIndefinie;
   unsigned int uiShoutIndefinie;
   unsigned int uiPageIndefinie;

   // Idle time checking
   unsigned char IdleChances;
   unsigned int  IdleTime;

   // Mestoph (31/04/2009) : Anti SpeedHack
   int iCountMovement;
   int iSpeedHackTime;
   bool bSpeedHackWarned;

   long m_lKeyCode;
   
   //Nightmare Firewall Variable Utility

   //Status
   UINT m_dwAccessLevel;
   //XpStat 
   UINT    m_dwXPCurrentTick;  
   UINT    m_dwXPLastTickCounter;
   __int64 m_XPCounter;

   //DPS STAT 
   UINT    m_dwDPSCurrentTick;  
   UINT    m_dwDPSLastTickCounter;
   __int64 m_DPSCounter;

   //Speedhack
   UINT    m_dwLastTickMove;
   UINT    m_dwMoveCount;
   CString m_strPseudo;
   //AutoXP Detect
   UINT m_dwLastPlayerAttack;
   UINT m_dwLastPlayerAttackID;
   UINT m_dwAttackCount;
   UINT m_dwAutoXPCount;
   UINT m_dwShowID;
   UINT m_dwLastShowID;
   UINT m_dwSpoofedID;
private:

   bool   m_bSaveInProgress;

   DWORD loadingCount;
   bool  pages;
   bool  boCanPVP;
   bool  boFullPVP;
   int   iNMSGold;

   int   iNMSGoldSGLv1;
   int   iNMSGoldSG;
   int   iNMSGoldSELv1;
   int   iNMSGoldSE;
   int   iNMSGoldReroll;
   int   iNMSGoldSDLv1;
   int   iNMSGoldSD;
   int   iNMSGoldToD;
   int   iNMSGoldToDLv1;

   int   iNMSGoldSGLv1_C;
   int   iNMSGoldSG_C;
   int   iNMSGoldSELv1_C;
   int   iNMSGoldSE_C;
   int   iNMSGoldSDLv1_C;
   int   iNMSGoldSD_C;

   DWORD dwNbrSanctionA;
   DWORD dwNbrSanctionB;
   DWORD dwLastSanctionTS;


   DWORD dwPaidTimeMax;

   DWORD dwRadiusPortID;

   DWORD dwPlayerAcctID;

   DWORD dwNextPageTime;
   DWORD dwNextPageCount;

   DWORD dwNextTalkTime;
   DWORD dwNextTalkCount;

   bool  boIdle;

   DWORD dwNextShout;

   DWORD dwNextSave;
   BOOL  m_bForceSave;

   time_t ttLogTime;

   unsigned __int64 i64GodFlags;

   CString m_strEmpty;
   CString m_Account;
   CString m_Password;
   CString m_Realm;

   CLock m_LockApl;
   std::vector<AccountPlayerInfo>	m_vAPlayerList; 

   


   CLock csUsageLock;

   static DWORD dwAutoSaveFrequency;

   BOOL boDeletionFlag;
};

#endif // !defined(AFX_PLAYERS_H__C2AF9A46_AC25_11D0_9B9E_444553540000__INCLUDED_)
