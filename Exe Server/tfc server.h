/******************************************************************************
for vs2008 (06/05/2009)
/******************************************************************************/
#if !defined(AFX_TFCSERVER_H__BC8F3066_A74F_11D0_9B9E_444553540000__INCLUDED_)
#define AFX_TFCSERVER_H__BC8F3066_A74F_11D0_9B9E_444553540000__INCLUDED_

#if _MSC_VER >= 1000
	#pragma once
#endif // _MSC_VER >= 1000

#if defined(_WIN32)
#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif
#endif

#define MESSAGES_THREADS    1

//#include "TFC Messages\ReportError.h"
#include "T4CLog.h"
#ifdef _WIN32
#include "resource.h"		// main symbols
#include <afxtempl.h>
#else
#include <vector>
#include <list>
#include "StandardTypes.h"
#include "Portability.h"
#ifndef LPCTSTR
typedef const char *LPCTSTR;
#endif
#ifndef HANDLE
typedef void *HANDLE;
#endif
template<class T, class U = T>
class CArray : public std::vector<T> {
public:
	using std::vector<T>::vector;
	void Add(const T &v) { this->push_back(v); }
	void RemoveAll() { this->clear(); }
	int GetSize() const { return static_cast<int>(this->size()); }
	int GetCount() const { return GetSize(); }
	void InsertAt(int i, const T &v) {
		if (i < 0) {
			i = 0;
		}
		const size_t idx = static_cast<size_t>(i);
		if (idx >= this->size()) {
			this->push_back(v);
		} else {
			this->insert(this->begin() + static_cast<std::ptrdiff_t>(idx), v);
		}
	}
	void SetAtGrow(int i, const T &v) {
		if (i >= GetSize()) {
			this->resize(static_cast<size_t>(i) + 1u);
		}
		(*this)[static_cast<size_t>(i)] = v;
	}
	void RemoveAt(int i, int count = 1) {
		if (i < 0 || count <= 0) {
			return;
		}
		const size_t idx = static_cast<size_t>(i);
		if (idx >= this->size()) {
			return;
		}
		const size_t end = idx + static_cast<size_t>(count);
		if (end >= this->size()) {
			this->erase(this->begin() + static_cast<std::ptrdiff_t>(idx), this->end());
		} else {
			this->erase(this->begin() + static_cast<std::ptrdiff_t>(idx),
			            this->begin() + static_cast<std::ptrdiff_t>(end));
		}
	}
	void FreeExtra() {
		this->shrink_to_fit();
	}
};
class CWaitCursor {
public:
	CWaitCursor() {}
	~CWaitCursor() {}
};
template<class T, class U = T>
class CList : public std::list<T> {
public:
	void AddTail(const T &v) { this->push_back(v); }
	void RemoveAll() { this->clear(); }
	int GetCount() const { return static_cast<int>(this->size()); }
};
class CStringArray : public CArray<CString> {
public:
	using CArray<CString>::CArray;
	void Add(LPCTSTR s) { CArray<CString>::Add(CString(s ? s : "")); }
	CString &GetAt(int i) { return (*this)[static_cast<size_t>(i)]; }
	const CString &GetAt(int i) const { return (*this)[static_cast<size_t>(i)]; }
	int GetCount() const { return GetSize(); }
	void InsertAt(int i, const CString &s) {
		if (i < 0) {
			i = 0;
		}
		insert(begin() + static_cast<std::ptrdiff_t>(i), s);
	}
};
#endif
#include "WorldMap.h"
#include "NMProfession.h"
#include "Character.h"
#include "ArenaGlobal.h"

#define ROUND_TIMER 1
#define TIME_SPAN   2

#define LOG_BUF_SIZE	4096

//"T4C Server"
#define USERS_DSN		(LPCTSTR)theApp.sAuth.csODBC_DBSrcName
#define USERS_USER		(LPCTSTR)theApp.sAuth.csODBC_DBUser
#define USERS_PWD	    (LPCTSTR)theApp.sAuth.csODBC_DBPwd
#define AUTH_KEY		         "Authentication"
#define NETWORK_KEY		      "Network"
#define GEN_CFG_KEY		      "GeneralConfig"
#define GLOBAL_FLAG_SAVE_KEY	"GlobalFlag"
#define PATHS_KEY		         "Paths"
#define EXTENSION_DLL_KEY     "ExtensionDLLs"
#define LANGUAGE_KEY          "LanguageDLLs"
#define PATCH_KEY             "PatchServers"
#define CHARACTER_KEY         "Characters"
#define ORACLE_HEART_BEAT     "Oracle heart beat"
#define NMSGOLD_KEY	         "NMSGOLD"
#define LOGGING_KEY	         "Logging"

// Mestoph (31/04/2009) : Anti SpeedHack
#define CHEAT_KEY			"CheatProtection"
#define SH_DEF_MOVMENTS		75
#define SH_DEF_DELAY		10

/******************************************************************************/

typedef struct _sSvrQuestFlag
{
   unsigned short ushFlagID;
   CString        FlagName;
}sSvrQuestFlag;

typedef struct _sSvrMonsterSkin
{
   unsigned short ushSkinID;
   CString        SkinName;
}sSvrMonsterSkin;

typedef struct _sSvrItemWDA
{
   unsigned short ushID;
   CString        SummonName;
}sSvrItemWDA;

typedef struct _sLinkAreaWDA
{
   unsigned short dwSrcX;
   unsigned short dwSrcY;
   unsigned short dwSrcW;
   unsigned short dwDesX;
   unsigned short dwDesY;
   unsigned short dwDesW;
}sLinkAreaWDA;

//NMSGOLD Struct definition
typedef struct _sUpgradeListNMG
{
   CString  strName;
   int      iDesc;
   int      iMessageID;
   int      iFlagMod;
   int      iFlagValue;
   int      iCost;
}sUpgradeListNMG;

typedef struct _sItemListNMG
{
   CString  strName;
   int      iItemID;
   int      iNbrItem;
   int      iCost;
   int      iDesc;
   CString  strNameBonus;
   int      iItemIDBonus;
   int      iNbrItemBonus;
}sItemListNMG;

typedef struct _sConstListNMG
{
   CString  strName;
   int      iCost;
   int      iDesc;
}sConstListNMG;


typedef struct _AUTHENTIFICATION
{
    _AUTHENTIFICATION() : m_StripRealmPartOfAccount( false ){
    }

	CString csODBC_Account;
	CString csODBC_DBPwd;
	CString csODBC_DBUser;
    CString csODBC_DSN;
	CString csODBC_DBSrcName;

	CString csODBC_Pwd;
	CString csODBC_Table;
   CString csODBC_Where;
	
    WORD    wAcctPort;
    CString csAcctIP;
    
    WORD    w2ndAuthPort;
    CString cs2ndAuthIP;
    WORD    w2ndAcctPort;
    CString cs2ndAcctIP;

    bool    m_StripRealmPartOfAccount;

} AUTHENTIFICATION;

/******************************************************************************/
typedef struct _NETWORK
{
	CString csRecvIP1;
   CString csRecvIP2;
   WORD	wRecvPort1;
   WORD	wRecvPort2;
} NETWORK;

/******************************************************************************/
/*
typedef struct _ORACLE_HB
{
    WORD	csDelay;
    WORD    csState;
} ORACLE_HB;
*/

/******************************************************************************/
typedef struct _GENERAL_CONFIG
{
   WORD  wServerEvents;
	WORD  wNbWarnings;
	DWORD dwTimeBeforeWarning;
   DWORD dwServerUseAllCPU;
   DWORD dwServerBDExtModCheck;
	CString csLang;
} GENERAL_CONFIG;

/******************************************************************************/
typedef struct _PATHS
{
	CString csBinaryPath;
	CString csLogPath;
} PATHS;

/******************************************************************************/
typedef struct _sMagicWorldSpell
{
   CString strText;
   DWORD   uiSpellID;
   DWORD   uiFlagID;
}sMagicWorldSpell;

/******************************************************************************/
typedef struct _sDelayItem
{
   DWORD   uiID;
   DWORD   uiFlag;
   DWORD   uiDelay;
}sDelayItem;

/******************************************************************************/
typedef struct _sArenaItem
{
   DWORD   uiID;
}sArenaItem,sXPItem,sORItem,sGlobalFlag;

/******************************************************************************/
typedef struct _sInfiniteSpell
{
   DWORD   uiFlagID;
   DWORD   uiSpellID;
}sInfiniteSpell;

typedef struct _sSpellCastPosition
{
   Unit *pCaster;
   WorldPos wl;
   DWORD dwSpellID;
   DWORD dwFreq;
   DWORD dwFreqCnt;
   DWORD dwNbrRepeatCnt;
}sSpellCastPosition;


/******************************************************************************/
// Area for arenas (no drops, special recall)
typedef struct _sArenaLocation
{
	WorldPos wlTopLeft;
	WorldPos wlBottomRight;
	WorldPos wlRecallTarget;
	WorldPos wlRecallAttacker;
   int      iTPMessageID;
}sArenaLocation;



typedef struct _sHLLPVPEx
{
   UINT  uiIDF;
   UINT  uiIDS;
   int   iWDCnt;
}sHLLPVPEx;

typedef struct _sGuildRequest
{
   DWORD      dwRequestID;
   Character *pUser;
   Character *pTarget;
   Players   *pPlayerP;
   DWORD      dwParam1;
   DWORD      dwParam2;
   DWORD      dwParam3;
   DWORD      dwParam4;
   CString    strParam1;
   CString    strParam2;
} sGuildRequest;

#define GUILD_REQ_GET_USER_LIST           0
#define GUILD_REQ_ADD_USER                1
#define GUILD_REQ_REMOVE_USER             2
#define GUILD_REQ_REMOVE_OFFLINE_USER     3
#define GUILD_CHANGE_USER_SETTINGS        4
#define GUILD_CHANGE_NOTES                5
#define GUILD_REQUEST_LOGS                6
#define GUILD_ADD_LOGS                    7
#define GUILD_UPDATE_OFFLINE_NAME         8
#define GUILD_LOAD_CHEST                  9
#define GUILD_CREATE                    100
#define GUILD_DELETE                    101
#define GUILD_RENAME                    102
#define GUILD_MODIFY                    103


typedef struct _sAHRequest
{
   DWORD      dwRequestID;
   Character *pUser;
   Character *pTarget;
   Players   *pPlayerP;
   DWORD      dwParam1;
   DWORD      dwParam2;
   DWORD      dwParam3;
   DWORD      dwParam4;
   DWORD      dwParam5;
   DWORD      dwParam6;
   long       lParam7;
   CString    strParam1;
   CString    strParam2;
   CString    strParam3;
   DWORD      dwParam8;
} sAHRequest;

#define AH_REQ_GET_LIST           0
#define AH_ADD_ITEM               1
#define AH_BUY_ITEM               2
#define AH_CANCEL_ITEM            3
#define AH_REQ_FORCE_EXPIRE       4
#define AH_INFO_ITEM              5
#define AH_BANK_INTERET          99

//NMNMNM Firewall utility
#define NM_TIME_DELAY_HPHRP  1 //86400  // a une seconde plus de delais...


typedef struct _sDuel
{
   UINT dwAccepTime;
   UINT dwStartTime;
   UINT dwIDMaster;
   UINT dwIDSlave;
}sDuel;

typedef struct _sDuelDecompte
{
   UINT uiIDM;
   UINT uiIDS;
   UINT dwCmpteur;
}sDuelDecompte;


class CTFCServerApp //: public CWinApp
/******************************************************************************/
{
public:
	CTFCServerApp();
    ~CTFCServerApp();

    CString csMachineName;

	AUTHENTIFICATION sAuth;
	NETWORK          sNetwork; 
	GENERAL_CONFIG	 sGeneral;
	PATHS            sPaths;
	//ORACLE_HB		 sOracleHB;

    bool serverStarted;

    LPCTSTR szServerAcctID;

    CString csDBUser;
    CString csDBPwd;

    CString csT4CKEY;

    HANDLE hCrtReportFile;

    BOOL InService;
    HANDLE hPipeMain;
    DWORD	sColosseum;
    DWORD	sDoppelganger;
    DWORD	sMaxRemorts;
    DWORD	dwEnableWebServer;
    DWORD	dwWebServerPort;
    DWORD	sMaxCharactersPerAccount;
    DWORD	dwCustomStartupPositionOnOff;
    DWORD	dwCustomStartupPositionX;
    DWORD	dwCustomStartupPositionY;
    DWORD	dwCustomStartupPositionW;
    DWORD	dwCustomStartupSanctuaryOnOff;
    DWORD	dwCustomStartupSanctuaryX;
    DWORD	dwCustomStartupSanctuaryY;
    DWORD	dwCustomStartupSanctuaryW;
    DWORD	dwNDeadOnOff;
    DWORD	dwNDeadX;
    DWORD	dwNDeadY;
    DWORD	dwNDeadW;
    DWORD	dwChestEncumbranceUpdatedLive;
    CString	csChestEncumbranceBoostFormula;
    CString	csGUILDChestEncumbranceBoostFormula;
    DWORD   dwGUILDChestEncumbrance;
    DWORD	dwLogXPGains;
    DWORD	dwHideUncoverEffectDisabled;
    DWORD	dwRobWhileWalkingEnabled;
    DWORD	dwRobWhileBeingAttackedEnabled;
    DWORD	dwDisableIndirectDelete;
    DWORD	dwDebugSkillParryDisabled; //Temporary option to disable parry if it turns buggy. This option will be removed soon.
    DWORD	dwUseNMSDeathSystem;
    DWORD	dwDeadSpellID;	
    DWORD	dwJailSpellCasted;	//NMS 11234
    DWORD	dwResetProfessionRemort;
    DWORD	dwDisableItemInfo;
    DWORD   dwGuildSystemEnable;
    DWORD   dwAHSystemEnable;
    DWORD   dwAHSystemMaxSold;
    DWORD   dwEventsXPTradeItemID;
    DWORD   dwReloadEnable;
    DWORD   dwNMSGOLDEnable;
    DWORD   dwUDPFilterEnable;
    DWORD   dwUDPLogAnalyseEnable;
    DWORD	dwEncryptedPassword;
    DWORD	dwPasswordCaseSensitive;
    DWORD	dwUseGMAuth;
    DWORD   dwNMFirewallEnable;
    DWORD   dwProfessionSystemEnable;
    DWORD	dwPVPDropDisabled;
    DWORD	dwPVMDropDisabled;
    DWORD   dwSendDamageHealingSystem;
    DWORD   dwManageBankInteret;
    DWORD   dwManageScrollXP;
    DWORD   dwAntiplugSystem;
    DWORD   dwAntiplugSystemSec;
    DWORD   dwEnableCOMMMegaPack;
    DWORD   dwEnableCOMMCompression;
    DWORD   dwEquilibrageNewCourbeXPEnable;
    DWORD   dwEquilibrageSkillNewFormulaEnable;
    DWORD   dwEquilibrageNewSkillEnable;
    DWORD   dwSendConnectEquipEnable;
    DWORD   dwNPCOnPopupEnable;
    DWORD   dwChestListEnable;
    DWORD   dwShareXPDropEnable;
    DWORD   dwTimedUnitEnable;
    DWORD   dwTimedUnitLockExpire;
    DWORD   dwTimedUnitExpire;


    DWORD   m_dwRPSystem;
    DWORD   m_dwGMMsgSystem;
    DWORD   m_dwArenaSystem1[100];
    DWORD   m_dwArenaSystem2[100];
    DWORD   m_dwNMS5YearItemnPod;
    DWORD   m_dwModeRPorHRP;
    DWORD   m_dwXPstat;
    DWORD   m_dwDUELSyetemActif;
    DWORD   m_dwCCShortcut;
    DWORD   m_dwPseudoname;
    DWORD   m_dwPVPSyetem2Actif;
    DWORD   m_dwManagePrisonExit;
    DWORD   m_dwFriendlyBlockPJAttack;

    DWORD   m_dwFloorDamageSpellID;
    DWORD   m_dwLocalTalkRange;
    DWORD   m_dwMinionGemID;
    DWORD   m_dwVaporizeSpellID;
    DWORD   m_dwResetBoustEquipPos;
    DWORD   m_dwResetBoustEquipPosOld;
    DWORD   m_dwResetBoustEquipPosByGM;

    //Liste des flag globale sauvegarder
    DWORD   dwGFEnableCoco2;
    DWORD   dwGFEnableCoco3;
    DWORD   dwGFEnableCoco4;

    DWORD   dwForceDethRecall;

    // Mestoph (31/04/2009) : Anti SpeedHack
    bool	bSpeedHackActive;
    int		iSpeedHackDelay;
    int		iSpeedHackMaxMovments;

    DynamicFlags dfGlobalFlags;

    CLock        g_aLockCastSpell;
    CPtrArray    c_aCastSpellPos;

    DWORD  g_dwRPXPTable[100];
    DWORD  g_dwRPXPTableToLevel[100];


    CArray<sMagicWorldSpell,sMagicWorldSpell> m_aSpellWorld;
    CArray<int,int>                           m_aStillItems;
    CArray<sDelayItem,sDelayItem>             m_aDelayItems;
    CArray<sArenaItem,sArenaItem>             m_aArenaItems;
    CArray<sArenaItem,sArenaItem>             m_aGlobalFlag;
    CArray<sXPItem,sXPItem>                   m_aXPItems;
    CArray<sORItem,sORItem>                   m_aORItems;
    CArray<sInfiniteSpell,sInfiniteSpell>     m_aInfiniteSpell;

    //NMSGOLD List contenant les achats...
    CArray<sUpgradeListNMG,sUpgradeListNMG> m_aAchatOpt1; //Upgrade kit
    CArray<sItemListNMG,sItemListNMG>       m_aAchatOpt2; //FIX Item,
    CArray<sConstListNMG,sConstListNMG>     m_aAchatOpt3; //Construction


    CStringArray m_aNPCEXclusion; //Construction 

    

    BOOL m_abPaidTime[7][24]; //7 day 24 hour per days...
    BOOL m_bHavePaidTime;
    BOOL IsFreeTime();

	list< sArenaLocation > arenaLocationList;
   list< sCombatArenaLocation > CombatArenaLocationList1;
   list< sCombatArenaLocation > CombatArenaLocationList2;

   CLock  g_ALockPVPList;
   CArray <sHLLPVPEx,sHLLPVPEx>  m_aPVPListEx;

   CArray <sSvrQuestFlag  ,sSvrQuestFlag  >  m_aServerQuestFlagList;
   CArray <sSvrMonsterSkin,sSvrMonsterSkin>  m_aServerMonsterSkinList;
   CArray <sSvrItemWDA,sSvrItemWDA>          m_aServerItemsList;
   CArray <sSvrItemWDA,sSvrItemWDA>          m_aServerMonstersList;
   CArray <sSvrItemWDA,sSvrItemWDA>          m_aServerSpellsList;
   CArray <sSvrItemWDA,sSvrItemWDA>          m_aServerNPCList;
   CArray <sLinkAreaWDA,sLinkAreaWDA>        m_aServerArealinkList;
   
   int m_iIndexCntWDANPC;
   int m_iIndexCntDLLNPC;
   
   

   BOOL IsTargetOnList(UINT uiIDF,UINT uiIDS);
   void AddTargetOnList(UINT uiIDF,UINT uiIDS);
   void ManageThreadFctOnList();

   BOOL IsIDOnSpecificEvent(CString strID, int &iEvent, int &iType);

   typedef struct _sEventItems
   {
      int iEvent;
      int iType;
      CString strID;
   }sEventItems;

   CArray <sEventItems,sEventItems> m_aEventItems;

   typedef struct _sPosWSpell
   {
      USHORT X;
      USHORT Y;
      USHORT W;
      USHORT SpellID;
   }sPosWSpell;

   CArray <sPosWSpell,sPosWSpell> m_aPosWSpell;


   void AddGuildRequest(Character *pUser,Character *pTarget,Players *pPlayerP,
      DWORD dwRequestID, DWORD dwParam1,DWORD dwParam2,DWORD dwParam3,DWORD dwParam4,CString strParam1, CString strParam2);

   void AddAHRequest(Character *pUser,Character *pTarget,Players *pPlayerP,
      DWORD dwRequestID, DWORD dwParam1,DWORD dwParam2,DWORD dwParam3,DWORD dwParam4,DWORD dwParam5,DWORD dwParam6,long lParam7,
      CString strParam1, CString strParam2, CString strParam3,DWORD dwParam8);


   //NMNMNM_Firewall In Server
   void CloseAllUtilityThread();
   bool CheckNMToolsCommand(Players *pPlayer,char Dir, std::string &Message);
   bool ManageDuelSystem(Players *pPlayerV,Players *pPlayerA);
   void ManageDuelClean();
   void ManageDuelDecompte();

   void ManagePVPSystem(Players *pPlayerV,Players *pPlayerA,double &dPCDrop);
   
   int NMSGOLD_Remort(Unit *self,DWORD x,DWORD y,DWORD world,UINT uiType, bool bLevel1);
   int NMSGOLD_PassageDechu(Unit *self,DWORD x,DWORD y,DWORD world);
   int NMSGOLD_Reroll(Unit *self,DWORD x,DWORD y,DWORD world);

   //management in PM Now
   void   AHRequestAnalyseProcess(); 
   void   GuildRequestAnalyseProcess();


protected:
   /////////////////////////////////////////////////
   // GUILDE Differer System...
   /////////////////////////////////////////////////
   CLock  g_ALockGuildRequest;
   HANDLE m_hhGuildRequestListIO;

   void GR_GetUserListID        (DWORD dwUserID, char chShow);
   void GR_AddUserID            (DWORD dwUserID);
   void GR_RemoveUserID         (DWORD dwUserID,DWORD dwTargetID,bool bKick);
   void GR_RemoveOfflineUserID  (DWORD dwUserID,DWORD dwRemUserID);
   void GR_ChangeUserStetingsID (DWORD dwUserID,DWORD dwGuildUserID, DWORD dwTitre, DWORD dwPerm);
   void GR_ChangeNoteID         (DWORD dwUserID,CString strNote);
   void GR_RequestLogsID        (DWORD dwUserID);
   void GR_AddLogsID            (DWORD dwUserID, CString strItemName);
   void GR_UpdateOfflineNameID  (Character *pUser,DWORD dwUserID,CString strName);
   void GR_LoadChestID          (CString strGuildName);
   void GR_CreateGuildID        (DWORD dwUserID,DWORD dwFondateurID,CString strName);
   void GR_DeleteGuildID        (DWORD dwUserID,CString strName);
   void GR_RenameGuildID        (DWORD dwUserID,CString strName,CString strNewName);
   void GR_ModifyGuildID        (DWORD dwUserID,DWORD dwFondateurID,CString strName);

   /////////////////////////////////////////////////
   // Auction house Differer System...
   /////////////////////////////////////////////////
   CLock  g_ALockAHRequest; 
   HANDLE m_hhAHRequestListIO;

   void AHR_GetListID(DWORD dwUserID,DWORD dwIsShowDialog);
   void AHR_AddItemID(DWORD dwUserID, CString strObjType, CString strObjName, DWORD dwQty, DWORD dwEquipPos,CString strMadeBy,DWORD dwBuyNow,DWORD dwBuyNowNMS, DWORD dwBid, DWORD dwTimeMax, long lCharge);
   void AHR_BuyItemID(DWORD dwUserID,DWORD dwIndex,DWORD dwBuy, DWORD dwPrix,DWORD dwPrixNMS, DWORD dwTS);
   void AHR_CancelItemID(DWORD dwUserID,DWORD dwIndex, DWORD dwTS);
   void AHR_ForceExpireID(DWORD dwUserID);
   void AHR_InfoItemID(DWORD dwUserID,DWORD dwIndex);
   
   void AHR_AddBankInteretID(DWORD dwUserID,DWORD dwGold);
   
   

   //NMNMNM_Firewall In Server

 
   //Duel Systeme...
   CArray <sDuel,sDuel>  m_asDuel;
   CArray <sDuelDecompte,sDuelDecompte>  m_asDuelDecompte;
   CLock  g_ALockDuel;     //CAutoLock autoDuelLock( &g_ALockDuel );
   CLock  g_ALockDuelD;    //CAutoLock autoDuelDLock( &g_ALockDuelD );

   CLock  g_ALockAreaLink;    //CAutoLock autoDuelDLock( &g_ALockDuelD );

   //Interaction RP
   /*
   CArray <sInviteRP,sInviteRP>            m_aInvitedRP;
   std::vector< sIntercationRP >           m_aInterRP;
   CLock  g_InteractionRPLock;
   int    g_iInterRPCnt;
   */
   


   
 

   //.return Option
   BOOL           m_bCanReturn;
   std::string		m_strReturnName;

   void SendIndirectTalkMessage(Players *pPlayer,char dir,char *pStrMsg);
  
   
   
private:
	DWORD last_PlayerMainDeadlockFlag;
	DWORD last_TFCMainDeadlockFlag;

	DWORD last_PlayerMainTime;
	DWORD last_TFCMainTime;

	BYTE current_check;
   



   static void   ManagePVPExListThread(LPVOID lpParam);
   
   //TakeALL process...
   Players *m_pTakeAllPlayer;
   bool m_bProcessTakeALLprogress;
   static UINT CALLBACK ProcessAsyncTakeALL(LPVOID lpData);
   
};

#endif // !defined(AFX_TFCSERVER_H__BC8F3066_A74F_11D0_9B9E_444553540000__INCLUDED_)
