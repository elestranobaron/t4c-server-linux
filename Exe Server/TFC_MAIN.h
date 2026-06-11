/******************************************************************************
Modify for vs2008 (06/05/2009)
Add New server packet, Guild, AH, combart mode, etc etc by NightMare (29/06/2009)
Add Attack BOW by NightMare (29/06/2009)
/******************************************************************************/
// TFC_MAIN.h: interface for the TFC_MAIN class.
//
// This somehow simulates good old main() ;) Its primary purpose
// though is to localize all MAIN server fonction non-related to
// windowing systems. So it will be called by OnIdle which will
// pass the fonction to TFC for a round.
//
// Francois Leblanc 1997
//////////////////////////////////////////////////////////////////////
#if !defined(AFX_TFC_MAIN_H__ED5A8768_A9CC_11D0_9B9E_444553540000__INCLUDED_)
#define AFX_TFC_MAIN_H__ED5A8768_A9CC_11D0_9B9E_444553540000__INCLUDED_

#define SERVER_CONNECTION_HI_VERSION	0x000E
#define SERVER_CONNECTION_LO_VERSION	0x0000

#include "GameDefs.h"
#include "Players.h"
#include "TFCPacket.h"
#include "Broadcast.h"
#include "TFCTime.h"
#include "Creatures.h"
#include "WorldMap.h"
#include "Unit.h"
#include "GAME_RULES.h"
#include "SpellMessageHandler.h"

#if _MSC_VER >= 1000
	#pragma once
#endif // _MSC_VER >= 1000

/******************************************************************************/
#define _DEFAULT_TOUCH_RANGE			3
#define _DEFAULT_RANGE					80 // 50
#define _DEFAULT_RANGE_CHANGE  		80 // 50
#define _DEFAULT_RANGE_REMOVE  		80 // 50
#define _NB_MAINTAINED_MONSTERS			650 // 500
#define  _MONSTER_LIFE_SPAN				1 MINUTE

#define PERCENT	/ 100

#define _DEFAULT_REACH_RANGE			3

#define	RQ_MoveNorth					1 
#define	RQ_MoveNorthEast				2 
#define	RQ_MoveEast						3 
#define	RQ_MoveSouthEast				4 
#define	RQ_MoveSouth					5 
#define	RQ_MoveSouthWest				6 
#define	RQ_MoveWest						7 
#define  RQ_MoveNorthWest				8 

#define RQ_GetPlayerPos					9 
#define RQ_Ack						    	10 
#define RQ_GetObject				   	11 
#define RQ_DepositObject				12 
#define RQ_PutPlayerInGame				13 // async
#define RQ_RegisterAccount				14 // async
#define RQ_DeletePlayer					15 // async
#define RQ_SendPeriphericObjects		16 
#define RQ_GodCreateObject				17 
#define RQ_QueryPatchServerInfo2    18
#define RQ_ViewEquiped					19 
#define RQ_ExitGame						20 
#define RQ_EquipObject					21 
#define RQ_UnequipObject				22 
#define RQ_UseObject					23 
#define RQ_Attack						24 
#define RQ_CreatePlayer					25 // async
#define RQ_GetPersonnalPClist			26 		
#define RQ_IndirectTalk					27 
#define RQ_Shout						28 
#define RQ_Page							29 
#define RQ_DirectedTalk					30 
#define RQ_Reroll						31 
#define RQ_CastSpell					32 
#define RQ_HPchanged					33
#define RQ_BroadcastTextChange			34
#define RQ_GetUnitName					35
#define RQ_BreakConversation			36
#define RQ_LevelUp						37
#define RQ_ReturnToMenu					38
#define RQ_GetSkillList					39
#define RQ_SendTrainSkillList			40
#define RQ_SendBuyItemList				41
#define RQ_UseSkill						42
#define RQ_GetStatus					43
#define RQ_XPchanged					44
#define RQ_GetTime						45
#define RQ_FromPreInGameToInGame		46
#define RQ_YouDied						47
#define RQ_EnterChatterChannel		    48 //CC
#define RQ_SendChatterMessage	          49 //CC
#define RQ_DirectedTalkNoFeed  			 50 
#define RQ_GetChatterUserList2	        51 //CC
#define RQ_GetSkillStatPoints			52
#define RQ_GoldChange					53
#define RQ_ViewGroundItemIndentContent	54
#define RQ_SendTeachSkillList			55
#define RQ_SendSellItemList				56
#define RQ_TeleportPlayer				57
#define RQ_SendStatTrain				58
#define RQ_QueryItemName				59
#define RQ_GetNearItems					60
#define RQ_PlayerFastMode				61
#define RQ_SendSpellList				62
#define RQ_ServerMessage				63
#define RQ_SpellEffect                  64
#define RQ_QueryServerVersion           65
#define RQ_MessageOfDay                 66
#define RQ_ManaChanged                  67
#define RQ_PuppetInformation            68
#define RQ_UnitUpdate                   69
#define RQ_MissingUnit                  70
#define RQ_QueryUnitExistence           71
#define RQ_UseItemByAppearance          72
#define RQ_CannotFindItemByAppearance   73
#define RQ_RemoveFromChatterChannel     74 //CC
#define RQ_GetChatterChannelList        75 //CC
#define RQ_ToggleChatterListening       86 //CC
#define RQ_UpdateGroupMembers           76
#define RQ_UpdateGroupInviteList        77
#define RQ_GroupInvite                  78
#define RQ_GroupJoin                    79
#define RQ_GroupLeave                   80
#define RQ_GroupKick                    81
#define RQ_NotifyGroupDisband           82
#define RQ_CreateEffectStatus           83
#define RQ_DispellEffectStatus          84
#define RQ_JunkItems                    85
#define RQ_UpdateGroupMemberHp          87
#define RQ_GroupToggleAutoSplit         88
#define RQ_TogglePage                   89
#define RQ_QueryNameExistence           90
#define RQ_QueryPatchServerInfo         91
#define RQ_UpdateWeight                 92
#define RQ_Rob                          93
#define RQ_DispellRob                   94
#define RQ_ArrowMiss                    95
#define RQ_ArrowHit                     96
#define RQ_GodFlagUpdate                97
#define RQ_SeraphArrival                98
#define RQ_AuthenticateServerVersion    99
#define RQ_Remort                       100
#define RQ_ExitGameAntiPlug				 101 
#define RQ_InfoMessage					    102
#define RQ_MaxCharactersPerAccountInfo  103
#define RQ_OpenURL						    105
// Chest Packets :D
#define RQ_ChestContents				   106
#define RQ_ChestAddItemFromBackpack		107
#define RQ_ChestRemoveItemToBackpack	108
#define RQ_ShowChest					      109
#define RQ_HideChest					      110
// Trade Packets :D
#define RQ_TradeContents				   111
#define RQ_TradeAddItemFromBackpack		112
#define RQ_TradeRemoveItemToBackpack	113
#define RQ_TradeInvite					116
#define RQ_TradeSetStatus				117
#define	RQ_TradeCancel					118
#define RQ_TradeClear					119
#define RQ_TradeStarted					120
#define RQ_TradeFinish					121

#define RQ_QueryItemInfo				122

#define RQ_UpdateBackpackAfterUse	123
#define RQ_DamageUnit               124
#define RQ_HealingUnit              125

#define RQ_NM_GetGuildList          126
#define RQ_NM_GuildInvite           127
#define RQ_NM_GuildJoin             128
#define RQ_NM_GuildLeave            129
#define RQ_NM_GuildKick             130
#define RQ_NM_GuildChangeSetting    131
#define RQ_NM_GuildChangeNote       132
#define RQ_NM_GuildGetLogs          133

#define RQ_NM_GuildChestShow        134
#define RQ_NM_GuildChestHide        135
#define RQ_NM_GuildChestContents  	136
#define RQ_NM_GUILDChestAddItem     137
#define RQ_NM_GUILDChestRemoveItem  138


#define RQ_NM_GetAHList             140
#define RQ_NM_AddAHItems            141
#define RQ_NM_BuyAHItems            142
#define RQ_NM_CancelAHItems         143
#define RQ_NM_InfoAHItems           144

#define RQ_ChestNormal				   148
#define RQ_ChestListe				   149
#define RQ_WeatherMsg				   150
#define RQ_GetStatus2				   151
#define RQ_GetUnitName2                152
#define RQ_GetPersonnalPClistEquitSkin 153
#define RQ_ViewBackpack2			   154
#define RQ_UseObject2			       155 
#define RQ_ViewInv                   156
#define RQ_AskCompagnonName			 157
#define RQ_1stApril                  158
#define RQ_UpdateSmile               159
 
#define RQ_NM_NMSGOLD_AchatOpt1     160
#define RQ_NM_NMSGOLD_AchatOpt2     161
#define RQ_NM_NMSGOLD_AchatOpt3     162
#define RQ_NM_NMSGOLD_AchatOpt4     163
#define RQ_NM_NMSGOLD_Acheter       164
#define RQ_NM_NMSGOLD_ListPanier     165
#define RQ_NM_NMSGOLD_UtiliserPanier 166
#define RQ_NM_NMSGOLD_CrediterPanier 167
#define RQ_NM_NMSGOLD_Sanction       168

#define RQ_RPStatus                 169
#define RQ_RP_BroadCastRP           170
#define RQ_RP_CreerRP               171
#define RQ_RP_TerminerRP            172
#define RQ_RP_RejoindreRP           173
#define RQ_RP_RejoindreAnswerRP     174
#define RQ_RP_ExpulserRP            175
#define RQ_RP_InviteRP              176
#define RQ_RP_InviteAnswerRP        177
#define RQ_RP_SignalerRP            178
//#define RQ_RP_EchangerRP            179 //Obselete
#define RQ_RP_BroadCastPVP          180
#define RQ_RP_BroadCastPVPStat      181

#define RQ_QB_GetQuestList          182
#define RQ_QB_GetQuestMsg           183
#define RQ_QB_GetActiveQuest        184
#define RQ_QB_GetQuestListComplete  185
#define RQ_QB_StopQuest             208


#define RQ_GMMSG_Post               186
#define RQ_GMMSG_Get                187
#define RQ_GMMSG_Close              188

#define RQ_GetAllArealinkForWorld   189
#define RQ_GetAllPlayerPos				190
#define RQ_SvrOptions               191
#define RQ_SvrNPC                   192
#define RQ_SvrSpellList             193
#define RQ_SvrMonsterList           194
#define RQ_SvrItemsList             195
#define RQ_SvrMonsterSkin     		196
#define RQ_SvrQuestFlagList    		197
#define RQ_AttackMode					198
#define RQ_NameChange					199
#define RQ_NM_DeathStatus     		200
#define RQ_NM_DeathProgress   		201
#define RQ_NM_DeathResurect   		202
#define RQ_NM_GetProfession         203
#define RQ_NM_SendTeachFormuleList	204
#define RQ_NM_SendMakeFormule       205
#define RQ_NM_XPScrollProgress 		206
#define RQ_NM_ORScrollProgress      207
//                                  208 //utiliser dans les quete de quest book
#define RQ_SendPointsItemList       209

#define	RQ_MoveNorthKB					211
#define	RQ_MoveNorthEastKB			212 
#define	RQ_MoveEastKB					213 
#define	RQ_MoveSouthEastKB			214 
#define	RQ_MoveSouthKB					215 
#define	RQ_MoveSouthWestKB			216 
#define	RQ_MoveWestKB					217 
#define  RQ_MoveNorthWestKB			218 

#define RQ_ChestNewContents			220 //RQ_ChestContents
#define RQ_ShowChestNew				   221 //RQ_ShowChest
#define RQ_HideChestNew	   		   222 //RQ_HideChest

#define RQ_GuildChestNewShow        223 //RQ_NM_GuildChestShow
#define RQ_GuildChestNewHide        224 //RQ_NM_GuildChestHide
#define RQ_GuildChestNewContents  	225 //RQ_NM_GuildChestContents

#define RQ_ARENA_SendTakeStatus     226

#define RQ_ARENA1_Join              227 //OK2
#define RQ_ARENA1_Leave             228 //OK2
#define RQ_ARENA1_UpdateTimeBS      229 //OK2
#define RQ_ARENA1_GetWaitPlayerList 231 //OK2
#define RQ_ARENA1_UpdatePlayStat    232
#define RQ_GetEventsList            233
#define RQ_UpdateFactionID          234


#define RQ_QUANTITY						240 // IMPORTANT! This must be the last and higher value on the list!




#define PL_FACTION_NONE 						   0x00000001 //   0
#define PL_FACTION_01  							   0x00000002 //   1
#define PL_FACTION_02						      0x00000004 //   2
#define PL_FACTION_03				      	   0x00000008 //   3
#define PL_FACTION_04							   0x00000010 //   4
#define PL_FACTION_05							   0x00000020 //   5
#define PL_FACTION_06					         0x00000040 //   6
#define PL_FACTION_07         				   0x00000080 //   7
#define PL_FACTION_08			      		   0x00000100 //   8
#define PL_FACTION_09					         0x00000200 //   9 
#define PL_FACTION_10						      0x00000400 //   10
#define PL_FACTION_11         				   0x00000800 //   11
#define PL_FACTION_12			      		   0x00001000 //   12
#define PL_FACTION_13	               	   0x00002000 //   13
#define PL_FACTION_14		            		0x00004000 //   14
#define PL_FACTION_15         					0x00008000 //   15
#define PL_FACTION_16	                     0x00010000 //   16
#define PL_FACTION_17	         			   0x00020000 //   17
#define PL_FACTION_18	         			   0x00040000 //   18
#define PL_FACTION_19            				0x00080000 //   19
#define PL_FACTION_20      					   0x00100000 //   20
#define PL_FACTION_21	         				0x00200000 //   21
#define PL_FACTION_22            				0x00400000 //   22
#define PL_FACTION_23	         			   0x00800000 //   23
#define PL_FACTION_24         				   0x01000000 //   24
#define PL_FACTION_25	                     0x02000000 //   25
#define PL_FACTION_26      					   0x04000000 //   26
#define PL_FACTION_27		      			   0x08000000 //   27
#define PL_FACTION_28	   					   0x10000000 //   28
#define PL_FACTION_29	         				0x20000000 //   29
#define PL_FACTION_30							   0x40000000 //   30
#define PL_FACTION_31							   0x80000000 //   31


/******************************************************************************/
typedef struct _DEATHROW
{
	Unit *lpuCondemned;
	BOOL boDelete;
} DEATHROW, *LPDEATHROW;

extern TemplateList <DEATHROW> tluDeathRow;

/******************************************************************************/
typedef struct _STD_SPELLMSG
{
 UINT SpellID;
 BYTE bCallReason;
 WORD wMana;
 Unit *self;
 Unit *medium;
 Unit *target;
 WorldPos PinPoint;
 void *INparameters;
 void *OUTparameters;

} STD_SPELLMSG, *LPSTD_SPELLMSG;

typedef void (* UNITSTARTUPFUNC)( void );

/******************************************************************************/
typedef struct _PLAYERMSG_CONTEXT
{
	Players *lpPlayer;
	DWORD dwMessageID;
	LPBYTE lpRawPacket;
	DWORD	dwRawPacketSize;
	TemplateList<TFCPacket> *lptlPacketList;
} PLAYERMSG_CONTEXT, *LPPLAYERMSG_CONTEXT;

/******************************************************************************/
// This is a DLL interface for TFC_MAIN
class __declspec(dllexport) TFCMAIN
/******************************************************************************/
{
public:
	static void RegisterUnitStartupFunction( UNITSTARTUPFUNC lpFunc );
	static void CallUnitStartupFunctions( void );

	static WorldMap *GetWorld(unsigned short which);
    static unsigned long GetRound();

   static DWORD GetColosseum();
   static DWORD GetColosseum2();
   static DWORD GetColosseum3();
   static DWORD GetColosseum4();
	static DWORD GetDoppelganger();
	static DWORD GetMaxRemorts();
	static DWORD GetMaxCharactersPerAccount();

   static long GlobalFlagView(unsigned long ulFlagID);
   static void GlobalFlagSet (unsigned long ulFlagID, long lValue,BOOL bSave);
   static void GlobalFlagDel (unsigned long ulFlagID);
   static unsigned long GetPLFactionMask(unsigned long ulVal);
   static void BroadcastFriendlyChange (WorldPos wlPos, DWORD dwMonsterID);

   static void AddCastSpellPos(Unit *pSelf,WorldPos wlPos,DWORD dwSpellID,DWORD dwFreq,DWORD dwRepeat);
   static void ProcessCastSpellPos();

	static CString GetHomeDir();
	static CString GetPlayerDir();
	static CString GetAccountDir();
	
	static WORD GetMaxWorlds();

	static DWORD *GetCurrentGlobalID();

	static void AddMonster(Unit *Creature);

	static int Attack( Unit *Attacker, Unit *Target, bool &blockedPath );
    static int AttackBow( Unit *Attacker, Unit *Target, bool &blockedPath ,int iDefRangeAttack, BoostFormula *pBoust, int ArcherySkill);

	//static void Broadcast(WorldPos where, unsigned char range, TFCPacket &sending);
	//static void Broadcast(unsigned int event, WorldPos where, unsigned char range, unsigned int *param);

	//static WORD GetMaxPlayers( void ){ return GetMaxCharactersPerAccount(); };

    static void StartBeat();

private:
	struct _FUNC{
		UNITSTARTUPFUNC lpFunc;
	};

	static TemplateList <_FUNC> tlFuncList;
};

/******************************************************************************/
struct SubmitNearUnit
{
	enum{
			ToDestroy,
			ToCreate
	};

	BOOL NoticeDeletion;
	BOOL IsInView;
	char Submission;
	WorldPos pos;
	Creatures *target;
};

/******************************************************************************/
// Here ends the DLL Interface functions
class TFC_MAIN 
/******************************************************************************/
{
public:
	TFC_MAIN();
	virtual ~TFC_MAIN();	

	TemplateList <HINSTANCE> tlDllInstance;

	unsigned int  flush_time;
	unsigned char max_chances;
	
	unsigned int world_number;

	WorldMap *World;

    WORD    wMaxPlayers;

    DWORD   dwVersion;
};

#endif // !defined(AFX_TFC_MAIN_H__ED5A8768_A9CC_11D0_9B9E_444553540000__INCLUDED_)
