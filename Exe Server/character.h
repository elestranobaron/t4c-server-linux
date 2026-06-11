/******************************************************************************
Modify for vs2008 (30/04/2009)
Add Profession, AH, etc etc etc by Nightmare (29/06/2009)
/******************************************************************************/
#if !defined(AFX_CHARACTER_H__6EB985F4_D137_11D0_B543_C09D9B000000__INCLUDED_)
#define AFX_CHARACTER_H__6EB985F4_D137_11D0_B543_C09D9B000000__INCLUDED_

#if _MSC_VER >= 1000
	#pragma once
#endif // _MSC_VER >= 1000
#include "StdAfx.h"
#include "Objects.h"
#include "SharedStructures.h"
#include "TFCPacket.h"
#include "Directions.h"
#include "Unit.h"
#include "Skills.h"
#include "Professions.h"
#include "QuestBook.h"
#include "_item.h"
#include "Group.h"
#include "SendPacketVisitor.h"
#include "ItemContainer.h"
#include "Trade.h"
#include "Minions.h"

#define DEATH_EFFECT_ID     30012

// These are the database row IDs. The IDs MUST be SORTED and ORDERED
#define DB_ID					1
#define DB_AccountName			2
#define DB_wlX					3
#define DB_wlY					4
#define DB_wlWorld				5
#define DB_nClass				6
#define DB_CurrentHP			7
#define DB_MaxHP				8
#define DB_CurrentMana			9
#define DB_MaxMana				10
#define DB_Strength				11
#define DB_Endurance			12
#define DB_Agility				13
#define DB_Intelligence			14
//#define DB_WillPower			15
#define DB_Wisdom				15
#define DB_Level				16
#define DB_AttackSkill			17
#define DB_DodgeSkill			18
#define DB_Gold					19
#define DB_Appearance			20
#define DB_Corpse				21
#define DB_XP					22
#define DB_StatPnts				23
#define DB_SkillPnts			24
#define DB_Karma                25
#define DB_Gender               26
#define DB_ListingTitle         27
#define DB_ListingMisc          28
#define DB_MoveExhaust          29
#define DB_MentalExhaust        30
#define DB_AttackExhaust        31
#define DB_Crime                32
#define DB_Honor                33
#define DB_Luck				     34
#define DB_MinionName   	     35
#define DB_MinionID     	     36
#define DB_RpXP                 37
#define DB_RpXPPoint            38

#define NB_CLASS				4
#define MAX_LEVEL_XP		   		260
#ifdef BUILD_NMS_CUSTOM_NPC //ok
   #define MAX_LEVEL_CAN_HAVE	   	240
#else
   #define MAX_LEVEL_CAN_HAVE	   	250
#endif

#define EQUIP_POSITIONS			17

#define GENDER_MALE				0
#define GENDER_FEMALE			1

#define DISTURB_CLOSECHEST		0x01
#define DISTURB_CLOSETRADE		0x02
#define DISTURB_UNHIDE			0x04
#define DISTURB_DONTCANCELROB	0x08

typedef struct _LOADED_ITEM
{
   BYTE	bEquipPos;
   DWORD	dwObjID;
   DWORD   dwQty;
   BYTE	lpszObjType[ 50 ];
   BYTE	lpszMadeBy[ 50 ];
} LOADED_ITEM;

typedef struct _EXTBDCheck
{
   WORD wStr;
   WORD wEnd;
   WORD wAgi;
   WORD wInt;
   WORD wWis;
   WORD wStP;
   WORD wSkP;
   WORD wPadding;
   double dXP;
   DWORD dwMaxHP;
   DWORD dwMaxMP;
   int  iGold;
   int  iNbrItem;
   int  iNbrChestItem;
   int  iNbrSpell;
   
} EXTBDCheck;




class Players;
class AutoArrowRemove;
/******************************************************************************/
// A visitor used by some packet sending functions to query wether the packet
// can be sent to the player or not.
/******************************************************************************/
class __declspec(dllexport) Character : public Unit
/******************************************************************************/
{
public:
	Character();	

	enum BodyPos
	{
		body		 = 0,// puppet
		feet		 = 1,// puppet
		gloves		 = 2,// puppet
		helm		 = 3,// puppet
		legs		 = 4,// puppet
		rings		 = 5,
		bracelets    = 6,
		necklace	 = 7,
		weapon_right = 8,// puppet
		weapon_left  = 9,// puppet
		two_hands    = 10,
		ring1        = 11,
		ring2        = 12,
		weapon       = 13,
		belt		 = 14,
		cape		 = 15,
		Orbe1        = 16,
	};
	class ErrorCodes 
	{
	public:
		enum TakeFromBackpack 
		{	
			ObjectTakenFromBackpack,
			ObjectCantBeDropped,
			ObjectNotFoundOnBackpack
		};
	};
	BOOL roll_stats();
   
   static void WaitForAsyncSaveFuncBD( void );
   static void InitializeODBC( void );
   static void DestroyODBC( void );
   
   void	  SetPlayer( Players *Player);
   char      load_character(CString name, CString account, LPBYTE lpbAnswers );
   char      DeleteCharacter(CString name, CString account, BOOL Report = TRUE);
   char	  PutPlayerInGame( void );
   int     PutItemToChest(Objects *pObj);
   
   static bool IsNameValid( CString &name );
   WorldPos  teleport_character(WorldPos where);	
   
   void      GetUnit( WorldPos originalPos, Unit *obj, bool bUpdateBackpack );
   Unit *    DropUnit( WorldPos where, DWORD itemID, DWORD qty );
   
   void JunkItems( DWORD id, DWORD qty, bool gameop );
   int  PutItemToAH(DWORD id,DWORD qty,CString &ObjType,CString &ObjName,DWORD &dwEquipedPos, CString &MadeBy,long &lCharge);


   void  LoadEquipedFromID(int *piEquiped, DWORD dwID);
   void  LoadHairColorFromID(USHORT &ushHairColor, DWORD dwID);

	ItemContainer* GetChest();
	void SetChest(ItemContainer *newChest);
	void MoveObjectFromBackpackToChest2( DWORD dwObjectID, DWORD dwQty );
	void MoveObjectFromChestToBackpack2( DWORD dwObjectID, DWORD dwQty );
	void ShowChest( void ); // Asks the client to show the chest interface
    void ShowGuildChest( void ); // Asks the client to show the Guildchest interface
	void StopUsingChest( void );
    void StopUsingGuildChest( void );
	void SendChestContentPacket(void); //Send the character the chest packet! :)
    void SendGuildChestPacket(BOOL bLock = TRUE);

	TradeMgr2* GetTradeMgr2(); // Returns a pointer to the trade manager;
	void TradeRequest(Character *invitedCharacter); // Invites or accept a trade
	void TradeCancel(); // Cancels the trade or invite
	void TradeFinish(); // Finish the trade
	void TradeSetStatus(TradeMgr2::Status::CharacterStatus newStatus); // Changes the status of the character within the trade
	void TradeAddItemFromBackpack(DWORD itemID, DWORD itemQty); // Add an item from the backpack to the trade
	void TradeRemoveItemToBackpack(DWORD itemID, DWORD itemQty); // Removes an item from the trade to the backpack
	void TradeClearItemsFromTrade(); // Removes all items from the trade back to the backpack

	unsigned long god_create_object(unsigned short which_item);

	char equip_object(unsigned long item_nb,bool gameop = false);
	char unequip_object(unsigned char from_where, bool remove = false, bool gameop = false );
	
	BOOL QueryEquip( Unit *lpuEquip, BYTE bItemType, BYTE bEquipPos, _item *item );

	void packet_equiped(TFCPacket &packet, bool gameop = false);
	void packet_stats(TFCPacket &packet, bool gameop = false);
	void PacketSkills (TFCPacket &sending,bool gameop = false );
	void PacketSpells (TFCPacket &sending,BYTE bUpdate, bool gameop = false );
	void PacketStatus (TFCPacket &sending,bool gameop = false );
   void PacketStatus2(TFCPacket &sending,bool gameop = false );

   void SendSkill(TemplateList <USER_SKILL> &tlSentSkills,short shSkillID,TFCPacket &sending);

	BOOL      can_get(WorldPos where, Objects *obj);	

	unsigned short memorized_spell[9];
	char memorize_spell(unsigned char where, unsigned short which_spell, unsigned long itemID);

	void use_item(unsigned long itemID, Unit *TargetPlayer);
	void use_item2(unsigned long itemID, Unit *TargetPlayer);

	int attack(LPATTACK_STRUCTURE strike,   Unit *Target);
	int attacked(LPATTACK_STRUCTURE strike, Unit *Mechant);
	int hit(LPATTACK_STRUCTURE strike, Unit *WhoHit);
	int attack_hit(LPATTACK_STRUCTURE s_asBlow, Unit *lpuTarget);
	void Death( LPATTACK_STRUCTURE lpBlow, Unit *lpKiller );
   void DeathOld(Unit *lpKiller );
   void DeathNMS(Unit *lpKiller );
   void NMResurect(BOOL bForceResurect = FALSE);
   void NMGetProfession();
   void NMMakeFormule(USHORT ushID);

   void NMGetGuildList(BYTE chShow);

   void NMCombatMode(int iAttackMode,bool bSilent = false);
   char GetNMCombatMode();

   void NMModeRPPhaseID(int iRPPhase);
   int  GetNMModeRPPhaseID();
   int  GetNMModeRPPhaseCntTalk();
   int  GetNMModeRPPhaseCntNOTTalk();
   void ResetNMModeRPPhaseCntTalk(){m_iRPTalkCnt = 0;};
   void ResetNMModeRPPhaseCntNOTTalk(){m_iRPNOTTalkCnt = 0;};
   void StpeNMModeRPPhaseCntNOTTalk(){if(m_iRPNOTTalkCnt<100) m_iRPNOTTalkCnt++;};

   void  SetArenaID(int iID);
   int   GetArenaID();
   void  SetArenaType(int iType);
   int   GetArenaType();
   void  SetArenaKill(int iVal);
   int   GetArenaKill();
   void  SetArenaDead(int iVal);
   int   GetArenaDead();
   void  SetArenaFlag(int iVal);
   int   GetArenaFlag();
   void  SetArenaTeam(int iVal);
   int   GetArenaTeam();
   void  SetArenaLastStart(DWORD dwVal);
   DWORD GetArenaLastStart();

   void   AddArenaPVP(double dVal);
   void   SetArenaPVP(double dVal);
   double GetArenaPVP();
   void  AddArenaINACTIF();
   void  ResetArenaINACTIF();
   void  ResetArenaINACTIFStart();
   int   GetArenaINACTIF();
   void  AddArenaDUREE();
   void  ResetArenaDUREE();
   int   GetArenaDUREE();
   void  AddArenaPOINTS(int dwVal,CString strReason);
   void  SetArenaPOINTS(int dwVal);
   void  ResetArenaPOINTS(CString strReason);
   int   GetArenaPOINTS();



   void StartMove(short shDirection);
   void StartGetObject(WorldPos wlPos,DWORD dwID);
   void StartPutPlayerInGame();


   void StartViewBackpack2(bool bAll, char chShow);
   void StartAsyncEquipItem(unsigned long ulItemID);
   void StartAsyncDirectTalk(CString strMessage,BYTE chDir, DWORD dwColor, DWORD dwID,WorldPos wlWhere);
   void StartAsyncDirectTalkNoFeed(CString strMessage,BYTE chDir, DWORD dwColor, DWORD dwID,WorldPos wlWhere);
   void StartAsyncPageTalk(CString strName,CString strMsg);
   void StartAsyncIndirectTalk(CString strMessage,BYTE chDir, DWORD dwColor, DWORD dwID);
   void StartAsyncFromPregameToGame();
 
  
   void NeedUpdateGold();
	int GetGold();
	void SetGold(int newGold, BOOL boEcho = TRUE );
	
//	void save_character();
	void reset_character(bool bValidGuildChest = false);

	WORD GetCorpse();
	BOOL Kill();
	
	void SetXP(__int64 XP,bool bforce=false);
	__int64 GetXP();
   
   void SetXPScrollBonnus(__int64 XP);
   void AddXPScrollBonnus(__int64 XP);
   __int64 GetXPScrollBonnus();

   void SetORScrollBonnus(__int64 iOr);
   void AddORScrollBonnus(__int64 iOr);
   __int64 GetORScrollBonnus();

   void AddYoursHealingValue(int iVal);
   void ProcessYoursHealingDisplay();

   //void SetTarget(Unit *newTarget);


   


	TemplateList <Unit> *GetBackpack();
	void SetBackpack(TemplateList <Unit> *list);

	WorldPos MoveUnit(DIR::MOVE where, BOOL boAbsolute, bool boCompressMove, bool boBroadcastMove );

	WORD GetClan();
	void SetClan(WORD newClan);

	DWORD GetTrueMaxHP();
	void  SetMaxHP(DWORD newMax);

	DWORD GetHP();
	void  SetHP(DWORD newMax, bool boUpdate );

	WORD  GetTrueMaxMana();
	void  SetMaxMana(WORD newMax);
	
	WORD GetMana();
	void SetMana(WORD newMax, BOOL boEcho = TRUE );

	EXHAUST GetExhaust();
	void SetExhaust(EXHAUST newExhaust);

   unsigned long GetLastMoveTime();
   void SetLastMoveTime();
   

	void	SetName(CString name);
	CString GetName( WORD wLang );
	CString GetName();
    void    SetPseudoName( CString csName );
    CString GetTrueName();

	BOOL SaveCharacter(BOOL bFORCE,char *pstrFromWho, BOOL boCallback = TRUE);
	int LoadCharacter(CString csName);

	Unit **GetEquipment( void );		

	void CheckIFLevelUP();

	Players *GetPlayer();
	inline unsigned __int64 GetGodFlags();

	BOOL UseSkillPnts( WORD bQuantity );
	BOOL UseStatPnts( WORD wQuantity );

	BOOL TrainSkillPnt( int nID, int nQuantity, WORD wMax );

	static void InitXPchart();

	BOOL SendUnitMessage( UINT MessageID, Unit *self, Unit *medium, Unit *target, void *INparameters, void *OUTparameters );

	void SendPlayerMessage( TFCPacket &sending );
   void SendPlayerXP(bool bForceUpdate);
   void SendIsXP();

	inline BOOL UseSkill( int nID, Unit *uTarget, LPVOID lpValueOUT );
	inline BOOL UseSkill( int nID, WorldPos wlPos );

	BOOL CastSpell( WORD wSpellID, Unit *uTarget );
   BOOL CastSpellNoCheck( WORD wSpellID, Unit *uTarget );
   BOOL CastSpellNoCheckFull( WORD wSpellID, Unit *uTarget );
	BOOL CastSpellDirect( WORD wSpellID, Unit *uTarget );
	BOOL CastSpell( WORD wSpellID, WorldPos wlPos );
   BOOL CastSpellPosSystem( WORD wSpellID, WorldPos wlPos );

	inline BOOL UseSpellEnergy( WORD wEnergy );

	void Regenerate( void );
    void ProcessNMDeath( void );
    void InfoScrollXP( char chStatus ,unsigned short ushDelayMin = 0);
    void InfoScrollOR( char chStatus ,unsigned short ushDelayMin = 0);

    LPUSER_SKILL LearnSkill( DWORD dwSkill, WORD wInitialStrength, bool boEcho, CString &errMsg );

	LPUSER_SKILL GetSkill(DWORD dwSkill);

    LPUSER_PROFESSION_F LearnProfessionFormule(DWORD dwFormuleID,DWORD dwCost,bool boEcho,CString &errMsg);
    BOOL UnLearnProfessionFormule(DWORD dwFormuleID,CString &errMsg);

	TemplateList<USER_SKILL> *GetSkillsList( void );
	TemplateList<USER_SKILL> *GetSpells( void );
    TemplateList<USER_PROFESSION_F> *GetProfession( void );

	static __int64 Character::sm_n64XPchart[MAX_LEVEL_XP];
    static __int64 Character::sm_n64XPchartOld[MAX_LEVEL_XP];

	WORD GetSkillPoints();
	WORD GetStatPoints();
    void SetStatPoints( WORD w )
	{
        wNbStatPnts = w;
    }
	void GiveSkillPoints( WORD wQuantity );

	double GetAC( void );
    double GetTrueAC( void );

	inline char GetAgressivness( void );
	inline void SetAgressivness( char cAgr );

	BOOL CanAttack( void );

	void WaitForSaving( void );

    BOOL CanEquip( Unit *lpuUnit, _item *lpProvidedItem = NULL, BOOL boEcho = TRUE, CString *reqText = NULL );

    inline WORD GetLang( void ) const;

	int  GetWeight( void );
    int  GetMaxWeight( void );
	int  GetFreeWeight( void );

    void SendPrivateMessage( CString &csMessage, Unit *lpuUnit, DWORD dwColor );

    __int64 NextLevelXP( void );
    __int64 PreviousLevelXP( void );
    __int64 XPtoLevel( void );

    void PacketPuppetInfo( TFCPacket &sending );

    void Disturbed( WORD pTriggers = 0 ); //Called when the unit gets disturbed. Parameter are which triggers will be triggered by the call

    bool PreTranslateInGameMessage( CString csText );

    static cODBCMage *GetODBC();

    bool UseItemByAppearance( WORD wAppearance, Unit *TargetPlayer );

    // Sets a character's group.    
    void SetGroup( Group *lpGroup );

    // Gets a character's group.
    Group *GetGroup( void );

    void VaporizeUnit( bool bLog = true );

    CString GetTitle( bool getAccountName );
    void    SetTitle( CString csNewTitle ){ csListingTitle = csNewTitle; csListingTitle.Remove('\''); };

    CString GetListingMiscDesc( void )             { return csListingMisc; };
    void    SetListingMiscDesc( CString csNewMisc ){ csListingMisc = csNewMisc; csListingMisc.Remove('\''); };
    
	// Guild stuff
	int		GetGuildPoints() const { return GuildPoints; };
	void	SetGuildPoints( int NewGuildPoints ) { GuildPoints = NewGuildPoints; };

	unsigned char GetGuildRank() const { return GuildRank; };
	void	SetGuildRank( unsigned char NewGuildRank ) { GuildRank = NewGuildRank; };

	//Guilds *GetGuild() const { return GuildRef; }
	//void	SetGuild(Guilds *NewGuild) { GuildRef = NewGuild; }

    void    CopySpells( Unit *lpSource );

	struct Attack
	{
		friend Character;
		enum AttackType{ 
			spell, 
			normal,
			range
		};

        Attack( AttackType newAttackType, DWORD newSpellID ) : attackType( newAttackType ), spellID( newSpellID )
		{}
	
	private:
		AttackType attackType;        
		DWORD spellID;
	};

    void    StartAutoCombat( Attack attack, Unit *target );
    void    StopAutoCombat();
    bool    QueryAutoCombatState( Attack *attack = NULL );
    bool    ExecAutoCombat( void );
    void    RestorePreviousAutoCombatState();

    void    SetGameOpContext( Character *newContext )
	{
        gameopContext = newContext;
    }

    Character *GetGameOpContext( void )
	{
        return gameopContext;
    }

    void PacketBackpack( TFCPacket &sending, bool gameop = false);
    void PacketRobBackpack( Unit *robber, TFCPacket &sending );
	void SendBackpackContentPacket(); // Send to client an updated list of backpack's contents

    void AddToBackpack( Objects *obj );
	ErrorCodes::TakeFromBackpack TakeFromBackpack( DWORD itemID, DWORD itemQty, Objects *returnedObj, BOOL ignoreCannotDropFlag = false );
    
    virtual void SetLevel( WORD wLevel );

    void Rob( DWORD objId );

    bool RangedAttack();    

    DWORD EquipCount( WORD itemId );
    DWORD BackCount( WORD itemId );
    void  BackRemove( WORD itemId ,int iQty);

    void ClearAllSkillsAndSpells();
    void ClearAllSkillsAndSpells2();
    void ClearAllTeleportSpells();

    void BroadcastSeraphArrival();

    bool CanInvite();

    virtual void RemoveReferenceTo( Unit *theUnit );

	int AnalyseActionWorld( char *pTxt );

	bool boAuthGM;
	WORD wLang;

   void    SetGuildName(char *pstrName);
   CString GetGuildName(){return m_strGuildName;};

   void    SetGuildNameInvited(char *pstrName, Character *lpCharacter);
   CString GetGuildNameInvited(){return m_strGuildNameInvited;};
   Character *GetGuildNameInvitedChar(){return m_pInvitedByChar;};

   void  SetGuildTitle(DWORD dwTitle);
   DWORD GetGuildTitle(){return m_dwGuildTitle;};

   void  SetGuildPermission(DWORD dwPermission);
   DWORD GetGuildPermission(){return m_dwGuildPermission;};
   void  SetGuildPermissionTmp(DWORD dwPermission);
   DWORD GetGuildPermissionTmp(){return m_dwGuildPermissionTmp;};

   BOOL GetIsGuildChesting(){return boCharacterIsGuildChesting;}

   void  ValidNMSGold();

   //MINIONS Prototype 
   void ShowMinion(bool bShow);
   void ManageMinionMaintenance();
   void DestroyMinion();

   void SetCompagnonName(CString strName);
   void SetCompagnonID( DWORD dwID )     {m_dwCompagnonID = dwID;};

   CString GetCompagnonName(){return m_strCompagnonName;};
   DWORD   GetCompagnonID()  {return m_dwCompagnonID;};


   void    SetRP_XP     ( int iVal );
   void    SetRP_XPLevel( int iLevel);
   int     GetRP_XP()       {return m_iRPXP;};
   int     GetRP_XPLevel();


  

private:
	WorldPos	initPos;
	WorldPos	prevPos;
	DWORD		initTime;
   DWORD		lastInviteTime;
   DWORD		MinionLastCall;

    virtual ~Character();
    void RangeAttack( Unit *target );
    static void AutoConfigUpdate( void );

    BOOL CreateCharacter(CString csName, LPBYTE lpbAnswers );

    void SynchronizeGold( void );

    inline void DestroyEquipment( void );

//    WORD wLang;

    static void DataSaveCallback( DWORD dwSaveResult, LPVOID lpData );
    
    
    inline void SavingStart( void );
    inline void SavingStop( void );	
    BYTE numberOfSaveFailures; // Counts how many sucessive times it failed to save player.
    
    HANDLE hCreationEvent;
    
    inline void PacketSingleEquip( BYTE equip_pos, TFCPacket &sending );
    
    Group *lpGroup;
    
    Character *gameopContext;
    
    signed char cAgressive;
    
    int nCurrentWeight;
    
    WORD clan;
    
    DWORD MaxHP;
    DWORD HP;
    DWORD dwLastHealing;

    BOOL  bNeedUpdateGold;
    BOOL  bNeedUpdateGoldMsg;
    DWORD dwGoldAcc;
    
    WORD MaxMana;
    WORD Mana;
    
    BOOL boLoaded;

    BOOL  bChestChanged;
    CLock LockChestChanged;
    
    unsigned __int64 xp;
    unsigned __int64 xpLastSend;
    
    CString account;
    
    CString m_strEmpty;
    CString PlayerName;
    CString csPseudoName;
    
    // Title and extra information for user listing.
    CString csListingTitle;
    CString csListingMisc;
    
    // Guild ref, rank, points
    //Guilds *GuildRef;
    unsigned char GuildRank;
    int GuildPoints;
    
    BOOL KillMe;
    BOOL _died_;
    bool seraphAlreadyArrived;
    
    Players *ThisPlayer;

    bool m_bAsyncViewBackpackProgress;
    bool m_bAsyncCCUserListProgress;
    bool m_bAsyncEquipItem;
    
    EXHAUST exhaust;
    unsigned long LastMoveTime;
    
	int PlayerGold;

	TemplateList <Unit> *backpack;	
	Unit *equipped[ EQUIP_POSITIONS ];

	ItemContainer *chest; // The chest :D
	BOOL boCharacterIsChesting; // Defines that the character is allowed to chest
    BOOL boCharacterIsGuildChesting; // Defines that the character is allowed to chest
	
	TradeMgr2 m_TradeMgr2; // The trade manager object

	WORD wNbSkillPnts;
	WORD wNbStatPnts;

	TemplateList <USER_SKILL> tlusSkills[NB_SKILL_HOOKS];
	TemplateList <USER_SKILL> tlusSpells;	// All the spells.

    TemplateList <USER_PROFESSION_F>   tlProfessionAcq;	// All Know Formula

    CString m_strGuildName;
    CString m_strGuildNameInvited;
    DWORD   m_dwGuildTitle;
    DWORD   m_dwGuildPermission;
    DWORD   m_dwGuildPermissionTmp;
    Character *m_pInvitedByChar;

    DWORD   m_dwSaveCallBackTimeStart;



	static BYTE m_bSkillPnt[ (MAX_LEVEL_XP / 10) + 1 ];

    // Auto-combat
    bool autoCombatState;
    Attack autoCombatAttack;
    Attack prevAutoCombatAttack;
    bool   prevAutoCombatState;
    Unit  *prevTarget;

    friend AutoArrowRemove; 

    DWORD
        bRollSTR : 4,
        bRollEND : 4,
        bRollINT : 4,
        bRollAGI : 4;	
    DWORD
        bRollWIL : 4,
        bRollWIS : 4,
        bRollLCK : 4;        
	
	char m_chCombatMode;
   int  m_iRPPhase;
   int  m_iRPTalkCnt;
   int  m_iRPNOTTalkCnt;

   int  m_iArenaID;
   int  m_iArenaType;
   int  m_iArenaKill;
   int  m_iArenaDead;
   int  m_iArenaFlag;
   int  m_iArenaTeam;
   DWORD m_dwArenaLastStart;
   double  m_dArenaPVP;
   DWORD   m_iArenaINACTIFTime;
   int     m_iArenaINACTIFMin;
   DWORD   m_iArenaDUREETime;
   int     m_iArenaDUREEMin;
   int     m_iArenaPOINTS;


   Minions *m_pMinions;
   CString  m_strCompagnonName;
   DWORD    m_dwCompagnonID;

   int    m_iRPXP;
   int    m_iRPXPPoint;
   


};


#endif // !defined(AFX_CHARACTER_H__6EB985F4_D137_11D0_B543_C09D9B000000__INCLUDED_)
