/******************************************************************************
Modify for vs2008 (01/05/2009)
NightMare_TODO:
Add Guild, AH,PVP... by NightMare (29/06/2009)
Add in RQFUNC_PutPlayerInGame     tyruc pour valider le PVP...et MOTD by NightMare (29/06/2009)
Add in RQFUNC_IndirectTalk   truc firewall complete auto, etc etc by NightMare (29/06/2009)
Add in RQFUNC_DirectedTalk truc firewall auto complete, etc etc by NightMare (29/06/2009)
/******************************************************************************/
#include "stdafx.h"
#include "TFC Server.h"
#include "TFC ServerDlg.h"
#include "TFCMessagesHandler.h"
#include "TFC_MAIN.h"
#include "Broadcast.h"
#include <eh.h>
#include "Random.h"
#include "TFCMessagesHandler.h"
#include "SkillListing.h"
#include "AsyncFuncQueue.h"
#include "SysopCmd.h"
#include "IntlText.h"
#include "RegKeyHandler.h"
#include "PacketManager.h"
#include "PlayerManager.h"
#include "AutoConfig.h"
#include "Format.h"
#include "PacketManager.h"
#include "QuestFlagsListing.h"
#include "FormatPlayerName.h"
#include "WeatherEffect.h"
#include "GuildMaster.h"
#include "RPMaster.h"
#include "Arena1Master.h"
#include "Arena2Master.h"
#include "GMMsgMaster.h"
#include "AuctionMaster.h"
#include "PacketManager.h"
#include "EventsMaster.h"
#include "UDP/NMPacketManager.h"

#ifdef _DEBUG
	#undef THIS_FILE
	static char THIS_FILE[]=__FILE__;
	#define new DEBUG_NEW
#endif


/******************************************************************************/
#define MAX_PLAYERS     75
#define MAX_USER_KEY    "UserMax"
#define RQ_HEADER           try{

#define RQ_FOOTER( __rq )   }catch( ... ){\
                                _LOG_DEBUG\
                                    LOG_ALWAYS,\
                                    "Detected crash in RQ_"__rq\
                                LOG_\
                                _LOG_DEBUG\
                                    LOG_ALWAYS,\
                                    "Packet: [ %s ]",\
                                    (LPCTSTR)msg->GetDebugPacketString()\
                                LOG_\
                                throw;\
                            }



#define LOG_PACKET_ERROR( __packetErrText )	_LOG_DEBUG \
		LOG_DEBUG_LVL1,\
		"Error in "#__packetErrText\
		", packet was incomplete. Packet:[ %s]",\
		(LPCTSTR)msg->GetDebugPacketString()\
	LOG_

#define RQTRAIN_STAT( stat, text )	GET_WORD( wTrain );\
	if( wTrain != 0 ){\
		dwStat = user->self->GetTrue##stat\
		(); if( user->self->UseStatPnts( wTrain ) ){\
			user->self->Set##stat\
			( (WORD)( dwStat + wTrain ) );\
            csText.Format( "Trained %u points in " text ". %u -> %u.", wTrain, dwStat, user->self->GetTrue##stat\
            () );\
            csReport += csText;\
		}\
	}

/******************************************************************************/
// Structure pour les fonction asynchrone
struct PATCH_SERVER
{
   CString csIP;
   WORD    wPort;
};

static struct WEBPATCH_SERVER : public CLock
{
   CString csIP;
   CString csImagePath;
   CString csIPNew;
   CString csImagePathNew;
   CString csUserName;
   CString csPassword;
} webPatchServer;

static struct vvector : public vector< PATCH_SERVER >, CLock{} vPatchServers;

typedef struct _ASYNC_PACKET_FUNC_PARAMSEX
{
   Players		*user;
   CString     strParam1;
   CString     strParam2;
   int         iParam1;
   int         iParam2;
} ASYNC_PACKET_FUNC_PARAMSEX, *LPASYNC_PACKET_FUNC_PARAMSEX;

typedef struct _ASYNC_PACKET_FUNC_PARAMS
{
	TFCPacket	*msg;
	Players		*user;
	RQ_SIZE		rqRequestID;
   CString     strParam;
} ASYNC_PACKET_FUNC_PARAMS, *LPASYNC_PACKET_FUNC_PARAMS;

typedef struct _RQSTRUCT_CREATE_PLAYER
{
	ASYNC_PACKET_FUNC_PARAMS sParams;
	BYTE lpbAnswers[ 6 ];
} RQSTRUCT_CREATE_PLAYER, *LPRQSTRUCT_CREATE_PLAYER;

typedef struct _RQSTRUCT_EQUIP_ITEM
{
   ASYNC_PACKET_FUNC_PARAMS sParams;
   unsigned long ulID;
} RQSTRUCT_EQUIP_ITEM, *LPRQSTRUCT_EQUIP_ITEM;

typedef struct _RQSTRUCT_REGISTER_ACCOUNT
{
   ASYNC_PACKET_FUNC_PARAMS sParams;	
   CString csPassword;
   CString csAccount;
   sockaddr_in sockAddrO;
   sockaddr_in sockAddrI;
   unsigned int packetSeedID;
   unsigned short usLangUsed; // Ajout du système multilingue.
} RQSTRUCT_REGISTER_ACCOUNT, *LPRQSTRUCT_REGISTER_ACCOUNT;

struct RQSTRUCT_QueryNameExistence
{
    CString name;
    sockaddr_in sockAddrO;
    sockaddr_in sockAddrI;
};

typedef struct _ASYNC_PACKET_FUNC_PARAMSPLUS
{
   ASYNC_PACKET_FUNC_PARAMS sParams;
   WorldPos wlPos;
   DWORD    dwID;
   DWORD    dwID2;
   BYTE     chDirection;
} ASYNC_PACKET_FUNC_PARAMSPLUS, *LPASYNC_PACKET_FUNC_PARAMSPLUS;


typedef struct _RQSTRUCT_PAGE_SHOUT_TALK
{
   ASYNC_PACKET_FUNC_PARAMS sParams;
   CString strMessage;
   DWORD   dwColor;
} RQSTRUCT_PAGE_SHOUT_TALK, *LPRQSTRUCT_PAGE_SHOUT_TALK;

/******************************************************************************/
// Externs..
extern TFC_MAIN *TFCServer;
extern CTFCServerApp theApp;
extern Random rnd;
extern TFC_MAIN *TFCServer;

TemplateList <LOCAL_USER> TFCMessagesHandler::tlLocalUsers;
DWORD                     TFCMessagesHandler::dwMaxLocalUsers;
DWORD                     TFCMessagesHandler::dwMaxRemoteUsers;

CString                   TFCMessagesHandler::m_strOptionsTexte[NBR_OPT_GM_OPTIONS];
char                      TFCMessagesHandler::m_chOptionsValue [NBR_OPT_GM_OPTIONS];

/******************************************************************************/
namespace
/******************************************************************************/
{
    CString csMotD;
    CString csMotDMessage;

	class PacketFunc
	{
	public:
		PacketFunc()
		{
			lpFunc = NULL;
		};

		void Setup( LPPACKET_FUNC lpPacketFunc )
		{
			lpFunc = lpPacketFunc;
        };

		LPPACKET_FUNC GetFunc()
		{
			return lpFunc; 
		};

    private:
        LPPACKET_FUNC lpFunc;
    };

    PacketFunc FuncTable[ RQ_QUANTITY ];    
    cODBCMage ODBCAuth;//Que des requete direct AUTO Commit     

};

/******************************************************************************/
// Creates the TFC Message handler
void TFCMessagesHandler::Create( void )
/******************************************************************************/
{
    _LOG_DEBUG
        LOG_DEBUG_LVL4,
        "Registring messages handlers."
    LOG_
		
   RegisterFunction( RQFUNC_PutPlayerInGame,				   RQ_PutPlayerInGame );           //Async Connection
   RegisterFunction( RQFUNC_RegisterAccount,				   RQ_RegisterAccount );           //Async Connection
   RegisterFunction( RQFUNC_QueryNameExistence,			   RQ_QueryNameExistence );        //Async Connection // and GM
   RegisterFunction( RQFUNC_DeletePlayer,					   RQ_DeletePlayer );              //Async Connection //delete player
   RegisterFunction( RQFUNC_CreatePlayer,					   RQ_CreatePlayer );              //Async Connection
   RegisterFunction( RQFUNC_QueryServerVersion,			   RQ_QueryServerVersion );        //      Connection
   RegisterFunction( RQFUNC_MessageOfDay,					   RQ_MessageOfDay );              //      Connection
   RegisterFunction( RQFUNC_QueryPatchServerInfo,			RQ_QueryPatchServerInfo );      //      Connection
   RegisterFunction( RQFUNC_QueryPatchServerInfo2,  		RQ_QueryPatchServerInfo2 );      //      Connection
   RegisterFunction( RQFUNC_AuthenticateServerVersion,	RQ_AuthenticateServerVersion ); //      Connection



   RegisterFunction( RQFUNC_Ack,						   	   RQ_Ack );                         //Dummy
   RegisterFunction( RQFUNC_GodCreateObject,				   RQ_GodCreateObject );             //Dummy
   RegisterFunction( RQFUNC_GetSkillStatPoints,			   RQ_GetSkillStatPoints );          //Dummy ?? jamais envoyer par le client
   RegisterFunction( RQFUNC_GoldChange,					   RQ_GoldChange );                  //Dummy ?? jamais envoyer par le client
   RegisterFunction( RQFUNC_ViewGroundItemIndentContent,	RQ_ViewGroundItemIndentContent ); //Dummy ?? jamais envoyer par le client
   RegisterFunction( RQFUNC_UpdateGroupMembers,			   RQ_UpdateGroupMembers );          //Dummy
   RegisterFunction( RQFUNC_UpdateGroupInviteList,			RQ_UpdateGroupInviteList );       //Dummy


   RegisterFunction( RQFUNC_SendPeriphericObjects,		   RQ_SendPeriphericObjects );      //
   RegisterFunction( RQFUNC_ViewBackpack2,					RQ_ViewBackpack2 );              //
   RegisterFunction( RQFUNC_ViewInv,		      			RQ_ViewInv );                    //
   RegisterFunction( RQFUNC_ViewEquipped,					   RQ_ViewEquiped );                //
   RegisterFunction( RQFUNC_EquipObject,					   RQ_EquipObject );                //
   RegisterFunction( RQFUNC_UnequipObject,					RQ_UnequipObject );              //
   RegisterFunction( RQFUNC_GetPersonnalPClist,		   	RQ_GetPersonnalPClist );         //
   RegisterFunction( RQFUNC_IndirectTalk,					   RQ_IndirectTalk );               //
   RegisterFunction( RQFUNC_Shout,							   RQ_Shout );                      //
   RegisterFunction( RQFUNC_Page,					   		RQ_Page );                       //
   RegisterFunction( RQFUNC_DirectedTalk,			   		RQ_DirectedTalk );               //
   RegisterFunction( RQFUNC_DirectedTalkNoFeed,			   RQ_DirectedTalkNoFeed );               //
   
   RegisterFunction( RQFUNC_Reroll,						      RQ_Reroll );                     //
   RegisterFunction( RQFUNC_GetUnitName,				  	   RQ_GetUnitName );                //
   RegisterFunction( RQFUNC_GetUnitName2,					   RQ_GetUnitName2 );               //
   RegisterFunction( RQFUNC_GetSkillList,					   RQ_GetSkillList );               //
   RegisterFunction( RQFUNC_GetStatus,					    	RQ_GetStatus );                  //
   RegisterFunction( RQFUNC_GetStatus2,						RQ_GetStatus2 );                 //
   RegisterFunction( RQFUNC_UpdateFactionID,		   		RQ_UpdateFactionID );            //
   
   RegisterFunction( RQFUNC_ChestNormal,						RQ_ChestNormal );                //
   RegisterFunction( RQFUNC_ChestListe,						RQ_ChestListe );                 //
   
   RegisterFunction( RQFUNC_FromPreInGameToInGame,			RQ_FromPreInGameToInGame );      //
   RegisterFunction( RQFUNC_SendChatterMessage,			   RQ_SendChatterMessage );         //
   RegisterFunction( RQFUNC_GetChatterUserList2,      	RQ_GetChatterUserList2 );        //
   RegisterFunction( RQFUNC_QueryItemName,					RQ_QueryItemName );              //
   RegisterFunction( RQFUNC_GetNearItems,					   RQ_GetNearItems );               //
   RegisterFunction( RQFUNC_SendSpellList,					RQ_SendSpellList );              //
   RegisterFunction( RQFUNC_PuppetInformation,				RQ_PuppetInformation );          //
   RegisterFunction( RQFUNC_RemoveFromChatterChannel,		RQ_RemoveFromChatterChannel );   //
   RegisterFunction( RQFUNC_GetChatterChannelList,			RQ_GetChatterChannelList );      //
   RegisterFunction( RQFUNC_GroupInvite,					   RQ_GroupInvite );                //
   RegisterFunction( RQFUNC_GroupJoin,						   RQ_GroupJoin );                  //
   RegisterFunction( RQFUNC_GroupLeave,					   RQ_GroupLeave );                 //
   RegisterFunction( RQFUNC_GroupKick,						   RQ_GroupKick );                  //
   RegisterFunction( RQFUNC_JunkItems,						   RQ_JunkItems );                  //
   RegisterFunction( RQFUNC_ToggleChatterListening,		RQ_ToggleChatterListening );     //
   RegisterFunction( RQFUNC_GroupToggleAutoSplit,			RQ_GroupToggleAutoSplit );       //
   RegisterFunction( RQFUNC_ChestAddItemFromBackpack,		RQ_ChestAddItemFromBackpack );   //
   RegisterFunction( RQFUNC_ChestRemoveItemToBackpack,	RQ_ChestRemoveItemToBackpack );  //
   RegisterFunction( RQFUNC_TradeInvite,					   RQ_TradeInvite );                //
   RegisterFunction( RQFUNC_TradeCancel,				   	RQ_TradeCancel );                //
   RegisterFunction( RQFUNC_TradeSetStatus,	   			RQ_TradeSetStatus );             //
   RegisterFunction( RQFUNC_TradeAddItemFromBackpack,		RQ_TradeAddItemFromBackpack );   //
   RegisterFunction( RQFUNC_TradeRemoveItemToBackpack,   RQ_TradeRemoveItemToBackpack );  //
   RegisterFunction( RQFUNC_TradeClear,			    		RQ_TradeClear );                 //
   RegisterFunction( RQFUNC_QueryItemInfo,					RQ_QueryItemInfo );              //
   RegisterFunction( RQFUNC_NM_GetGuildList,             RQ_NM_GetGuildList );            //Async par GUILD Request
   RegisterFunction( RQFUNC_NM_GuildInvite,              RQ_NM_GuildInvite );             //

   RegisterFunction( RQFUNC_NM_GuildJoin,                RQ_NM_GuildJoin );               //Async par GUILD Request
   RegisterFunction( RQFUNC_NM_GuildLeave,               RQ_NM_GuildLeave );              //Async par GUILD Request
   RegisterFunction( RQFUNC_NM_GuildKick,                RQ_NM_GuildKick );               //Async par GUILD Request
   RegisterFunction( RQFUNC_NM_GuildChangeSetting,       RQ_NM_GuildChangeSetting );      //Async par GUILD Request
   RegisterFunction( RQFUNC_NM_GuildChangeNote,          RQ_NM_GuildChangeNote );         //Async par GUILD Request
   RegisterFunction( RQFUNC_NM_GuildGetLogs,             RQ_NM_GuildGetLogs );            //Async par GUILD Request
   RegisterFunction( RQFUNC_NM_GUILDChestAddItem,        RQ_NM_GUILDChestAddItem );       //
   RegisterFunction( RQFUNC_NM_GUILDChestRemoveItem,     RQ_NM_GUILDChestRemoveItem );    //

   RegisterFunction( RQFUNC_NM_GetAHList,                RQ_NM_GetAHList );               //Async par AH Request
   RegisterFunction( RQFUNC_NM_AddAHItems,               RQ_NM_AddAHItems );              //Async par AH Request
   RegisterFunction( RQFUNC_NM_BuyAHItems,               RQ_NM_BuyAHItems );              //Async par AH Request
   RegisterFunction( RQFUNC_NM_CancelAHItems,            RQ_NM_CancelAHItems );           //Async par AH Request
   RegisterFunction( RQFUNC_NM_InfoAHItems,              RQ_NM_InfoAHItems );             //Async par AH Request
   RegisterFunction( RQFUNC_NM_NMSGOLD_Acheter,          RQ_NM_NMSGOLD_Acheter );         //
   RegisterFunction( RQFUNC_NM_NMSGOLD_ListPanier,       RQ_NM_NMSGOLD_ListPanier );      //
   RegisterFunction( RQFUNC_NM_NMSGOLD_UtiliserPanier,   RQ_NM_NMSGOLD_UtiliserPanier );  //
   RegisterFunction( RQFUNC_NM_NMSGOLD_CrediterPanier,   RQ_NM_NMSGOLD_CrediterPanier );  //


   RegisterFunction( RQFUNC_GMMSG_Post,                  RQ_GMMSG_Post);                  //
   RegisterFunction( RQFUNC_GMMSG_Get ,                  RQ_GMMSG_Get);                   //
   RegisterFunction( RQFUNC_GMMSG_Close,                 RQ_GMMSG_Close);                 //


   RegisterFunction( RQFUNC_GetAllPlayerPos,             RQ_GetAllPlayerPos);             //
   RegisterFunction( RQFUNC_SvrNPC,                      RQ_SvrNPC);                      //
   RegisterFunction( RQFUNC_SvrSpellList,                RQ_SvrSpellList);                //
   RegisterFunction( RQFUNC_SvrMonsterList,              RQ_SvrMonsterList);              //
   RegisterFunction( RQFUNC_SvrItemsList,                RQ_SvrItemsList);                //
   RegisterFunction( RQFUNC_SvrMonsterSkin,              RQ_SvrMonsterSkin);              //
   RegisterFunction( RQFUNC_SvrQuestFlagList,            RQ_SvrQuestFlagList);            //
   RegisterFunction( RQFUNC_NM_DeathResurect,            RQ_NM_DeathResurect );           //
   RegisterFunction( RQFUNC_NM_GetProfession,            RQ_NM_GetProfession );           //
   RegisterFunction( RQFUNC_NM_SendMakeFormule,          RQ_NM_SendMakeFormule );         //

 
   RegisterFunction( RQFUNC_RQ_RP_BroadCastRP,           RQ_RP_BroadCastRP);
   RegisterFunction( RQFUNC_RQ_RP_CreerRP,               RQ_RP_CreerRP );   
   RegisterFunction( RQFUNC_RQ_RP_TerminerRP,            RQ_RP_TerminerRP);
   RegisterFunction( RQFUNC_RQ_RP_RejoindreRP,           RQ_RP_RejoindreRP);
   RegisterFunction( RQFUNC_RQ_RP_RejoindreAnswerRP,     RQ_RP_RejoindreAnswerRP);
   RegisterFunction( RQFUNC_RQ_RP_ExpulserRP,            RQ_RP_ExpulserRP);
   RegisterFunction( RQFUNC_RQ_RP_InviteRP,              RQ_RP_InviteRP);
   RegisterFunction( RQFUNC_RQ_RP_InviteAnswerRP,        RQ_RP_InviteAnswerRP);
   RegisterFunction( RQFUNC_RQ_RP_SignalerRP,            RQ_RP_SignalerRP);
   RegisterFunction( RQFUNC_RQ_RP_BroadCastPVP,          RQ_RP_BroadCastPVP);
   RegisterFunction( RQFUNC_RQ_RP_BroadCastPVPStat,      RQ_RP_BroadCastPVPStat);

   RegisterFunction( RQFUNC_RQ_QB_GetQuestList,          RQ_QB_GetQuestList);
   RegisterFunction( RQFUNC_RQ_QB_GetQuestMsg,           RQ_QB_GetQuestMsg);
   RegisterFunction( RQFUNC_RQ_QB_GetActiveQuest,        RQ_QB_GetActiveQuest);
   RegisterFunction( RQFUNC_RQ_QB_GetQuestListComplete,  RQ_QB_GetQuestListComplete);
   RegisterFunction( RQFUNC_RQ_QB_StopQuest,             RQ_QB_StopQuest);
   
   
  

   
   
   
   
   
   
   
   

   


	RegisterFunction( RQFUNC_PlayerMove,					   RQ_MoveNorth );                 
	RegisterFunction( RQFUNC_PlayerMove,					   RQ_MoveNorthEast );
	RegisterFunction( RQFUNC_PlayerMove,					   RQ_MoveEast );
	RegisterFunction( RQFUNC_PlayerMove,					   RQ_MoveSouthEast );
	RegisterFunction( RQFUNC_PlayerMove,					   RQ_MoveSouth );
	RegisterFunction( RQFUNC_PlayerMove,					   RQ_MoveSouthWest );
	RegisterFunction( RQFUNC_PlayerMove,					   RQ_MoveWest );
	RegisterFunction( RQFUNC_PlayerMove,					   RQ_MoveNorthWest );
   RegisterFunction( RQFUNC_PlayerMove,					   RQ_MoveNorthKB );                 
   RegisterFunction( RQFUNC_PlayerMove,					   RQ_MoveNorthEastKB );
   RegisterFunction( RQFUNC_PlayerMove,					   RQ_MoveEastKB );
   RegisterFunction( RQFUNC_PlayerMove,					   RQ_MoveSouthEastKB );
   RegisterFunction( RQFUNC_PlayerMove,					   RQ_MoveSouthKB );
   RegisterFunction( RQFUNC_PlayerMove,					   RQ_MoveSouthWestKB );
   RegisterFunction( RQFUNC_PlayerMove,					   RQ_MoveWestKB );
   RegisterFunction( RQFUNC_PlayerMove,					   RQ_MoveNorthWestKB );
	RegisterFunction( RQFUNC_QueryPlayerPos,  			   RQ_GetPlayerPos );
	RegisterFunction( RQFUNC_GetObject,						   RQ_GetObject );
	RegisterFunction( RQFUNC_DepositObject,				   RQ_DepositObject );
	RegisterFunction( RQFUNC_ExitGame,					   	RQ_ExitGame );
	RegisterFunction( RQFUNC_UseObject,						   RQ_UseObject );
	RegisterFunction( RQFUNC_UseObject2,						RQ_UseObject2 );
	RegisterFunction( RQFUNC_Attack,						      RQ_Attack );
	RegisterFunction( RQFUNC_CastSpell,					    	RQ_CastSpell );
	RegisterFunction( RQFUNC_HPchanged,				    		RQ_HPchanged );
	RegisterFunction( RQFUNC_BroadcastTextChange,			RQ_BroadcastTextChange );
	RegisterFunction( RQFUNC_BreakConversation,				RQ_BreakConversation );
	RegisterFunction( RQFUNC_ReturnToMenu,				   	RQ_ReturnToMenu );
   RegisterFunction( RQFUNC_SendTrainSkillList,		   	RQ_SendTrainSkillList );
	RegisterFunction( RQFUNC_UseSkill,					    	RQ_UseSkill );
	RegisterFunction( RQFUNC_XPchanged,						   RQ_XPchanged );
	RegisterFunction( RQFUNC_GetTime,						   RQ_GetTime );
	RegisterFunction( RQFUNC_EnterChatterChannel,			RQ_EnterChatterChannel );   //a mettre async eventuellement
	RegisterFunction( RQFUNC_SendTeachSkillList,			   RQ_SendTeachSkillList );    //a mettre async eventuellement
	RegisterFunction( RQFUNC_SendBuyItemList,			   	RQ_SendBuyItemList );       //a mettre async eventuellement
   RegisterFunction( RQFUNC_SendPointsItemList,			 	RQ_SendPointsItemList );       //a mettre async eventuellement
	RegisterFunction( RQFUNC_SendSellItemList,				RQ_SendSellItemList );      //a mettre async eventuellement
	RegisterFunction( RQFUNC_SendStatTrain,					RQ_SendStatTrain );         //a mettre async eventuellement
   RegisterFunction( RQFUNC_QueryUnitExistence,		   	RQ_QueryUnitExistence );    //a valider si safe...
   RegisterFunction( RQFUNC_UseItemByAppearance,			RQ_UseItemByAppearance );
   RegisterFunction( RQFUNC_UpdateSmile ,		         	RQ_UpdateSmile );
   RegisterFunction( RQFUNC_TogglePage,					   RQ_TogglePage );
   RegisterFunction( RQFUNC_Rob,     							RQ_Rob );
   RegisterFunction( RQFUNC_NM_NMSGOLD_AchatOpt1,        RQ_NM_NMSGOLD_AchatOpt1 ); 
   RegisterFunction( RQFUNC_NM_NMSGOLD_AchatOpt2,        RQ_NM_NMSGOLD_AchatOpt2 ); 
   RegisterFunction( RQFUNC_NM_NMSGOLD_AchatOpt3,        RQ_NM_NMSGOLD_AchatOpt3); 
   RegisterFunction( RQFUNC_NM_NMSGOLD_AchatOpt4,        RQ_NM_NMSGOLD_AchatOpt4 ); 
	RegisterFunction( RQFUNC_NM_NMSGOLD_Sanction,         RQ_NM_NMSGOLD_Sanction ); 
   RegisterFunction( RQFUNC_SvrOptions,                  RQ_SvrOptions);
   RegisterFunction( RQFUNC_AttackMode,                  RQ_AttackMode);
   RegisterFunction( RQFUNC_SendTeachFormuleList,        RQ_NM_SendTeachFormuleList ); //a mettre async eventuellement
   RegisterFunction( RQFUNC_AskCompagnonName,            RQ_AskCompagnonName );
   RegisterFunction( RQFUNC_GetEventsList,               RQ_GetEventsList );
   
   

   RegisterFunction( RQFUNC_ARENA1_Join ,                  RQ_ARENA1_Join);        
   RegisterFunction( RQFUNC_ARENA1_Leave ,                 RQ_ARENA1_Leave);        
   RegisterFunction( RQFUNC_ARENA1_GetWaitPlayerList ,     RQ_ARENA1_GetWaitPlayerList);        
   RegisterFunction( RQFUNC_ARENA1_UpdatePlayStat    ,     RQ_ARENA1_UpdatePlayStat);        

   
    
    
    
    
    



    m_strOptionsTexte[ 0].Format("PVP drop");
    m_strOptionsTexte[ 1].Format("PVM drop");
    m_strOptionsTexte[ 2].Format("Filter UDP Packet");
    m_strOptionsTexte[ 3].Format("Log UDP Packet Analysed");
    m_strOptionsTexte[ 4].Format("Reload system");
    m_strOptionsTexte[ 5].Format("Guild system");
    m_strOptionsTexte[ 6].Format("Auction House / Interet system");
    m_strOptionsTexte[ 7].Format("Profession system");
    m_strOptionsTexte[ 8].Format("Damage and Healing broadcast");
    m_strOptionsTexte[ 9].Format("RP HRP Mode");
    m_strOptionsTexte[10].Format("PVP System");
    m_strOptionsTexte[11].Format("DUEL System");
    m_strOptionsTexte[12].Format("GM Message System");
    m_strOptionsTexte[13].Format("Jail System");
    m_strOptionsTexte[14].Format("Pseudoname System");
    m_strOptionsTexte[15].Format("CC Shortcut System");
    m_strOptionsTexte[16].Format("XPStat System");
    m_strOptionsTexte[17].Format("Bank management");
    m_strOptionsTexte[18].Format("ScrollXP Management");
	 m_strOptionsTexte[19].Format("Antiplug System");
   
    
	_LOG_DEBUG
		LOG_DEBUG_LVL4,
		"Connecting to ODBC authentication database."
	LOG_

	// Create the ODBC interface.
	ODBCAuth.Connect( theApp.sAuth.csODBC_DSN, theApp.csDBUser, theApp.csDBPwd );
	ODBCAuth.Cancel();

	_LOG_DEBUG
		LOG_DEBUG_LVL4,
		"Setting up auto-registry callback"
	LOG_

	CAutoConfig::AddRegUpdateCallback( MaxUserUpdate );

    // Get the 'max users' variables
    MaxUserUpdate();
}
/******************************************************************************/
//  Destroys the TFCMessageHandler
void TFCMessagesHandler::Destroy( void )
/******************************************************************************/
{
}
/******************************************************************************/
// Called when the there is a registry update and we must update the max users.
void TFCMessagesHandler::MaxUserUpdate( void )
/******************************************************************************/
{
	LPLOCAL_USER lpLocalUser;
	RegKeyHandler regKey;
	regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+MAX_USER_KEY );

	tlLocalUsers.Lock();

	// Destroy the current local user list.
	tlLocalUsers.AnnihilateList();

	dwMaxLocalUsers  = regKey.GetProfileInt( "LocalUserMax",  65536 );
	dwMaxRemoteUsers = regKey.GetProfileInt( "RemoteUserMax", 65536 );

    int i = 1;
	// Fetch all local ip definitions.
	CString csIP = regKey.GetProfileString( "LocalUserIP1", "$NULL$" );
	while( csIP != "$NULL$" )
	{
		lpLocalUser = new LOCAL_USER;

		lpLocalUser->dwIP      = inet_addr( (LPCTSTR)csIP );
		TRACE( "\r\nFoundIP=%u", lpLocalUser->dwIP );
		if( lpLocalUser->dwIP != 0xFFFFFFFF )
		{
			csIP.Format( "LocalUserNetmask%u", i );                
			
			lpLocalUser->dwNetmask = inet_addr( regKey.GetProfileString( (LPCTSTR)csIP, "255.255.255.0" ) );
			if( lpLocalUser->dwNetmask != 0xFFFFFFFF )
			{
				TRACE( "\r\nFoundNetmask=%d", lpLocalUser->dwNetmask );

				// Add the local user definition.
				tlLocalUsers.AddToTail( lpLocalUser );
			}
			else
			{
				if (lpLocalUser != NULL)
				{
					delete lpLocalUser;
					lpLocalUser = NULL;
				}
			}
		}
		else
		{
			if (lpLocalUser != NULL)
			{
				delete lpLocalUser;
				lpLocalUser = NULL;
			}
		}
		csIP.Format( "LocalUserIP%u", ++i );
		csIP = regKey.GetProfileString( (LPCTSTR)csIP, "$NULL$" );
	}

	regKey.Close();
	regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+PATCH_KEY );

	tlLocalUsers.Unlock();

    vPatchServers.Lock();

	// Flush old patch server listing
	vPatchServers.erase( vPatchServers.begin(), vPatchServers.end() );

	i = 1;
	// Fetch all local ip definitions.
	csIP = regKey.GetProfileString( "OtherServer1_IP", "$NULL$" );
	while( csIP != "$NULL$" )
	{
		PATCH_SERVER pPatch;

		pPatch.csIP = (LPCTSTR)csIP;               

		csIP.Format( "OtherServer%u_Port", i );                
		pPatch.wPort = static_cast< WORD >( regKey.GetProfileInt( (LPCTSTR)csIP, 11679 ) );

		TRACE( "\r\nFound patch server at %sp%u", (LPCTSTR)pPatch.csIP, pPatch.wPort );

		vPatchServers.push_back( pPatch );

		csIP.Format( "OtherServer%u_IP", ++i );      
		csIP = regKey.GetProfileString( (LPCTSTR)csIP, "$NULL$" );
	}

	vPatchServers.Unlock();

    // New web patch.
	webPatchServer.Lock();

	// Fetch all local ip definitions.    
	webPatchServer.csIP           = regKey.GetProfileString( "WebPatch_URL", "" );                
	webPatchServer.csImagePath    = regKey.GetProfileString( "WebPatch_ImageFile", "" );
   webPatchServer.csIPNew        = regKey.GetProfileString( "WebPatchNew_URL", "" );                
   webPatchServer.csImagePathNew = regKey.GetProfileString( "WebPatchNew_ImageFile", "" );
	webPatchServer.csUserName     = regKey.GetProfileString( "WebPatch_User", "" );
	webPatchServer.csPassword     = regKey.GetProfileString( "WebPatch_Pwd", "" );

	webPatchServer.Unlock();

	{
		RegKeyHandler regKey;
		regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+PATHS_KEY );

      csMotDMessage = " ";
		csMotD = regKey.GetProfileString( "MOTD", "\\INVALID\\" );
      if( csMotD != "\\INVALID\\" )
      {
         HANDLE hFile = CreateFile((LPCTSTR)csMotD,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
         // If file could be read.
         if( hFile != INVALID_HANDLE_VALUE )
         {
            LPBYTE lpBuffer = (LPBYTE)csMotDMessage.GetBuffer( GetFileSize( hFile, NULL ) );
            DWORD dwRead = 0;

            ReadFile( hFile, lpBuffer, GetFileSize( hFile, NULL ) - 1, &dwRead, NULL );           

            csMotDMessage.ReleaseBuffer( GetFileSize( hFile, NULL ) - 1 );

            TRACE( "\r\nMOTD = %s.", csMotDMessage );
            CloseHandle( hFile );
         }
      }
	}

	regKey.Close();
	regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+CHARACTER_KEY );

	ChatterChannels &chatter =  CPlayerManager::GetChatter();
   chatter.Lock();

   chatter.ClearSystemChannels();

   CString key;
   i = 2;
   string channelId = regKey.GetProfileString( "PublicChannel1", "$NULL$" );
   while( channelId != "$NULL$" )
   {
      chatter.AddSystemChannel( channelId );

      key.Format( "PublicChannel%u", i );
      i++;
      channelId = regKey.GetProfileString( key, "$NULL$" );
   }

   CPlayerManager::RefreshSystemChannels();

   chatter.Unlock();
}
/******************************************************************************/
// Registers a function in the jump table
void TFCMessagesHandler::RegisterFunction(
 LPPACKET_FUNC lpFunc,	// Function associated with request
 RQ_SIZE rqRequestID	// Request type
)
/******************************************************************************/
{
	if( rqRequestID < RQ_QUANTITY)
	{
		try
		{
			FuncTable[ rqRequestID ].Setup( lpFunc );
		}
		catch(...)
		{
			_LOG_DEBUG
				LOG_CRIT_ERRORS,
				"Crashed setting up packet function table. Probable cause: performance counters initialization."
			LOG_
			throw;
		}
	}
}
/******************************************************************************/
// Dispatches a message to a specific packet handler
void TFCMessagesHandler::DispatchPacket(PACKET_FUNC_PROTOTYPE)
/******************************************************************************/
{		
	TFormat format;

	// send the packet
	BOOL boInvalid = FALSE;
	//TRACE( "\r\n** Received packet ID %u, packet is ", rqRequestID );
	if( rqRequestID > 0 && rqRequestID < RQ_QUANTITY)
	{
		if( FuncTable[ rqRequestID ].GetFunc() != NULL )
		{
			//TRACE( "valid." );
			try
			{
				try
				{
					( FuncTable[ rqRequestID ].GetFunc() )( PACKET_FUNC_PARAM );
				}
				catch(...)
				{
					_LOG_DEBUG
						LOG_CRIT_ERRORS,
						"Detected crash in TFCMessagesHandler::DispatchPacket, requestID %u, while calling request's function!.", 
						rqRequestID
					LOG_
					throw;
				}

			}
			catch(...)
			{
            _LOG_DEBUG
               LOG_CRIT_ERRORS,
               "Detected crash in TFCMessagesHandler, request %u.", 
               rqRequestID
            LOG_
            if( user != NULL )
            {
               _LOG_DEBUG
                  LOG_CRIT_ERRORS,
                  "Happened while handling account %s.",
                  (LPCTSTR)user->GetFullAccountName()
               LOG_
               if( user->in_game )
               {
                  _LOG_DEBUG
                     LOG_CRIT_ERRORS,
                     "User was in game under character %s (ID %u).",
                     (LPCTSTR)user->self->GetTrueName(),user->self->GetID()
                     LOG_
               }
            }
            throw;
			}
		}
		else
		{
			boInvalid = TRUE;
		}
	}
	else
	{
		boInvalid = TRUE;
	}

	if( boInvalid )
	{
        if( user != NULL )
		{
		    if( user->registred )
			{
			    if( user->in_game )
				{
				    _LOG_DEBUG LOG_DEBUG_LVL1, 
					    "Received an unregistered packet ID %u from user %s "
					    "playing character %s, IP %s.", 
					    rqRequestID,
					    (LPCTSTR)user->GetFullAccountName(),
					    (LPCTSTR)user->self->GetTrueName(),
					    (LPCTSTR)user->GetIP()
				    LOG_
			    }
				else
				{
				    _LOG_DEBUG LOG_DEBUG_LVL1, 
					    "Received an unregistered packet ID %u from user %s"
					    ", IP %s.", 
					    rqRequestID,
					    (LPCTSTR)user->GetFullAccountName(),
					    (LPCTSTR)user->GetIP()
				    LOG_
			    }            
		    }
			else
			{
			    _LOG_DEBUG LOG_DEBUG_LVL1, 
				    "Received an unregistered packet ID %u"
				    ", from IP %s.", 
				    rqRequestID,
				    (LPCTSTR)user->GetIP()
			    LOG_						
		    }
        }
	}
}


// Called once the player has been successfully authenticated.
bool LoadPlayer(LPRQSTRUCT_REGISTER_ACCOUNT lpStruct,CString &csErrorMsg,TFCPacket &sending, long &LongKeyCode)
{
   bool boAccountSuccessfullyLoggedOn = false;

   _LOG_DEBUG
      LOG_DEBUG_LVL3,
         "Creating player %s.",
         (LPCTSTR)lpStruct->csAccount
      LOG_

   Players *lpPlayer = CPlayerManager::CreatePlayer( lpStruct->sockAddrO,lpStruct->sockAddrI, lpStruct->csAccount );
   if( lpPlayer != NULL )
   {
      _LOG_DEBUG
         LOG_DEBUG_LVL3,
            "Successfully created player %s",
            (LPCTSTR)lpStruct->csAccount
         LOG_

      lpPlayer->SetPwd("");
      lpPlayer->SetAccount( lpStruct->csAccount );
      lpPlayer->SetLang( lpStruct->usLangUsed ); // Ajout du système multilingue.

      if( lpPlayer->LoadAccount( lpStruct->csAccount ))
      {
         // Was user locked out temporary and can unlock ???
         if( lpPlayer->boLockedOut  && lpPlayer->uiLockedComplete >0)
         {
            time_t TimeMsTmp =  time(NULL);
            if(TimeMsTmp > lpPlayer->uiLockedComplete)
            {
               lpPlayer->boLockedOut = FALSE;
            }
         }

         if( lpPlayer->boLockedOut  && lpPlayer->uiLockedComplete >0)
         {
            time_t TimeMsTmp =  time(NULL);
            int diff = lpPlayer->uiLockedComplete - TimeMsTmp;
            lpPlayer->registred      = FALSE;
            lpPlayer->dwExitDecompte = 0; //user locker



            //csErrorMsg = _DEFAULT_STR( 452 );
            sending << (char)1;

            //csErrorMsg.Format("%s%d%s",_DEFAULT_STR( 15034),(int)(diff/86400) ,_DEFAULT_STR( 15033));/*86400*/

            int iNbrDay = diff/86400;
            int iNbrH   = (diff-(iNbrDay*86400))/3600;
            int iNbrM   = ((diff-(iNbrDay*86400))-(iNbrH*3600))/60;
            csErrorMsg.Format(_DEFAULT_STR( 15149),iNbrDay,iNbrH,iNbrM);

            _LOG_WORLD
               LOG_WARNING, 
               "User %s at IP %s tried to log on server. This account is currently locked-out.",
               (LPCTSTR)lpStruct->csAccount,
               lpPlayer->GetIP()
               LOG_

         }
         else if( lpPlayer->boLockedOut ) 
         {
            lpPlayer->registred = FALSE;
            lpPlayer->dwExitDecompte = 0; //user locker
            sending << (char)1;
            csErrorMsg = _DEFAULT_STR( 452 );
            _LOG_WORLD
               LOG_WARNING, 
                  "User %s at IP %s tried to log on server. This account is currently locked-out.",
                  (LPCTSTR)lpStruct->csAccount,
                  lpPlayer->GetIP()
               LOG_
         }
         else
         {
            //Check Credit time
            if(!lpPlayer->CanPaidPlay())
            {
               lpPlayer->registred = FALSE;
               lpPlayer->dwExitDecompte = 0; //user dont have pay time
               sending << (char)1;
               csErrorMsg = _DEFAULT_STR( 15256 );
               _LOG_WORLD
                  LOG_WARNING, 
                     "User %s at IP %s tried to log on server. This account is not paied and try play on paid time",
                     (LPCTSTR)lpStruct->csAccount,
                     lpPlayer->GetIP()
                  LOG_
            }
            else  if(!lpPlayer->CanConnectGMOnly())
            {
               lpPlayer->registred = FALSE;
               lpPlayer->dwExitDecompte = 0; //Server GM Only now
               sending << (char)1;
               csErrorMsg = _DEFAULT_STR( 15255 );
            }
            else
            {
               _LOG_WORLD
                  LOG_MISC_1,
                     "User %s just logged in from IP( %s p%u ). %u users online!",
                     (LPCTSTR)lpStruct->csAccount,
                     (LPCTSTR)lpPlayer->GetIP(),
                     (LPCTSTR)lpPlayer->GetPort(),
                     CPlayerManager::GetUserCount()
                  LOG_

               if( CPlayerManager::VerifyPlayerUnique( lpPlayer ) )
               {
                  TRACE( "\r\nUser registred\r\n" );
                  csErrorMsg = _DEFAULT_STR( 453 );
                  sending << (char)0;
                  LongKeyCode = rnd(1,999999999);
                  lpPlayer->SetKeyCode(LongKeyCode);
                  boAccountSuccessfullyLoggedOn = true;

               }
               else
               {
                  lpPlayer->registred       = FALSE;
                  lpPlayer->dwExitDecompte = 0; //Player not unique
                  csErrorMsg = _DEFAULT_STR( 454 );
                  sending << (char)2;
               }
            }
         }
      }
      else
      {
         _LOG_WORLD
            LOG_WARNING,
               "Could not load user information for %s at IP %s. Access denied.",
               (LPCTSTR)lpStruct->csAccount,
               inet_ntoa( lpStruct->sockAddrO.sin_addr )
            LOG_
         csErrorMsg = _DEFAULT_STR( 455 );
         lpPlayer->registred       = FALSE;
         lpPlayer->dwExitDecompte = 0; //Unable load account
         sending << (char)1;
      }

      lpPlayer->Unlock();
      //CPlayerManager::FreePlayerResourceFct( lpPlayer );
   }
   else
   {
      // not loaded?
      _LOG_DEBUG
         LOG_DEBUG_LVL3,
            "Could NOT created player %s",
            (LPCTSTR)lpStruct->csAccount
         LOG_

      // User was probably already online. Accept it.
      csErrorMsg = _DEFAULT_STR( 453 );
      sending << (char)0;
      boAccountSuccessfullyLoggedOn = true;
   }

   return boAccountSuccessfullyLoggedOn;
}


// Asynchronious function which will load the player.
void AsyncRQFUNC_PutPlayerInGame( LPVOID lpData ) 
{
   LPASYNC_PACKET_FUNC_PARAMS lpParams = (LPASYNC_PACKET_FUNC_PARAMS)lpData;
   Players *user = lpParams->user;

   // Makes sure the user always unlocks its UseLock wherever this function exits.
   struct AutoExit
   {
      AutoExit( Players *theUser, LPASYNC_PACKET_FUNC_PARAMS theParams ) 
         : user( theUser ), lpParams( theParams )
      {
      }
      // Auto cleans.
      ~AutoExit()
      {
         // Unlock user's UseLock
         user->UseUnlock(__FILE__, __LINE__);
         // Delete the parameters.
         if (lpParams != NULL)
         {
            delete lpParams;
            lpParams = NULL;
         }
      }
   private:
      Players *user;
      LPASYNC_PACKET_FUNC_PARAMS lpParams;
   }// Auto object. 

   cAutoExit( user, lpParams );

   // If user is already in_game, then its CERTAIN that the user has knowledge of being in_game.
   if( user->in_game )
   {
      return;
   }

   //user->Lock();	
   _LOG_DEBUG
      LOG_DEBUG_LVL1,
         "Waiting for character %s memaddr( 0x%x ) to save.",
         lpParams->strParam
      LOG_

   user->self->WaitForSaving();

   #ifdef BUILD_NMS_CUSTOM_NPC//ok
      user->self->ValidNMSGold();//valid els account NMS
   #endif

   char receive = 1;
   if( user->registred )
   {
      // If this packet wasn't already sent before.
      if( !user->boPreInGame )
      {
         _LOG_DEBUG
            LOG_DEBUG_LVL1,
               "Loading character %s data.",
               lpParams->strParam
            LOG_
         // Load player.
         receive = user->self->load_character( lpParams->strParam, user->GetAccount(), 0);
         _LOG_DEBUG
            LOG_DEBUG_LVL1,
               "Finished loading return %d.",receive
            LOG_
      }
      else
      {
         receive = 0;// Otherwise do as if we were loading the player.
      }
   }

  WorldPos player_pos;
   player_pos = user->self->GetWL();
   if(player_pos.world < TFCMAIN::GetMaxWorlds())
   {
      WorldMap *world = TFCMAIN::GetWorld(player_pos.world);
      if( world != NULL && world->IsValidPosition(player_pos) )
      {
         if(receive == 0)
         {	
            user->self->StartPutPlayerInGame();
            _LOG_WORLD
               LOG_MISC_1,
                  "Character %s ( user %s ) just entered the realm at %u, %u, %u!",
                  lpParams->strParam,
                  user->GetFullAccountName(),
                  user->self->GetWL().X,
                  user->self->GetWL().Y,
                  user->self->GetWL().world
               LOG_

            
            user->self->StartViewBackpack2(true,0); //backpack
            
            TFCPacket sendingS1;
            user->self->PacketStatus(sendingS1);
            user->self->SendPlayerMessage(sendingS1);

            TFCPacket sendingS2;
            user->self->PacketStatus(sendingS2);
            user->self->SendPlayerMessage(sendingS2);

            //Set le mode RP a tous les GM connecter par defaut
            if(user->IsGod())
               user->self->SetFlag(__FLAG_RPHRP_STATUS,1);

            user->m_dwXPLastTickCounter  = user->m_dwLastTickMove = GetTickCount();
            user->m_dwDPSLastTickCounter = user->m_dwLastTickMove = GetTickCount();
            user->m_XPCounter            = user->self->GetXP();
            user->m_DPSCounter           = 0;
            user->self->SetFlag(__FLAG_UNIT_COLOR,user->self->ViewFlag(__FLAG_UNIT_COLOR_OLD));


            TemplateList <TFCPacket> theList;
            user->boPreInGame = TRUE;
            user->in_game = FALSE;

            user->BeginSession();

            TFCPacket sending;
            int read;
            read = world->packet_inview_units( player_pos, sending, _DEFAULT_RANGE, user->self );
            if( read != 0 )
            {
               user->self->SendPlayerMessage( sending );
            }
            user->SetNextSave();
         }
      }
      else
      {
      }
   }
   else
   {
      TRACE(_T("Player file corrupted"));
      user->in_game = FALSE;
      user->self->reset_character();
   }

   if( user->GetGodFlags() & GOD_CAN_RUN_CLIENT_SCRIPTS )
   {
      TFCPacket sending;
      sending << (RQ_SIZE)RQ_GodFlagUpdate;
      sending << (char)UPDATE_GOD_CAN_RUN_CLIENT_SCRIPTS;
      sending << (char)1;

      user->self->SendPlayerMessage( sending );
   }
   if( user->GetGodFlags() & GOD_CAN_SLAY_USER )
   {
      TFCPacket sending;
      sending << (RQ_SIZE)RQ_GodFlagUpdate;
      sending << (char)UPDATE_GOD_CAN_SLAY_USER;
      sending << (char)1;

      user->self->SendPlayerMessage( sending );
   }

   WeatherEffect::GetInstance()->CheckWeatherState( user->self, user->self->GetWL() );

   // Send a welcome message to the player
   RegKeyHandler regKey;     
   if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY ) )
   {
      // Get the text to send
      CString csWelcome;
      csWelcome = regKey.GetProfileString( "WelcomeMessage", "" );		

      // If there is a text, send it
      if( !csWelcome.IsEmpty() )
      {
         TFCPacket sending;

         sending << (RQ_SIZE)RQ_ServerMessage;
         sending << (short)30;
         sending << (short)3;
         sending << csWelcome;
         sending << (long)CL_BLUE_LIGHT;

         user->Lock();
         user->self->SendPlayerMessage( sending );
         user->Unlock();		
      }

      regKey.Close();
   }
}

// Asynchronously registers a player using ODBC.
void TFCMessagesHandler::AsyncRQFUNC_RegisterAccountODBC( LPVOID lpData)
{
   LPRQSTRUCT_REGISTER_ACCOUNT lpStruct = (LPRQSTRUCT_REGISTER_ACCOUNT)lpData;

   // ***********************************************************
   // SQL-Injektion 
   bool blogCheat = false;

   CString accountName;
   CString accountNameOri;
   accountNameOri = lpStruct->csAccount;
   accountName.Format("%s", lpStruct->csAccount);
   if(accountName.Find(';') != -1)
      blogCheat = true;
   if(accountName.Find(',') != -1)
      blogCheat = true;
   if(accountName.Find('\'') != -1)
      blogCheat = true;


   accountName.Remove(';');
   accountName.Remove(',');
   accountName.Remove('\'');

   if (accountName.GetLength() > 20)
   {
      lpStruct->csAccount.Format("%s", accountName.Left(20));
      blogCheat = true;
   }
   else
   {
      lpStruct->csAccount.Format("%s", accountName);
   }

   if(blogCheat)
   {
      _LOG_CHEAT
         LOG_MISC_1,
         "Player (IP:%s) TRY SQL Injection : %s",
          inet_ntoa( lpStruct->sockAddrO.sin_addr ),
         accountNameOri
         LOG_
   }

   // ***********************************************************

   TFCPacket sending;
   bool    bOK = true;
   CString csErrorMsg;

   sending.SetPacketSeedID( lpStruct->packetSeedID );
   sending << (RQ_SIZE)RQ_RegisterAccount;

   // Allow multi account per IP?
   RegKeyHandler regKey;   
   int nMaxAccountPerIP = 0;
   long LongKeyCode = 0;
   if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+AUTH_KEY ) )
   {
      // Get the value		
      nMaxAccountPerIP = regKey.GetProfileInt( "NO_MULTI_ACC_PER_IP", 0 );
      regKey.Close();
   }

   // If the user cap has been reached.
   if( CPlayerManager::GetUserCount() >= (int)GetUserMax( lpStruct->sockAddrO ) )
   {
      bOK = false;
      csErrorMsg = _DEFAULT_STR( 451 );
      sending << (char)1;            
   }
   else if( lpStruct->csAccount.GetLength() <= 1 )
   {
      bOK = false;
      csErrorMsg = _DEFAULT_STR( 461 );
      sending << (char)1;
   }
   // If multi account per IP isn't allowed
   else if( (nMaxAccountPerIP != 0) && (Players::IPLogged( inet_ntoa( lpStruct->sockAddrO.sin_addr ) ) >= nMaxAccountPerIP) )
   {
      bOK = false;
      csErrorMsg = "An account is already logged in with your IP.";
      sending << (char)1;
   }
   // Otherwise if the user is already logged on this, or another server.    
   else if( Players::AccountLogged( lpStruct->csAccount, inet_ntoa( lpStruct->sockAddrO.sin_addr ) ) )
   {
      bOK = false;
      // If player is logged on a server.
      csErrorMsg = _DEFAULT_STR( 2845 );
      sending << (char)1;
   }
   else
   {
      bool boAccountSuccessfullyLoggedOn = false;

      // Do authentification
      CString csQuery;
      BOOL boAuth = TRUE;
      BOOL boDBError = FALSE;

      char szPassword[ 250 ];
      szPassword[ 0 ] = 0;
      char cSendingCode = 0;
      char szAccount[ 1024 ];
      lpStruct->csAccount.MakeLower();
      Players::QuotedAccount( szAccount, lpStruct->csAccount );

      ODBCAuth.Lock();
      csQuery.Format("SELECT %s FROM %s WHERE %s='%s'",theApp.sAuth.csODBC_Pwd,theApp.sAuth.csODBC_Table,theApp.sAuth.csODBC_Account,szAccount);
      
      // Append the supplied where statement if it exists.
      if( !theApp.sAuth.csODBC_Where.IsEmpty() )
      {
         csQuery += " AND (";
         csQuery += theApp.sAuth.csODBC_Where;
         csQuery += ")";
      }

      // Send the request
      if( ODBCAuth.SendRequest( (LPCTSTR)csQuery ) )
      {                        
         if( ODBCAuth.Fetch() )
         {
            // Fetch password.
            ODBCAuth.GetString( 1, szPassword, 250 );
         }
         else
         {
            csErrorMsg = _DEFAULT_STR( 462 );
            boAuth = FALSE;
         }
      }
      else
      {
         boDBError = TRUE;
         boAuth = FALSE;
         csErrorMsg = _DEFAULT_STR( 459 );
      }                    

      // If no DB error occured
      ODBCAuth.Cancel();
      ODBCAuth.Unlock();

      // If password encryption has been requested
      if( theApp.dwEncryptedPassword )
      {
         // TODO
      }

      // Check password (case sensitive).
      if( theApp.dwPasswordCaseSensitive && boAuth && strcmp( szPassword, (LPCTSTR)lpStruct->csPassword ) != 0 )
      {
         csErrorMsg = _DEFAULT_STR( 458 );
         boAuth = FALSE;
      }

      // Check password
      else if( boAuth && _stricmp( szPassword, (LPCTSTR)lpStruct->csPassword ) != 0 )
      {
         csErrorMsg = _DEFAULT_STR( 458 );
         boAuth = FALSE;
      }

      // User registered
      if( boAuth )
      {
         boAccountSuccessfullyLoggedOn = LoadPlayer( lpStruct, csErrorMsg, sending ,LongKeyCode);
      }
      else
      {
         // Database out
         if( boDBError )
         {
            _LOG_PC
               LOG_CRIT_ERRORS, 
                  "User %s was denied access because ODBC authentication server "
                  "did not answer.",
                  (LPCTSTR)lpStruct->csAccount
               LOG_
            sending << (char)1;
         }
         else
         {
            sending << (char)1;
         }
      }
      // If the account did not successfully log on.
      if( !boAccountSuccessfullyLoggedOn )
      {
         bOK = false;
         // Notify the players that the loggon failed.
         Players::AccountLoggonFailed( lpStruct->csAccount );
      }
   }

   sending << csErrorMsg;
   sending << (long)LongKeyCode;

   WorldPos wlPos = { -1, -1, -1 };
   CPacketManager::SendPacket( sending, lpStruct->sockAddrO,lpStruct->sockAddrI, -1, wlPos, FALSE ); //OK

   if (lpStruct != NULL)
   {
      delete lpStruct;
      lpStruct = NULL;
   }
}



void TFCMessagesHandler::AsyncRQFUNC_QueryNameExistence( LPVOID lpData )// Data. 
{
   RQSTRUCT_QueryNameExistence *rq = reinterpret_cast< RQSTRUCT_QueryNameExistence * >( lpData );

   TFCPacket sending;

   if( !Character::IsNameValid( rq->name ) )
   {
      sending << (RQ_SIZE)RQ_QueryNameExistence;
      sending << (char)2; // name is invalid.
   }
   else
   {
      if( Players::NameExists( std::string( rq->name ) ) )
      {
         sending << (RQ_SIZE)RQ_QueryNameExistence;
         sending << (char)1; // name exists
      }
      else
      {
         sending << (RQ_SIZE)RQ_QueryNameExistence;
         sending << (char)0;
      }
   }

   // Send packet to requesting IP.
   WorldPos wlPos = {-1,-1,-1};
   CPacketManager::SendPacket( sending, rq->sockAddrO,rq->sockAddrI, -1, wlPos, FALSE ); //OK

   // Delete name string allocated in RQFUNC_QueryNameExistence.
   if (rq != NULL)
   {
      delete rq;
      rq = NULL;
   }
}

// Asyn deletion function
void AsyncRQFUNC_DeletePlayer( LPVOID lpData )
{
   LPASYNC_PACKET_FUNC_PARAMS lpStruct = (LPASYNC_PACKET_FUNC_PARAMS)lpData;
   Players *user = lpStruct->user;
   TFCPacket sending;

   // Wait for saving.
   user->self->WaitForSaving();

   sending << (RQ_SIZE)RQ_DeletePlayer;

   sending << user->self->DeleteCharacter( lpStruct->strParam, user->GetAccount());			
   user->self->SendPlayerMessage( sending );

   user->UseUnlock(__FILE__, __LINE__);
   if (lpStruct != NULL)
   {
      delete lpStruct;
      lpStruct = NULL;
   }
}

// Asynchronously create player.
void AsyncRQFUNC_CreatePlayer( LPVOID lpData ) 
{
   LPRQSTRUCT_CREATE_PLAYER lpStruct = (LPRQSTRUCT_CREATE_PLAYER)lpData;

   TFCPacket sending;
   Players *user = lpStruct->sParams.user;	
   char receive;

   // Wait for saving.
   user->self->WaitForSaving();

   receive = user->self->load_character( lpStruct->sParams.strParam, user->GetAccount(), lpStruct->lpbAnswers );
   sending << (RQ_SIZE)RQ_CreatePlayer;
   sending << (char)receive;		
   user->self->packet_stats(sending);			
   user->self->SendPlayerMessage( sending );

   user->UseUnlock(__FILE__, __LINE__);
   if (lpStruct != NULL)
   {
      delete lpStruct;
      lpStruct = NULL;
   }
}

void AsyncRQ_GetPersonnalPCList( LPVOID lpData ) 
{
   LPRQSTRUCT_CREATE_PLAYER lpStruct = (LPRQSTRUCT_CREATE_PLAYER)lpData;

   TFCPacket sending;
   Players *user = lpStruct->sParams.user;	

   try
   {
      user->BuildAccountPlayerListEquipAsync();

      if (lpStruct != NULL)
      {
         delete lpStruct;
         lpStruct = NULL;
      }

   }
   catch (...)
   {
      
   }
}

void AsyncRQFUNC_RQ_RP_BroadCastRP( LPVOID lpData ) 
{
   LPASYNC_PACKET_FUNC_PARAMSEX lpStruct = (LPASYNC_PACKET_FUNC_PARAMSEX)lpData;

   RPMaster::RPBroadcastInfo(lpStruct->user);

   if (lpStruct != NULL)
   {
      delete lpStruct;
      lpStruct = NULL;
   }
}

void AsyncRQFUNC_RQ_RP_CreerRP( LPVOID lpData ) 
{
   LPASYNC_PACKET_FUNC_PARAMSEX lpStruct = (LPASYNC_PACKET_FUNC_PARAMSEX)lpData;


   RPMaster::CreateNewRP(lpStruct->user,lpStruct->strParam1);

   if (lpStruct != NULL)
   {
      delete lpStruct;
      lpStruct = NULL;
   }
}

void AsyncRQFUNC_RQ_RP_TerminerRP( LPVOID lpData ) 
{
   LPASYNC_PACKET_FUNC_PARAMSEX lpStruct = (LPASYNC_PACKET_FUNC_PARAMSEX)lpData;

   RPMaster::RPInteractionTerminate(lpStruct->user);

   if (lpStruct != NULL)
   {
      delete lpStruct;
      lpStruct = NULL;
   }
}

void AsyncRQFUNC_RQ_RP_RejoindreRP( LPVOID lpData ) 
{
   LPASYNC_PACKET_FUNC_PARAMSEX lpStruct = (LPASYNC_PACKET_FUNC_PARAMSEX)lpData;

   RPMaster::RPRejoindreRP(lpStruct->user,lpStruct->iParam1);

   if (lpStruct != NULL)
   {
      delete lpStruct;
      lpStruct = NULL;
   }
}

void AsyncRQFUNC_RQ_RP_RejoindreAnswerRP( LPVOID lpData ) 
{
   LPASYNC_PACKET_FUNC_PARAMSEX lpStruct = (LPASYNC_PACKET_FUNC_PARAMSEX)lpData;

   RPMaster::RPRejoindreRPResult(lpStruct->user,lpStruct->iParam1);

   if (lpStruct != NULL)
   {
      delete lpStruct;
      lpStruct = NULL;
   }
}

void AsyncRQFUNC_RQ_RP_ExpulserRP( LPVOID lpData ) 
{
   LPASYNC_PACKET_FUNC_PARAMSEX lpStruct = (LPASYNC_PACKET_FUNC_PARAMSEX)lpData;

   RPMaster::RPInteractionExpluser(lpStruct->user,lpStruct->iParam1);

   if (lpStruct != NULL)
   {
      delete lpStruct;
      lpStruct = NULL;
   }
}

void AsyncRQFUNC_RQ_RP_InviteRP( LPVOID lpData ) 
{
   LPASYNC_PACKET_FUNC_PARAMSEX lpStruct = (LPASYNC_PACKET_FUNC_PARAMSEX)lpData;

   RPMaster::RPInviteRP(lpStruct->user,lpStruct->iParam1);

   if (lpStruct != NULL)
   {
      delete lpStruct;
      lpStruct = NULL;
   }
}


void AsyncRQFUNC_RQ_RP_InviteAnswerRP( LPVOID lpData ) 
{
   LPASYNC_PACKET_FUNC_PARAMSEX lpStruct = (LPASYNC_PACKET_FUNC_PARAMSEX)lpData;

   RPMaster::RPInviteRPResult(lpStruct->user,lpStruct->iParam1);

   if (lpStruct != NULL)
   {
      delete lpStruct;
      lpStruct = NULL;
   }
}


void AsyncRQFUNC_RQ_RP_SignalerRP( LPVOID lpData ) 
{
	LPASYNC_PACKET_FUNC_PARAMSEX lpStruct = (LPASYNC_PACKET_FUNC_PARAMSEX)lpData;

	RPMaster::RPSignalerRP(lpStruct->user,lpStruct->iParam1);

	if (lpStruct != NULL)
	{
		delete lpStruct;
		lpStruct = NULL;
	}
}








void TFCMessagesHandler::RQFUNC_MessageOfDay( PACKET_FUNC_PROTOTYPE )
{
   RQ_HEADER;
   TFCPacket sending;
   sending.SetPacketSeedID( msg->GetPacketSeedID() );
   sending << (RQ_SIZE)RQ_MessageOfDay;
   sending << csMotDMessage;
   WorldPos wlPos = { -1, -1, -1 };
   CPacketManager::SendPacket( sending, sockAddrO,sockAddrI, -1, wlPos, FALSE );//OK
   RQ_FOOTER( "RQ_MessageOfTheDay" );
}

void TFCMessagesHandler::RQFUNC_QueryServerVersion( PACKET_FUNC_PROTOTYPE )
{
   RQ_HEADER;
   TFCPacket sending;    
   sending << (RQ_SIZE)RQ_QueryServerVersion;
   sending << static_cast< long >( TFCServer->dwVersion );
   vPatchServers.Lock();
   sending << static_cast< short >( vPatchServers.size() );
   vector< PATCH_SERVER >::iterator i;
   for( i = vPatchServers.begin(); i != vPatchServers.end(); i++ )
   {
      if( (*i).wPort == 0 )
      {
         sending << static_cast< short >( 11679 );
      }
      else
      {
         sending << static_cast< short >( (*i).wPort );
      }
      sending << (*i).csIP;
   }

   vPatchServers.Unlock();
   WorldPos wlPos = { -1, -1, -1 };
   CPacketManager::SendPacket( sending, sockAddrO,sockAddrI, -1, wlPos, FALSE ); //OK
   RQ_FOOTER( "RQ_QueryServerVersion" );
}

void TFCMessagesHandler::RQFUNC_QueryPatchServerInfo( PACKET_FUNC_PROTOTYPE )
{
   RQ_HEADER;

   TFCPacket sending;

   sending.SetPacketSeedID( msg->GetPacketSeedID() );

   sending << (RQ_SIZE)RQ_QueryPatchServerInfo;
   sending << static_cast< long >( TFCServer->dwVersion );

   webPatchServer.Lock();
   sending << webPatchServer.csIP;        
   sending << webPatchServer.csImagePath;
   sending << webPatchServer.csUserName;
   sending << webPatchServer.csPassword;
   sending << (short)IntlText::GetDefaultLng();
   webPatchServer.Unlock();

   WorldPos wlPos = { -1, -1, -1 };
   CPacketManager::SendPacket( sending, sockAddrO,sockAddrI, -1, wlPos, FALSE ); //OK

   RQ_FOOTER( "RQ_QueryPatchServerInfo" )
}

void TFCMessagesHandler::RQFUNC_QueryPatchServerInfo2( PACKET_FUNC_PROTOTYPE )
{
   RQ_HEADER;

   TFCPacket sending;

   sending.SetPacketSeedID( msg->GetPacketSeedID() );

   sending << (RQ_SIZE)RQ_QueryPatchServerInfo2;
   sending << static_cast< long >( TFCServer->dwVersion );

   webPatchServer.Lock();
   sending << webPatchServer.csIPNew;        
   sending << webPatchServer.csImagePathNew;
   sending << webPatchServer.csUserName;
   sending << webPatchServer.csPassword;
   sending << (short)IntlText::GetDefaultLng();
   webPatchServer.Unlock();

   WorldPos wlPos = { -1, -1, -1 };
   CPacketManager::SendPacket( sending, sockAddrO,sockAddrI, -1, wlPos, FALSE ); //OK

   RQ_FOOTER( "RQ_QueryPatchServerInfo2" )
}

void TFCMessagesHandler::RQFUNC_AuthenticateServerVersion( PACKET_FUNC_PROTOTYPE )
{
   RQ_HEADER;
   if( user->boLockedOut || !user->CanPaidPlay() || !user->CanConnectGMOnly())
   {
      return;
   }

   DWORD clientVersion = 0;
   GET_LONG( clientVersion );
   TFCPacket sending;

   sending.SetPacketSeedID( msg->GetPacketSeedID() );
   if( clientVersion == TFCServer->dwVersion )
   {
      sending << (RQ_SIZE)RQ_AuthenticateServerVersion;
      sending << (long)1;

      // Allow access to the user.
      user->registred = TRUE;
   }
   else
   {
      sending << (RQ_SIZE)RQ_AuthenticateServerVersion;
      sending << (long)0;
   }
   user->self->SendPlayerMessage( sending );
   RQ_FOOTER( "RQ_AuthenticateServerVersion" );
}











void TFCMessagesHandler::RQFUNC_Ack( PACKET_FUNC_PROTOTYPE )
{
}

void TFCMessagesHandler::RQFUNC_GodCreateObject(PACKET_FUNC_PROTOTYPE)
{
}

void TFCMessagesHandler::RQFUNC_GetSkillStatPoints(PACKET_FUNC_PROTOTYPE)
{
   /*
   RQ_HEADER;
   TFCPacket sending;
   // Returns a list of the online users.
   sending << (short)user->self->GetSkillPoints();
   sending << (short)user->self->GetStatPoints();
   user->self->SendPlayerMessage( sending );
   RQ_FOOTER( "RQ_GetSkillStatPoints" );
   */
}

void TFCMessagesHandler::RQFUNC_GoldChange(PACKET_FUNC_PROTOTYPE)
{
   /*
   RQ_HEADER;
   TFCPacket sending;
   // Returns the amount of gold on the player.
   sending << (RQ_SIZE)RQ_GoldChange;
   sending << (long)user->self->GetGold();
   user->self->SendPlayerMessage( sending );
   RQ_FOOTER( "RQ_GoldChange" );
   */
}

void TFCMessagesHandler::RQFUNC_ViewGroundItemIndentContent(PACKET_FUNC_PROTOTYPE)
{
}

void TFCMessagesHandler::RQFUNC_UpdateGroupMembers( PACKET_FUNC_PROTOTYPE )
{
}

void TFCMessagesHandler::RQFUNC_UpdateGroupInviteList(  PACKET_FUNC_PROTOTYPE )
{
}








// Message handler of packet type RQFUNC_
void TFCMessagesHandler::RQFUNC_SendPeriphericObjects(PACKET_FUNC_PROTOTYPE )
{
   RQ_HEADER;

   // This request sends the current time to the client,	
   if( user->in_game )
   {
      char     direction = 0;
      WorldPos where = {0,0,0};

      msg->Get( (char  *)&direction );
      msg->Get( (short *)&where.X );
      msg->Get( (short *)&where.Y );
      where.world = user->self->GetWL().world;

      WorldMap *wl = TFCMAIN::GetWorld( where.world );
      if( wl )
      {
         // If player asks for a valid position
         if( wl->IsValidPosition( where ) )
         {
            if(direction < 9 )
            {
               TFCPacket sending;
               wl->packet_peripheral_units(where, _DEFAULT_RANGE,(DIR::MOVE)direction, sending,user->self);
               user->self->SendPlayerMessage( sending );
            }	
         }
      }
   }
   RQ_FOOTER( "RQ_SendPeriphericObjects" );
}

void TFCMessagesHandler::RQFUNC_ViewBackpack2(PACKET_FUNC_PROTOTYPE )
{
   RQ_HEADER;
   // View backpack, returns the objects in the user's backpack	
   if(user && user->in_game || user->boPreInGame )
   {
      user->self->StartViewBackpack2(false,0); //backpack
   }
   RQ_FOOTER( "RQ_ViewBackpack2" );
}

void TFCMessagesHandler::RQFUNC_ViewInv(PACKET_FUNC_PROTOTYPE )
{
   RQ_HEADER;
   // View backpack, returns the objects in the user's backpack	
   if(user && user->in_game || user->boPreInGame )
   {
       user->self->StartViewBackpack2(true,1); //backpack, equip,stat
   }
   RQ_FOOTER( "RQ_ViewInv" );
}

// Message handler of packet type RQFUNC_ViewEquipped
void TFCMessagesHandler::RQFUNC_ViewEquipped(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;
   // View equiped objects, returns the objects equiped by the user, in the BodyPos order	
   if(user->in_game || user->boPreInGame)
   {
      TFCPacket sending;
      // puts the backpack into the packet		
      user->self->packet_equiped( sending );
      user->self->SendPlayerMessage( sending );
   }
   RQ_FOOTER( "RQ_ViewEquipped" );
}

void TFCMessagesHandler::RQFUNC_EquipObject(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;
   if(user && user->in_game)
   {
      unsigned long itemID;
      msg->Get((long *)(&itemID));
      user->self->StartAsyncEquipItem(itemID);
   }
   RQ_FOOTER( "RQ_EquipObject" );
}

void TFCMessagesHandler::RQFUNC_UnequipObject(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;
   if(user && user->in_game)
   {
      unsigned char position;
      msg->Get((char *)(&position));

      user->self->unequip_object((char)position);
      user->self->StartViewBackpack2(false,0); //backpack
   }
   RQ_FOOTER( "RQ_UnequipObject" );
}

void TFCMessagesHandler::RQFUNC_GetPersonnalPClist(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;
   if( user->boLockedOut || !user->CanPaidPlay() || !user->CanConnectGMOnly())
   {
      return;
   }

   // If user was previously in the reroll menu.
   if( user->boRerolling && !user->in_game ) //CV_VALID devrait ter OK ici cette validation de INGame a false
   {
      // Not rerolling anymore.
      user->boRerolling = FALSE;
      // Save rerolled data.
      user->self->SaveCharacter(TRUE,"GetPersonnalPClist"); //On getpersonnalPCList Player in REROLL menu... (not in game)
   }

   // This request sends the list of PCs attached to the player
   if(!user->in_game)
   {
      {
         // Tells the client how many characters each account can have, so it can display
         // the 'New Character' option according to server rules.
         TFCPacket sending;
         sending << (RQ_SIZE)RQ_MaxCharactersPerAccountInfo;
         sending << (char)TFCMAIN::GetMaxCharactersPerAccount();
         user->self->SendPlayerMessage( sending );
      }

      user->BuildAccountPlayerListAsync();

      if( theApp.dwSendConnectEquipEnable == 1)
      {
         // Prepare async deletion
         LPASYNC_PACKET_FUNC_PARAMS lpStruct = new ASYNC_PACKET_FUNC_PARAMS;			
         lpStruct->msg         = NULL;
         lpStruct->user        = user;
         lpStruct->rqRequestID = rqRequestID;
         lpStruct->strParam    = "";
         AsyncFuncQueue::GetMainQueue()->Call( AsyncRQ_GetPersonnalPCList, lpStruct );
      }
   }
   RQ_FOOTER( "RQ_GetPersonnalPCList" );
}

void TFCMessagesHandler::RQFUNC_IndirectTalk(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   if(user->self->ViewFlag( __FLAG_NMS_PLAYER_DEATH ) != 0)
      return;
   /////////////////////////////////////////////////////////////////////////////////////////////
   // This request handles non-personnalized talking.
   // If user is god.	
   BYTE bDirection = 0;
   DWORD dwID = 0;
   DWORD dwTextColor = 0;
   long lKey = 0;
   LPBYTE lpbText = NULL;


   if( user->NMCanTalk() && user->in_game)
   {
      GET_LONG( dwID );
      GET_BYTE( bDirection );
      GET_LONG( dwTextColor );
      GET_LONG( lKey );

      if(lKey != user->GetKeyCode())
      {
         _LOG_CHEAT
            LOG_MISC_1,
            "Player %s Account %s (IP:%s) Invalid Validation Key (RQ_IndirectedTalk)",
            (LPCTSTR)user->self->GetTrueName(),
            (LPCTSTR)user->GetAccount(),
            user->GetIP()
            LOG_

            user->dwKickoutTime = 2 SECONDS TDELAY;
      }
      else
      {
         GET_STRING( lpbText );
         user->self->StartAsyncIndirectTalk(lpbText,bDirection,dwTextColor,dwID);
         if (lpbText != NULL)
         {
            delete lpbText;
            lpbText = NULL;
         }		
      }
   }
   else
   {
      // Notify the user that he cannot page right now.
      if(user->in_game)
         user->self->SendSystemMessage( _STR( 15324, user->self->GetLang() ) );   
   }
   RQ_FOOTER( "RQ_IndirectedTalk" );
}



void TFCMessagesHandler::RQFUNC_Shout(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   // If user can shout and page.
   //Correction contre la faille du log cc sans être en jeu...	
   if( user->boCanShout && user->boCanPage && user->in_game)
   {
      // This function handles global (shout) broadcasted talking,		
      LPBYTE lpSender = NULL;
      LPBYTE lpText = NULL;
      DWORD  dwColor = 0;

      GET_STRING( lpSender );
      GET_LONG  ( dwColor );
      GET_STRING( lpText );

      ChatterChannels &cChatter = CPlayerManager::GetChatter();
      cChatter.Talk( user, cChatter.GetMainChannel(), string( reinterpret_cast< char * >( lpText) ));

      if (lpText != NULL)
         delete lpText;
      lpText = NULL;

      if (lpSender != NULL)
         delete lpSender;
      lpSender = NULL;
   }
   else
   {
      // Send a shouts revoked message.
      user->self->SendSystemMessage( _STR( 7171, user->self->GetLang() ) );
   }

   RQ_FOOTER( "RQ_Shout" );
}



void TFCMessagesHandler::RQFUNC_Page(PACKET_FUNC_PROTOTYPE )
{
   RQ_HEADER;

   // This function handles personnalized talking (to an ID, whisper/page)
   if( user->in_game )
   {
      unsigned char *pMessage = NULL;
      unsigned char *pName = NULL;
      GET_STRING( pName );
      GET_STRING( pMessage );

      user->self->StartAsyncPageTalk(pName,pMessage);

      if (pName)
         delete pName;
      pName = NULL;

      if (pMessage)
         delete pMessage;
      pMessage = NULL;
   }
   RQ_FOOTER( "RQ_Page" );
}

void TFCMessagesHandler::RQFUNC_DirectedTalk(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   // This functions talks directly in game to a PC/NPC (caught by NPC)
   if(user->in_game)
   {
      if(user->self->ViewFlag( __FLAG_NMS_PLAYER_DEATH ) != 0)
         return;

      if(user->self->ViewFlag( __FLAG_NMS_EN_PRISON ) == 1)
         return;

      WorldPos where = {0,0,0};
      unsigned short nb_chars = 0;		
      unsigned int i;
      BYTE * message;
      BYTE bDirection = 0;
      DWORD color; 
      DWORD theID = 0;
      long lKey;

      msg->Get((short *)&where.X);
      msg->Get((short *)&where.Y);			
      msg->Get((long  *)&theID);
      msg->Get((char  *)&bDirection );
      msg->Get((long  *)&color);
      msg->Get((long  *)&lKey);
      msg->Get((short *)&nb_chars);

      if(lKey != user->GetKeyCode())
      {
         _LOG_CHEAT
            LOG_MISC_1,
            "Player %s Account %s (IP:%s) Invalid Validation Key (RQ_DirectedTalk)",
            (LPCTSTR)user->self->GetTrueName(),
            (LPCTSTR)user->GetAccount(),
            user->GetIP()
            LOG_

            user->dwKickoutTime = 2 SECONDS TDELAY;
      }
      else
      {
         message = new BYTE[nb_chars + 1];
         for(i = 0; i < nb_chars; i++)
            msg->Get(&message[i]);	
         message[i] = 0x00;

         user->self->StartAsyncDirectTalk(message,bDirection,color,theID,where);

         if(message)
            delete []message;
         message = NULL;
      }
   }
   RQ_FOOTER( "RQ_DirectedTalk" );
}

void TFCMessagesHandler::RQFUNC_DirectedTalkNoFeed(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   // This functions talks directly in game to a PC/NPC (caught by NPC)
   if(user->in_game)
   {
      if(user->self->ViewFlag( __FLAG_NMS_PLAYER_DEATH ) != 0)
         return;

      if(user->self->ViewFlag( __FLAG_NMS_EN_PRISON ) == 1)
         return;

      WorldPos where = {0,0,0};
      unsigned short nb_chars = 0;		
      unsigned int i;
      BYTE * message;
      BYTE bDirection = 0;
      DWORD color; 
      DWORD theID = 0;
      long lKey;

      msg->Get((short *)&where.X);
      msg->Get((short *)&where.Y);			
      msg->Get((long  *)&theID);
      msg->Get((char  *)&bDirection );
      msg->Get((long  *)&color);
      msg->Get((long  *)&lKey);
      msg->Get((short *)&nb_chars);

      if(lKey != user->GetKeyCode())
      {
         _LOG_CHEAT
            LOG_MISC_1,
            "Player %s Account %s (IP:%s) Invalid Validation Key (RQ_DirectedTalk)",
            (LPCTSTR)user->self->GetTrueName(),
            (LPCTSTR)user->GetAccount(),
            user->GetIP()
            LOG_

            user->dwKickoutTime = 2 SECONDS TDELAY;
      }
      else
      {
         message = new BYTE[nb_chars + 1];
         for(i = 0; i < nb_chars; i++)
            msg->Get(&message[i]);	
         message[i] = 0x00;

         user->self->StartAsyncDirectTalkNoFeed(message,bDirection,color,theID,where);

         if(message)
            delete []message;
         message = NULL;
      }
   }
   RQ_FOOTER( "RQ_DirectedTalkNoFeed" );
}








// Message handler of packet type RQFUNC_Reroll
void TFCMessagesHandler::RQFUNC_Reroll(PACKET_FUNC_PROTOTYPE )
{
   RQ_HEADER;

   // Rerolls a PC (function reroll should hold a "level" checker
   if(!user->in_game && user->registred)
   {

      if(user->self->roll_stats())
      {
         user->boRerolling = TRUE;
         TFCPacket sending;
         sending << (RQ_SIZE)RQ_Reroll;
         user->self->packet_stats( sending );
         user->self->SendPlayerMessage( sending );
      }
   }
   RQ_FOOTER( "RQ_Reroll" );
}

void TFCMessagesHandler::RQFUNC_GetUnitName(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   // This request is sent by the client to get the name of the near unit of ID x
   DWORD dwID = 0;
   WorldPos wlUnitPos = {0,0,0};
   

   msg->Get((long *)&dwID);
   msg->Get((short *)&wlUnitPos.X);
   msg->Get((short *)&wlUnitPos.Y);

   Unit *lpuUnit;
   CString csName         = "";
   CString csGuildName    = "";
   CString csPlayerPseudo = "";

   WorldPos ppos = user->self->GetWL();
   WorldMap *world = TFCMAIN::GetWorld(ppos.world);

   if(world)
   {
      lpuUnit = world->FindNearUnit(wlUnitPos, dwID);

      if(lpuUnit)
      {
         csName = lpuUnit->GetName( user->self->GetLang() );
         TFCPacket sending;
         sending << (RQ_SIZE)RQ_GetUnitName;
         sending << (long)lpuUnit->GetID();

         DWORD color = CL_RED;
         if( lpuUnit->GetType() == U_OBJECT )
         {
            Objects *obj = static_cast< Objects * >( lpuUnit );
            if( obj->GetQty() > 1 )
            {
               char buf[ 256 ];
               csName += " x";
               _itoa_s( obj->GetQty(), buf, 256, 10 );
               csName += buf; 
            }
            if(lpuUnit->GetLockedID() == user->self->GetID())
               color = U_OBJECT_COLORYOUR
            else
               color = U_OBJECT_COLOR
         } 
         else if (lpuUnit->GetType() == U_NPC || lpuUnit->GetType() == U_MINIONS) 
         {
            color = U_NPC_COLOR;
         }
         else if (lpuUnit->GetType() == U_PC ) 
         {
            Character *lpChar = static_cast< Character * >( lpuUnit );
            Players   *lpPlayer = lpChar->GetPlayer();

            csGuildName    = lpChar->GetGuildName();
            csPlayerPseudo = lpPlayer->m_strPseudo;
            color = lpChar->GetPlayer()->self->ViewFlag(__FLAG_UNIT_COLOR);
            if(!color)
            {
               if(theApp.m_dwPVPSyetem2Actif == 1) //PVP SYSTEM
               {
                  if ( lpChar->GetPlayer()->IsGod() ) 
                  {
                     color = U_GOD_COLOR
                  }
                  else
                  {
                     if(lpChar->GetPlayer()->self->GetCrime() == 0 && lpChar->GetPlayer()->self->GetHonor() == 0)
                        color = U_OBJECT_COLOR
                     else if(lpChar->GetPlayer()->self->GetCrime() >= lpChar->GetPlayer()->self->GetHonor())
                        color = U_PC_COLOR
                     else
                        color = U_PCRP_COLOR
                  }
               }
               else
               {
                  if (lpChar->GetPlayer()->IsGod()) 
                     color = U_GOD_COLOR
                  else if(lpChar->GetPlayer()->self->ViewFlag(__FLAG_RPHRP_STATUS) == 1)
                     color = U_PCRP_COLOR
                  else
                     color = U_PC_COLOR
               }
            }
         }
         CString strNameOK;
         if(lpuUnit->ViewFlag(__FLAG_NMS_PLAYER_DEATH) == 0)
            strNameOK = csName;
         else
         {
            color = CL_GRAY;
            strNameOK.Format("%s %s )",_STR( 15333 , user->self->GetLang() ),csName);
         }

         CString strTmp = strNameOK;
         if(user->m_dwShowID)
         {
            strNameOK.Format("[%d] %s",dwID,strTmp);
            user->m_dwLastShowID = dwID;
         }

         strTmp = strNameOK;
         if(csPlayerPseudo != "")
         {
            strNameOK.Format("%s : %s",strTmp,csPlayerPseudo);
         }

         DWORD GuildColor = CL_GREEN_DARK;


         sending << strNameOK;
         sending << csGuildName;
         sending << (long)color;
         sending << (long)GuildColor;

         user->self->SendPlayerMessage( sending );
      }
   }

   RQ_FOOTER( "RQ_GetUnitName" );
}

void TFCMessagesHandler::RQFUNC_GetUnitName2(PACKET_FUNC_PROTOTYPE )
{
   RQ_HEADER;

   // This request is sent by the client to get the name of the near unit of ID x
   DWORD dwID = 0;
   WorldPos wlUnitPos = {0,0,0};
   short shShow = 1;

   msg->Get((long *)&dwID);
   msg->Get((short *)&wlUnitPos.X);
   msg->Get((short *)&wlUnitPos.Y);
   msg->Get((short *)&shShow);

   Unit *lpuUnit;
   CString csName         = "";
   CString csGuildName    = "";
   CString csPlayerPseudo = "";
   WorldPos ppos = user->self->GetWL();
   WorldMap *world = TFCMAIN::GetWorld(ppos.world);


   if(world)
   {
      lpuUnit = world->FindNearUnit(wlUnitPos, dwID);

      if(lpuUnit)
      {   
         TFCPacket sending;
         csName = lpuUnit->GetName( user->self->GetLang() );
         sending.Destroy();
         sending << (RQ_SIZE)RQ_GetUnitName2;
         sending << (long)lpuUnit->GetID();

         DWORD color = CL_RED;
         if( lpuUnit->GetType() == U_OBJECT )
         {
            Objects *obj = static_cast< Objects * >( lpuUnit );
            if( obj->GetQty() > 1 )
            {
               char buf[ 256 ];
               csName += " x";
               _itoa_s( obj->GetQty(), buf, 256, 10 );
               csName += buf; 
            }
            if(lpuUnit->GetLockedID() == user->self->GetID())
               color = U_OBJECT_COLORYOUR
            else
               color = U_OBJECT_COLOR
         } 
         else if (lpuUnit->GetType() == U_NPC || lpuUnit->GetType() == U_MINIONS) 
         {
            color = U_NPC_COLOR;
         }
         else if (lpuUnit->GetType() == U_PC ) 
         {
            Character *lpChar = static_cast< Character * >( lpuUnit );
            Players   *lpPlayer = lpChar->GetPlayer();

            csGuildName    = lpChar->GetGuildName();
            csPlayerPseudo = lpPlayer->m_strPseudo;
            color = lpChar->GetPlayer()->self->ViewFlag(__FLAG_UNIT_COLOR);
            if(!color)
            {
               if(theApp.m_dwPVPSyetem2Actif == 1) //PVP SYSTEM
               {
                  if ( lpChar->GetPlayer()->IsGod() ) 
                  {
                     color = U_GOD_COLOR
                  }
                  else
                  {
                     if(lpChar->GetPlayer()->self->GetCrime() == 0 && lpChar->GetPlayer()->self->GetHonor() == 0)
                        color = U_OBJECT_COLOR
                     else if(lpChar->GetPlayer()->self->GetCrime() >= lpChar->GetPlayer()->self->GetHonor())
                     color = U_PC_COLOR
                     else
                     color = U_PCRP_COLOR
                  }
               }
               else
               {
                  if (lpChar->GetPlayer()->IsGod()) 
                     color = U_GOD_COLOR
                  else if(lpChar->GetPlayer()->self->ViewFlag(__FLAG_RPHRP_STATUS) == 1)
                  color = U_PCRP_COLOR
                  else
                  color = U_PC_COLOR
               }
            }


         }
         CString strNameOK;
         if(lpuUnit->ViewFlag(__FLAG_NMS_PLAYER_DEATH) == 0)
            strNameOK = csName;
         else
         {
            color = CL_GRAY;
            strNameOK.Format("%s %s )",_STR( 15333 ,user->self->GetLang() ),csName);
         }

         CString strTmp = strNameOK;
         if(user->m_dwShowID)
         {
            strNameOK.Format("[%d] %s",dwID,strTmp);
            user->m_dwLastShowID = dwID;
         }

         strTmp = strNameOK;
         if(csPlayerPseudo != "")
            strNameOK.Format("%s : %s",strTmp,csPlayerPseudo);

         DWORD GuildColor = CL_GREEN_DARK;
         sending << strNameOK;
         sending << csGuildName;
         sending << (long)color;
         sending << (long)GuildColor;
         sending << (short)shShow;

         user->self->SendPlayerMessage( sending );
      }
   }
   RQ_FOOTER( "RQ_GetUnitName2" );
}

void TFCMessagesHandler::RQFUNC_GetSkillList(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   TFCPacket sendingSkill;
   user->self->PacketSkills(sendingSkill);
   user->self->SendPlayerMessage(sendingSkill);

   RQ_FOOTER( "RQ_GetSkillList" );
}

void TFCMessagesHandler::RQFUNC_GetStatus(PACKET_FUNC_PROTOTYPE )
{
   RQ_HEADER;

   TFCPacket sendingS1;
   user->self->PacketStatus(sendingS1);
   user->self->SendPlayerMessage(sendingS1);

   RQ_FOOTER( "RQ_GetStatus" );
}

void TFCMessagesHandler::RQFUNC_GetStatus2(PACKET_FUNC_PROTOTYPE )
{
   RQ_HEADER;

   TFCPacket sendingS2;
   user->self->PacketStatus2(sendingS2);
   user->self->SendPlayerMessage(sendingS2);

   RQ_FOOTER( "RQ_GetStatus2" );
}

void TFCMessagesHandler::RQFUNC_UpdateFactionID(PACKET_FUNC_PROTOTYPE )
{
   RQ_HEADER;

   user->self->Teleport( user->self->GetWL(), 0 );

   RQ_FOOTER( "RQ_UpdateFactionID" );
}



void TFCMessagesHandler::RQFUNC_ChestNormal(PACKET_FUNC_PROTOTYPE )
{
   RQ_HEADER;

   user->self->SetFlag(__FLAG_PLAYER_USE_NEW_CHEST,0);

   RQ_FOOTER( "RQFUNC_ChestNormal" );
}

void TFCMessagesHandler::RQFUNC_ChestListe(PACKET_FUNC_PROTOTYPE )
{
   RQ_HEADER;

   user->self->SetFlag(__FLAG_PLAYER_USE_NEW_CHEST,1);
   if(theApp.dwChestListEnable == 0)
   {
      user->self->SendInfoMessage( _STR( 15469 , user->self->GetLang() ),CL_RED);
   }

   RQ_FOOTER( "RQFUNC_ChestListe" );
}


void TFCMessagesHandler::RQFUNC_FromPreInGameToInGame(PACKET_FUNC_PROTOTYPE)
{
   if( user->boLockedOut || !user->CanPaidPlay() || !user->CanConnectGMOnly())
   {
      return;
   }

   RQ_HEADER;

   // Create new player message structure
   LPASYNC_PACKET_FUNC_PARAMS lpParams = new ASYNC_PACKET_FUNC_PARAMS;
   lpParams->user        = user;
   lpParams->rqRequestID = rqRequestID;
   lpParams->msg         = NULL;
   lpParams->strParam    = "";

   user->self->StartAsyncFromPregameToGame();

   RQ_FOOTER( "RQ_FromPreInGameToInGame" );
}

void TFCMessagesHandler::RQFUNC_SendChatterMessage(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   LPBYTE szChannel;
   LPBYTE szMessage;

   // Get the channel and message from the packet.
   GET_STRING( szChannel );
   GET_STRING( szMessage );    


   // Correction de la faille des paroles sur cc sans être en jeu
   if ( user->in_game ) 
   {
      ChatterChannels &cChatter = CPlayerManager::GetChatter();
      cChatter.Talk( user, (char*)szChannel, (char*)szMessage );
   }
   else
   {
      _LOG_DEBUG 
         LOG_DEBUG_LVL1, 
         "Received an unregistered channel message.\r\n\tAccountName : %s\r\n\tPlayer IP : %s",
         (LPCTSTR)user->GetFullAccountName(),
         (LPCTSTR)user->GetIP()
         LOG_	
         user->SetDeletePlayerFlags();
   }

   if(szChannel)
      delete []szChannel;
   szChannel = NULL;

   if(szMessage)
      delete []szMessage;
   szMessage = NULL;


   RQ_FOOTER( "RQ_SendChatterMessage" );
}

void TFCMessagesHandler::RQFUNC_GetChatterUserList2(PACKET_FUNC_PROTOTYPE )
{
   RQ_HEADER;
   if(user && user->self && user->in_game)
   {
      LPBYTE szChannel;
      GET_STRING( szChannel );

      ChatterChannels &cChatter = CPlayerManager::GetChatter();
      cChatter.SendChannelUsers2( user, (char*)szChannel);

      if(szChannel)
         delete []szChannel;
      szChannel = NULL;
   }
   RQ_FOOTER( "RQ_GetChatterUserList2" );
}

void TFCMessagesHandler::RQFUNC_QueryItemName( PACKET_FUNC_PROTOTYPE )
{
   RQ_HEADER;


   DWORD dwID;
   BYTE  whereToSearchFrom;
   // a changer dans guild master si jamais
   // PL_SEARCHGUILDCHEST change de la valeur 4...

   GET_BYTE( whereToSearchFrom );
   GET_LONG( dwID );

   Players		*pUser = user;

   enum ePlacesToSearch { PL_SEARCHBACKPACK=0, PL_SEARCHCHEST=1, PL_SEARCHMYTRADE=2, PL_SEARCHOTHERTRADE=3,PL_SEARCHGUILDCHEST=4};
   BOOL boFound = FALSE;
   if (whereToSearchFrom == PL_SEARCHBACKPACK) 
   {
      TemplateList<Unit> *lptluBackpack;

      if( pUser->self->GetGameOpContext() != NULL )
         lptluBackpack = pUser->self->GetGameOpContext()->GetBackpack();
      else
         lptluBackpack = pUser->self->GetBackpack();

      lptluBackpack->Lock();
      lptluBackpack->ToHead();
      while( lptluBackpack->QueryNext() && !boFound )
      {
         Objects *obj = static_cast< Objects * >( lptluBackpack->Object() );

         if( obj->GetID() == dwID )
         {
            TFCPacket sending;
            sending << (RQ_SIZE)RQ_QueryItemName;
            sending << (char)PL_SEARCHBACKPACK;
            sending << (long)obj->GetID();
            CString csName = obj->GetName( pUser->self->GetLang() );
            sending << (CString)( csName );
            pUser->self->SendPlayerMessage( sending );
            boFound = TRUE;                
         }
      }
      lptluBackpack->Unlock();
   }
   else if (whereToSearchFrom == PL_SEARCHCHEST) 
   {
      if (boFound == FALSE) 
      {
         ItemContainer *lpicChest;
         if (pUser->self->GetGameOpContext() != NULL) 
            lpicChest = pUser->self->GetGameOpContext()->GetChest();
         else 
            lpicChest = pUser->self->GetChest();
         TemplateList<Objects> *tlChestList = lpicChest->LockAndGetList();
         tlChestList->ToHead();
         while( tlChestList->QueryNext() && !boFound )
         {
            Objects *obj = tlChestList->Object();
            if( obj->GetID() == dwID )
            {
               TFCPacket sending;
               sending << (RQ_SIZE)RQ_QueryItemName;
               sending << (char)PL_SEARCHCHEST;
               sending << (long)obj->GetID();
               CString csName = obj->GetName( pUser->self->GetLang() );
               sending << (CString)( csName );
               pUser->self->SendPlayerMessage( sending );
               boFound = TRUE;                
            }
         }
         lpicChest->UnlockAndReleaseList();
      }
   }
   else if (whereToSearchFrom == PL_SEARCHGUILDCHEST) 
   {
      if (boFound == FALSE) 
      {
         Character *lpCharacter = static_cast< Character * >( pUser->self );
         boFound = ::GuildMaster::GetChestUnitName(lpCharacter,dwID);
      }
   }

   else if (whereToSearchFrom == PL_SEARCHMYTRADE) 
   {
      CString itemName("");
      BOOL itemFound = pUser->self->GetTradeMgr2()->GetItemName(dwID, itemName, pUser->self->GetLang());
      if (itemFound == TRUE) 
      {
         TFCPacket sending;
         sending << (RQ_SIZE)RQ_QueryItemName;
         sending << (char)PL_SEARCHMYTRADE;
         sending << (long)dwID;
         sending << (CString)( itemName );
         pUser->self->SendPlayerMessage( sending );
      }
   }
   else if (whereToSearchFrom == PL_SEARCHOTHERTRADE) 
   {
      CString itemName("");
      BOOL itemFound = pUser->self->GetTradeMgr2()->GetItemNameFromOther(dwID, itemName, pUser->self->GetLang());
      if (itemFound == TRUE) 
      {
         TFCPacket sending;
         sending << (RQ_SIZE)RQ_QueryItemName;
         sending << (char)PL_SEARCHOTHERTRADE;
         sending << (long)dwID;
         sending << (CString)( itemName );
         pUser->self->SendPlayerMessage( sending );
      }
   }

   RQ_FOOTER( "RQ_QueryItemName" );
}

void TFCMessagesHandler::RQFUNC_GetNearItems( PACKET_FUNC_PROTOTYPE )
{
   RQ_HEADER;

   if( user->boPreInGame || user->in_game )
   {
      WorldPos wlPos = user->self->GetWL();
      WorldMap *lpWorld = TFCMAIN::GetWorld( wlPos.world );				
      if( lpWorld )
      {
         // Then send the list of all objects near the player.
         TFCPacket sending;
         int read;
         read = lpWorld->packet_inview_units( wlPos, sending, _DEFAULT_RANGE, user->self );
         if( read > 0 )
         {
            user->self->SendPlayerMessage( sending );
         }
         else
         {
            sending << (RQ_SIZE)RQ_GetNearItems;
            user->self->SendPlayerMessage( sending );
         }
      }
   }
   RQ_FOOTER( "RQ_GetNearItems" );
}

void TFCMessagesHandler::RQFUNC_SendSpellList( PACKET_FUNC_PROTOTYPE )
{
   RQ_HEADER;

   BYTE bUpdate = 0;
   GET_BYTE( bUpdate );

   TFCPacket sendingSpell;
   user->self->PacketSpells(sendingSpell,(BYTE)bUpdate);
   user->self->SendPlayerMessage(sendingSpell);

 
   RQ_FOOTER( "RQ_SendSpellList" );
}

void TFCMessagesHandler::RQFUNC_PuppetInformation(  PACKET_FUNC_PROTOTYPE )
{
   RQ_HEADER;

   DWORD dwID;
   WorldPos wlPos = { 0, 0, user->self->GetWL().world };

   GET_LONG( dwID );
   GET_WORD( wlPos.X );
   GET_WORD( wlPos.Y );

   WorldMap *wlWorld = TFCMAIN::GetWorld( user->self->GetWL().world );
   if( wlWorld != NULL )
   {
      Unit *lpUnit;
      lpUnit = wlWorld->FindNearUnit( wlPos, dwID );
      if( lpUnit != NULL )
      {
         TFCPacket sending;
         lpUnit->PacketPuppetInfo( sending );
         user->self->SendPlayerMessage( sending );
      }
   }

   RQ_FOOTER( "RQ_PuppetInformation" );
}

void TFCMessagesHandler::RQFUNC_RemoveFromChatterChannel( PACKET_FUNC_PROTOTYPE )
{
   RQ_HEADER;

   LPBYTE szChannel;
   GET_STRING( szChannel );

   ChatterChannels &cChatter = CPlayerManager::GetChatter();
   cChatter.Remove( user, (char*)szChannel);
   cChatter.SendRegisteredChannelList( user );

  if(szChannel)
      delete []szChannel;
   szChannel = NULL;
   RQ_FOOTER( "RQ_RemoveFromChatterChannel" );
}

void TFCMessagesHandler::RQFUNC_GetChatterChannelList( PACKET_FUNC_PROTOTYPE )
{
   RQ_HEADER;

   ChatterChannels &cChatter = CPlayerManager::GetChatter();
   cChatter.SendRegisteredChannelList( user );

   RQ_FOOTER( "RQ_GetPublicChatterChannelList" );
}

void TFCMessagesHandler::RQFUNC_GroupInvite( PACKET_FUNC_PROTOTYPE )
{
   RQ_HEADER;

   if(user->self->GetArenaID() >0)
   {
      user->self->SendInfoMessage( _STR( 15474 , user->self->GetLang() ),CL_RED);
      return;
   }

   DWORD dwID;
   GET_LONG ( dwID );
   WorldPos wlPos = { 0, 0, user->self->GetWL().world };
   GET_WORD ( wlPos.X );
   GET_WORD ( wlPos.Y );

   if( dwID == user->self->GetID() )
   {
      user->self->SendSystemMessage(_STR( 7502, user->self->GetLang() ));
   }
   else
   {
      bool newlyCreated = false;
      if( user->self->GetGroup() == NULL ) // If the player isn't already in a group.
      {
         if( Group::CreateGroup( user->self ) == NULL )// If a new group could not be created.
            user->self->SendSystemMessage( _STR( 2841, user->self->GetLang() ) );
         else
            newlyCreated = true;
      }

      // If the group was created.
      if( user->self->GetGroup() != NULL )
      {
         bool dismissGroup = true;
         WorldMap *wlWorld = TFCMAIN::GetWorld( user->self->GetWL().world );

         if( wlWorld != NULL )// World should never be NULL! Means that the GetWL().world is screwed up or wrong.
         {
            // Try to find the unit.
            Unit *lpUnit = wlWorld->FindNearUnit( wlPos, dwID );
            if( lpUnit != NULL )
            {
               // If the unit is a PC.
               if( lpUnit->GetType() == U_PC )
               {
                  // Cast its character.
                  Character *lpCharacter = static_cast< Character * >( lpUnit );

                  // If target player isn't already in a group, or is
                  // in the asker's group.
                  if( lpCharacter->GetGroup() == NULL || lpCharacter->GetGroup() == user->self->GetGroup() )
                  {
                     // If the player could be invited.
                     if( user->self->GetGroup()->Invite( user->self, lpCharacter ) )
                     {
                        // Don't dismiss the group.
                        dismissGroup = false;
                     }
                  }
                  else
                  {
                     // Notify player.
                     user->self->SendSystemMessage( _STR( 2843, user->self->GetLang() ) );
                  }
               }
               else
               {
                  // Otherwise notify player.
                  user->self->SendSystemMessage( _STR( 2842, user->self->GetLang() ) );
               }

               // If the group was created with this request,
               // and the request failed to invite anyone.
               if( newlyCreated && dismissGroup )
               {
                  // Dismiss the group.
                  user->self->GetGroup()->Dismiss( user->self );
               }
            }
         }
      }
   }


   RQ_FOOTER( "RQ_GroupInvite" );
}

void TFCMessagesHandler::RQFUNC_GroupJoin( PACKET_FUNC_PROTOTYPE )
{
   RQ_HEADER;

   if(user->self->GetArenaID() >0)
   {
      user->self->SendInfoMessage( _STR( 15474 , user->self->GetLang() ),CL_RED);
      return;
   }
   // If user has binded group.
   if( user->self && user->self->GetGroup() != NULL )
   {
      // Try to join it.
      if( user->self->GetGroup()->Join( user->self ) )
      {
         // Notify player of succesfull addition to group.
         TFormat format;
         user->self->SendSystemMessage(format( _STR( 2844, user->self->GetLang() ), user->self->GetGroup()->GetLeader()->GetTrueName()));
      }
   }


   RQ_FOOTER( "RQ_GroupJoin" );
}

void TFCMessagesHandler::RQFUNC_GroupLeave( PACKET_FUNC_PROTOTYPE )
{
   RQ_HEADER;

   if(user->self->GetArenaID() >0)
   {
      user->self->SendInfoMessage( _STR( 15474 , user->self->GetLang() ),CL_RED);
      return;
   }

   if( user->self && user->self->GetGroup() != NULL )
   {
      user->self->GetGroup()->Dismiss( user->self );// Leave it
   }
   RQ_FOOTER( "RQ_GroupLeave" );
}

void TFCMessagesHandler::RQFUNC_GroupKick( PACKET_FUNC_PROTOTYPE )
{
   RQ_HEADER;

   if(user->self->GetArenaID() >0)
   {
      user->self->SendInfoMessage( _STR( 15474 , user->self->GetLang() ),CL_RED);
      return;
   }

   DWORD dwID;
   GET_LONG( dwID );

   // If player has a group.
   if(user->self && user->self->GetGroup() != NULL )
   {
      // If the leader of this group is this player.
      if( user->self->GetGroup()->GetLeader() == user->self )
      {
         // Dismiss this character.
         user->self->GetGroup()->Dismiss( dwID );
      }
   }

   RQ_FOOTER( "RQ_GroupKick" );
}

void TFCMessagesHandler::RQFUNC_JunkItems( PACKET_FUNC_PROTOTYPE )
{
   RQ_HEADER;

   DWORD id;
   DWORD qty;
   GET_LONG( id );
   GET_LONG( qty );

   if(user->self)
      user->self->JunkItems( id, qty, false );

   RQ_FOOTER( "RQ_JunkItems" );
}

void TFCMessagesHandler::RQFUNC_ToggleChatterListening( PACKET_FUNC_PROTOTYPE )
{
   RQ_HEADER;

   BYTE *channelID;
   BYTE listenState;
   GET_STRING( channelID );
   GET_BYTE  ( listenState );

   ChatterChannels &cChatter = CPlayerManager::GetChatter();
   cChatter.ToggleListening( user, (const char *)channelID, ( listenState == 0 ? false : true ) );
   cChatter.SendRegisteredChannelList( user );// Send only the registered list of channels.

   if (channelID != NULL)
      delete channelID;
   channelID = NULL;
   RQ_FOOTER( "RQ_ToggleChatterListening" );
}

void TFCMessagesHandler::RQFUNC_GroupToggleAutoSplit( PACKET_FUNC_PROTOTYPE )
{
   RQ_HEADER;
   BYTE bNewState = 0;
   GET_BYTE( bNewState );

   if( user->self && user->self->GetGroup() != NULL )
   {
      // If the leader of this group is this player.
      if( user->self->GetGroup()->GetLeader() == user->self )
      {
         user->self->GetGroup()->ToggleAutoSplit( bNewState == 0 ? false : true );
      }
   }

   RQ_FOOTER( "RQ_GroupToggleAutoSplit" );
}


void TFCMessagesHandler::RQFUNC_ChestAddItemFromBackpack(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   // Moves an item from the backpack to the chest
   DWORD dwItemID, dwQty;
   msg->Get(&dwItemID);
   msg->Get(&dwQty);

   user->self->MoveObjectFromBackpackToChest2(dwItemID, dwQty);

   RQ_FOOTER( "RQ_ChestAddItemFromBackpack" );
}

void TFCMessagesHandler::RQFUNC_ChestRemoveItemToBackpack(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   // Moves an item from the chest to the backpack
   DWORD dwItemID, dwQty;
   msg->Get(&dwItemID);
   msg->Get(&dwQty);

   user->self->MoveObjectFromChestToBackpack2(dwItemID, dwQty);

   RQ_FOOTER( "RQ_ChestRemoveItemToBackpack" );
}

void TFCMessagesHandler::RQFUNC_TradeInvite(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   // Grab ID and pos, try to find the unit near this coords, if found, call TradeInvite function!
   DWORD dwID;
   WorldPos wlPos = { 0, 0, user->self->GetWL().world };
   GET_LONG ( dwID );
   GET_WORD ( wlPos.X );
   GET_WORD ( wlPos.Y );

   WorldMap *wlWorld = TFCMAIN::GetWorld( user->self->GetWL().world );

   // World should never be NULL! Means that the GetWL().world is screwed up or wrong.
   if( wlWorld != NULL )
   {
      // Try to find the unit.
      Unit *lpUnit = wlWorld->FindNearUnit( wlPos, dwID );
      if( lpUnit != NULL )
      {
         // If the unit is a PC.
         if( lpUnit->GetType() == U_PC )
         {
            // Cast its character.
            Character *lpCharacter = static_cast< Character * >( lpUnit );
            user->self->TradeRequest(lpCharacter);
         }
         else
         {
            // Otherwise notify player.
            user->self->SendSystemMessage( _STR( 12951, user->self->GetLang() ) );
         }
      }
   }

   RQ_FOOTER( "RQ_TradeInvite" );
}

void TFCMessagesHandler::RQFUNC_TradeCancel(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   if(user->self)
      user->self->TradeCancel();

   RQ_FOOTER( "RQ_TradeCancel" );
}

void TFCMessagesHandler::RQFUNC_TradeSetStatus(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;
   WORD newStatus;
   GET_WORD(newStatus);

   if(user->self)
      user->self->TradeSetStatus((TradeMgr2::Status::CharacterStatus)newStatus);

   RQ_FOOTER( "RQ_TradeSetStatus" );
}

void TFCMessagesHandler::RQFUNC_TradeAddItemFromBackpack(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   DWORD dwObjID, dwObjQty;
   GET_LONG(dwObjID);
   GET_LONG(dwObjQty);

   if(user->self)
      user->self->TradeAddItemFromBackpack(dwObjID,dwObjQty);

   RQ_FOOTER( "RQ_TradeAddItemFromBackpack" );
}

void TFCMessagesHandler::RQFUNC_TradeRemoveItemToBackpack(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;
   DWORD dwObjID, dwObjQty;
   GET_LONG(dwObjID);
   GET_LONG(dwObjQty);

   if(user->self)
      user->self->TradeRemoveItemToBackpack(dwObjID, dwObjQty);

   RQ_FOOTER( "RQ_TradeRemoveItemToBackpack" );
}

void TFCMessagesHandler::RQFUNC_TradeClear(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   if(user->self)
      user->self->TradeClearItemsFromTrade();
  
   RQ_FOOTER( "RQ_TradeClear" );
}

void TFCMessagesHandler::RQFUNC_QueryItemInfo( PACKET_FUNC_PROTOTYPE )
{
   RQ_HEADER;

   DWORD dwID;
   GET_LONG(dwID);

   if(user->self)
   {
      TFCPacket sending;
      sending << (RQ_SIZE)RQ_QueryItemInfo;

      Unit *lpUnit = Unit::GetByID(dwID);
      if (lpUnit)
      {
         if (lpUnit->GetType() == U_OBJECT)
         {
            sending << (char)0;
            sending << lpUnit->GetName(IntlText::GetDefaultLng());
            if(theApp.dwDisableItemInfo == 0) //ItemInfo is enabled...
            {
               _item *item;
               lpUnit->SendUnitMessage(MSG_OnGetUnitStructure,user->self,0,0,0,&item);
               sending << (short)item->appearance;
               sending << (char)item->cRadiance;
               sending << (short)item->armor.AC;
               sending << (short)item->armor.Dod;
               sending << (short)item->armor.End;
               sending << (long)(item->weapon.cDamage.GetMinBoost(user->self));
               sending << (long)(item->weapon.cDamage.GetMaxBoost(user->self));
               sending << (short)item->weapon.Att;
               sending << (short)item->weapon.Str;
               sending << (short)item->weapon.Agi;
               sending << (short)item->magic.nMinWis;
               sending << (short)item->magic.nMinInt;
               WORD plInt = user->self->GetTrueINT();
               WORD plWis = user->self->GetTrueWIS();
               sending << (short)item->tlBoosts.NbObjects();
               item->tlBoosts.ToHead();
               while (item->tlBoosts.QueryNext())
               {
                  LPOBJECT_BOOST lpBoost = item->tlBoosts.Object();
                  if (lpBoost->wStat > 10000 /* SkillBoostOffset */)
                  {
                     switch(lpBoost->wStat)
                     {
                     case 10001: // Stun Blow
                        sending << (char)27;
                        break;
                     case 10002: // Powerful Blow
                        sending << (char)28;
                        break;
                     case 10004: // First Aid
                        sending << (char)29;
                        break;
                     case 10008: // Parry
                        sending << (char)30;
                        break;
                     case 10009: // Meditate
                        sending << (char)31;
                        break;
                     case 10011: // Dodge
                        sending << (char)32;
                        break;
                     case 10012: // Attack
                        sending << (char)33;
                        break;
                     case 10014: // Hide
                        sending << (char)34;
                        break;
                     case 10015: // Rob
                        sending << (char)35;
                        break;
                     case 10016: // Sneak
                        sending << (char)36;
                        break;
                     case 10017: // Search
                        sending << (char)37;
                        break;
                     case 10026: // Picklock
                        sending << (char)38;
                        break;
                     case 10027: // Armor Penetration
                        sending << (char)39;
                        break;
                     case 10028: // Peek
                        sending << (char)40;
                        break;
                     case 10029: // Rapid Healing
                        sending << (char)41;
                        break;
                     case 10035: // Archery
                        sending << (char)42;
                        break;
                     case 10036: // Dual Weapons
                        sending << (char)43;
                        break;
                     case 10037: // plunder
                        sending << (char)44;
                        break;
                     case 10038: // resurect
                        sending << (char)45;
                        break;
                     case 10020: // Power Conjuring
                        sending << (char)46;
                        break;
                     case 10021: // Primal Scream
                        sending << (char)47;
                        break;
                     case 10022: // Immobilization
                        sending << (char)48;
                        break;
                     case 10010: // Critical Strike
                        sending << (char)49;
                        break;
                     default: // Invalid or unused skill
                        sending << (char)0;
                     }
                  }
                  else
                  {
                     sending << (char)lpBoost->wStat;
                  }
                  if ((lpBoost->nMinWIS <= plWis) && (lpBoost->nMinINT <= plInt))
                  {
                     sending << (long)(lpBoost->bfBoost.GetMinBoost(user->self));
                     sending << (long)(lpBoost->bfBoost.GetMaxBoost(user->self));
                  }
                  else
                  {
                     sending << (long)0;
                     sending << (long)0;
                  }
               }
            }
            else //Blank Window
            {
               sending << (short)0;
               sending << (char)0;
               sending << (short)0;
               sending << (short)0;
               sending << (short)0;
               sending << (long)0;
               sending << (long)0;
               sending << (short)0;
               sending << (short)0;
               sending << (short)0;
               sending << (short)0;
               sending << (short)0;
               sending << (short)0;
            }
         }
         else
         {
            sending << (char)2;
         }
      }
      else
      {
         sending << (char)1;
      }
      user->self->SendPlayerMessage(sending);
   }

   RQ_FOOTER( "RQ_QueryItemInfo" );
}

void TFCMessagesHandler::RQFUNC_NM_GetGuildList(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;
   user->self->NMGetGuildList(1);
   RQ_FOOTER( "RQFUNC_NM_GetGuildList" );
}

void TFCMessagesHandler::RQFUNC_NM_GuildInvite(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   if(theApp.dwGuildSystemEnable == 0 || (theApp.dwGuildSystemEnable == 2 && !user->IsGod()))
   {
      user->self->SendInfoMessage( _STR( 15113 , user->self->GetLang() ),0x0020FF);
      return;
   }

   DWORD dwID;
   WorldPos wlPos = { 0, 0, user->self->GetWL().world };
   GET_LONG ( dwID );
   GET_WORD ( wlPos.X );
   GET_WORD ( wlPos.Y );

   if(user->self)
   {
      if( dwID == user->self->GetID() )
      {
         //invite lui meme....
         user->self->SendInfoMessage(_STR( 7502, user->self->GetLang() ),0x0080FF);
         return;
      }

      if(user->self->GetGuildName() == "")
      {
         //ye meme pas dans une gild comment peu til en inviter unn autres...
         user->self->SendInfoMessage(_STR( 15049, user->self->GetLang() ),0x0080FF);
         return;
      }

      uGuildPermission uPermission;
      uPermission.dwVal = user->self->GetGuildPermission();


      if(uPermission.Permission.CanInvite == 0)
      {
         //ye meme pas dans une gild comment peu til en inviter unn autres...
         user->self->SendInfoMessage(_STR( 15053, user->self->GetLang() ),0x0080FF);
         return;
      }


      WorldMap *wlWorld = TFCMAIN::GetWorld( user->self->GetWL().world );

      if( wlWorld != NULL )
      {

         // Try to find the unit.
         Unit *lpUnit = wlWorld->FindNearUnit( wlPos, dwID );

         if( lpUnit != NULL )
         {
            // If the unit is a PC.
            if( lpUnit->GetType() == U_PC )
            {
               // Cast its character.
               Character *lpCharacter = static_cast< Character * >( lpUnit );

               // If target player isn't already in a group, or is
               // in the asker's group.
               if( lpCharacter->GetGuildName() == "")
               {
                  //invite a etre membre de la guild...
                  //if( user->self->GetGroup()->Invite( user->self, lpCharacter ) )
                  // Send invitation notice!
                  TFCPacket sending;

                  sending << (RQ_SIZE)RQ_NM_GuildInvite;
                  sending << (long)user->self->GetID();
                  sending << user->self->GetTrueName();
                  sending << user->self->GetGuildName();
                  lpCharacter->SetGuildNameInvited(user->self->GetGuildName().GetBuffer(0),user->self);
                  lpCharacter->SendPlayerMessage( sending );
                  user->self->SendInfoMessage( _STR( 15069, user->self->GetLang() ) ,0x0080FF);
               }
               else
               {
                  // Notify player.
                  user->self->SendInfoMessage( _STR( 15051, user->self->GetLang() ) ,0x0080FF);
               }
            }
            else
            {
               // Otherwise notify player.
               user->self->SendInfoMessage( _STR( 15050, user->self->GetLang() ) ,0x0080FF);
            }
         }
      }
   }
   RQ_FOOTER( "RQFUNC_NM_GuildInvite" );
}

// Change user permission and titre...
void TFCMessagesHandler::RQFUNC_NM_GuildChangeNote(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   if(!user->self)
      return;

   if(theApp.dwGuildSystemEnable == 0 || (theApp.dwGuildSystemEnable == 2 && !user->IsGod()))
      user->self->SendInfoMessage( _STR( 15113 , user->self->GetLang() ),0x0020FF);
   else
   {
      // If user has a binded group
      Character *lpCharacter = static_cast< Character * >( user->self );
      if(lpCharacter)
      {
         uGuildPermission uPerm;
         uPerm.dwVal = lpCharacter->GetGuildPermission();
         if( uPerm.Permission.CanWriteNote)
         {

            LPBYTE lpNotes = NULL;
            GET_STRING( lpNotes );

            if(strlen((char*)lpNotes) >254)
               lpNotes[254] = 0x00;

            CString strNoteTmp;
            strNoteTmp.Format("%s",lpNotes);
            theApp.AddGuildRequest(NULL,NULL,NULL,GUILD_CHANGE_NOTES,user->self->GetID(),0,0,0,strNoteTmp,"");

            if(lpNotes)
               delete []lpNotes;
            lpNotes = NULL;
         }
         else
         {
            lpCharacter->SendInfoMessage( _STR( 15053, user->self->GetLang() ),0x0080FF);
         }
      }
   }

   RQ_FOOTER( "RQFUNC_NM_GuildChangeNote" );
}

void TFCMessagesHandler::RQFUNC_NM_GuildJoin(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   if(!user->self)
      return;

   if(theApp.dwGuildSystemEnable == 0 || (theApp.dwGuildSystemEnable == 2 && !user->IsGod()))
   {
      user->self->SendInfoMessage( _STR( 15113 , user->self->GetLang() ),0x0020FF);
   }
   else
   {
      // s<assure qui a ete inviter et quil est pas deja dans une guilde...
      if( user->self->GetGuildNameInvited() != "" && user->self->GetGuildName() == "")
         theApp.AddGuildRequest(NULL,NULL,NULL,GUILD_REQ_ADD_USER,user->self->GetID(),0,0,0,"","");
   }
   RQ_FOOTER( "RQFUNC_NM_GuildJoin" );
}

// Allows a player to leave a group or refuse an invitation.
void TFCMessagesHandler::RQFUNC_NM_GuildLeave(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   if(!user->self)
      return;

   if(theApp.dwGuildSystemEnable == 0 || (theApp.dwGuildSystemEnable == 2 && !user->IsGod()))
      user->self->SendInfoMessage( _STR( 15113 , user->self->GetLang() ),0x0020FF);
   else
   {
      // If user has a binded group
      Character *lpCharacter = static_cast< Character * >( user->self );
      if(lpCharacter)
      {
         if( lpCharacter->GetGuildNameInvited() != "" )
         {
            // juste refuser invitation...
            if(lpCharacter->GetGuildNameInvitedChar())
               lpCharacter->SetGuildNameInvited("",NULL);
         }
         else
         {
            theApp.AddGuildRequest(NULL,NULL,NULL,GUILD_REQ_REMOVE_USER,user->self->GetID(),0,0,0,"","");
         }
      }
   }

   RQ_FOOTER( "RQFUNC_NM_GuildLeave" );
}

// Allows a player to leave a group or refuse an invitation.
void TFCMessagesHandler::RQFUNC_NM_GuildKick(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;
   DWORD dwID;
   GET_LONG( dwID );

   if(!user->self)
      return;

   if(theApp.dwGuildSystemEnable == 0 || (theApp.dwGuildSystemEnable == 2 && !user->IsGod()))
      user->self->SendInfoMessage( _STR( 15113 , user->self->GetLang() ),0x0020FF);
   else
   {
      // If user has a binded group
      Character *lpCharacter = static_cast< Character * >( user->self );
      if(lpCharacter)
      {
         uGuildPermission uPerm;
         uPerm.dwVal = lpCharacter->GetGuildPermission();

         if( uPerm.Permission.CanKick)
         {
            Players * pPlayer = CPlayerManager::GetCharacterRessourceByID(dwID); //PM Pas besoin de lock un simple PING si il existe,...
            if(pPlayer)
            {
               theApp.AddGuildRequest(NULL,NULL,NULL,GUILD_REQ_REMOVE_USER,user->self->GetID(),pPlayer->self->GetID(),1,0,"","");
               CPlayerManager::FreePlayerResource(pPlayer);
            }
            else
            {
               theApp.AddGuildRequest(NULL,NULL,NULL,GUILD_REQ_REMOVE_OFFLINE_USER,user->self->GetID(),dwID,0,0,"","");
            }
         }
         else
         {
            lpCharacter->SendInfoMessage( _STR( 15053, user->self->GetLang() ),0x0080FF);
         }
      }
   }
   RQ_FOOTER( "RQFUNC_NM_GuildKick" );
}

// Change user permission and titre...
void TFCMessagesHandler::RQFUNC_NM_GuildChangeSetting(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;
   DWORD dwID;
   GET_LONG( dwID );

   if(!user->self)
      return;

   if(theApp.dwGuildSystemEnable == 0 || (theApp.dwGuildSystemEnable == 2 && !user->IsGod()))
      user->self->SendInfoMessage( _STR( 15113 , user->self->GetLang() ),0x0020FF);
   else
   {
      // If user has a binded group
      Character *lpCharacter = static_cast< Character * >( user->self );
      if(lpCharacter)
      {
         uGuildPermission uPerm;
         uPerm.dwVal = lpCharacter->GetGuildPermission();

         if( uPerm.Permission.CanChPerm)
         {
            DWORD dwTitre;
            DWORD dwPerm;

            GET_LONG( dwTitre );
            GET_LONG( dwPerm );

            theApp.AddGuildRequest(NULL,NULL,NULL,GUILD_CHANGE_USER_SETTINGS,user->self->GetID(),dwID,dwTitre,dwPerm,"","");
         }
         else
         {
            lpCharacter->SendInfoMessage( _STR( 15053, user->self->GetLang() ),0x0080FF);
         }
      }
   }
   RQ_FOOTER( "RQFUNC_NM_GuildChangeSetting" );
}


// Change user permission and titre...
void TFCMessagesHandler::RQFUNC_NM_GuildGetLogs(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;
   if(!user->self)
      return;

   // If user has a binded group
   if(theApp.dwGuildSystemEnable == 0 || (theApp.dwGuildSystemEnable == 2 && !user->IsGod()))
   {
      user->self->SendInfoMessage( _STR( 15113 , user->self->GetLang() ),0x0020FF);
   }
   else
   {
      Character *lpCharacter = static_cast< Character * >( user->self );
      if(lpCharacter)
      {
         uGuildPermission uPerm;
         uPerm.dwVal = lpCharacter->GetGuildPermission();

         if( uPerm.Permission.CanSeeLog)
         {
            theApp.AddGuildRequest(NULL,NULL,NULL,GUILD_REQUEST_LOGS,user->self->GetID(),0,0,0,"","");
         }
         else
         {
            lpCharacter->SendInfoMessage( _STR( 15053, user->self->GetLang() ),0x0080FF);
         }
      }
   }
   RQ_FOOTER( "RQFUNC_NM_GuildGetLogs" );
}

void TFCMessagesHandler::RQFUNC_NM_GUILDChestAddItem(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   DWORD dwItemID, dwQty;

   if(!user->self)
      return;

   if(theApp.dwGuildSystemEnable == 0 || (theApp.dwGuildSystemEnable == 2 && !user->IsGod()))
   {
      user->self->SendInfoMessage( _STR( 15113 , user->self->GetLang() ),0x0020FF);
   }
   else
   {
      msg->Get(&dwItemID);
      msg->Get(&dwQty);

      if(user->self)
      {
         Character *lpCharacter = static_cast< Character * >( user->self );

         if (lpCharacter->GetIsGuildChesting() == FALSE) 
         {
            user->self->SendSystemMessage( _STR( 15092, user->self->GetLang() ) ); // You must click on a chest to use it
            user->self->StopUsingGuildChest();
            return;
         }

         if (lpCharacter->GetGameOpContext() != NULL) 
         {
            // Editing other user chest is not allowed for gameops. Perhaps we could add it later?
            user->self->SendSystemMessage( _STR( 12952, user->self->GetLang() ) );
            return;
         }

         uGuildPermission uPerm;
         uPerm.dwVal = user->self->GetGuildPermission();
         if(uPerm.Permission.CanDeposit == 0)
         {
            user->self->SendInfoMessage(_STR( 15053, user->self->GetLang() ),0x0080FF);
            return;
         }

         GuildMaster::MoveObjectFromBackpackToChest(lpCharacter,dwItemID, dwQty,FALSE);
      }

   }

   RQ_FOOTER( "RQ_NM_GUILDChestAddItem" );
}

void TFCMessagesHandler::RQFUNC_NM_GUILDChestRemoveItem(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;
   DWORD dwItemID, dwQty;

   if(!user->self)
      return;

   if(theApp.dwGuildSystemEnable == 0 || (theApp.dwGuildSystemEnable == 2 && !user->IsGod()))
   {
      user->self->SendInfoMessage( _STR( 15113 , user->self->GetLang() ),0x0020FF);
   }
   else
   {
      msg->Get(&dwItemID);
      msg->Get(&dwQty);

      if(user->self)
      {
         Character *lpCharacter = static_cast< Character * >( user->self );

         if (lpCharacter->GetIsGuildChesting() == FALSE) 
         {
            user->self->SendSystemMessage( _STR( 15092, user->self->GetLang() ) ); // You must click on a chest to use it
            user->self->StopUsingGuildChest();
            return;
         }

         if (lpCharacter->GetGameOpContext() != NULL) 
         {
            // Editing other user chest is not allowed for gameops. Perhaps we could add it later?
            user->self->SendSystemMessage( _STR( 12952, user->self->GetLang() ) );
            return;
         }

         uGuildPermission uPerm;
         uPerm.dwVal = user->self->GetGuildPermission();
         if(uPerm.Permission.CanTake == 0)
         {
            user->self->SendInfoMessage(_STR( 15053, user->self->GetLang() ),0x0080FF);
            return;
         }
         GuildMaster::MoveObjectFromChestToBackpack(lpCharacter,dwItemID, dwQty,FALSE);
      }
   }
   RQ_FOOTER( "RQ_NM_GUILDChestRemoveItem" );
}

void TFCMessagesHandler::RQFUNC_NM_GetAHList(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;
   if(!user->self)
      return;

   if(!user->IsGod())
      user->self->SendInfoMessage( _STR( 15053 , user->self->GetLang() ),0x0020FF);
   else if(theApp.dwAHSystemEnable == 0)
      user->self->SendInfoMessage( _STR( 15121 , user->self->GetLang() ),0x0020FF);
   else 
      theApp.AddAHRequest(NULL,NULL,NULL,AH_REQ_GET_LIST,user->self->GetID(),1,0,0,0,0,0,"","","",0);

   RQ_FOOTER( "RQFUNC_NM_GetAHList" );
}


// Change user permission and titre...
void TFCMessagesHandler::RQFUNC_NM_AddAHItems(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;
   if(!user->self)
      return;

   if(theApp.dwAHSystemEnable == 0)
      user->self->SendInfoMessage( _STR( 15121 , user->self->GetLang() ),0x0020FF);
   else
   {
      // If user has a binded group
      Character *lpCharacter = static_cast< Character * >( user->self );
      if(lpCharacter)
      {
         if(AuctionMaster::IsCanAddItems(lpCharacter))
         {
            DWORD dwItemID, dwQty, dwBuyNow,dwBuyNowNMS,dwBid,dwMaxTime;
            //on valide que l'item et la quantite sont OK...
            msg->Get(&dwItemID);
            msg->Get(&dwQty);
            msg->Get(&dwBuyNow);
            msg->Get(&dwBuyNowNMS);
            msg->Get(&dwBid);
            msg->Get(&dwMaxTime);

            /*
            if(dwBuyNowNMS > 0 && dwBid > 0) //pas de BID si vente avec ECU
               user->self->SendInfoMessage(_STR( 15505, user->self->GetLang() ),0x0080FF);
            else 
               */
            if(dwBuyNow <= 0 && dwBid <= 0 && dwBuyNowNMS <= 0)
               user->self->SendInfoMessage(_STR( 15126, user->self->GetLang() ),0x0080FF);
            else if(dwQty <=0)
               user->self->SendInfoMessage(_STR( 15130, user->self->GetLang() ),0x0080FF);
            else
            {
               CString ObjType,ObjName,MadeBy;
               DWORD   dwEquipedPos;
               long    lCharge;
               int iResult = lpCharacter->PutItemToAH(dwItemID,dwQty,ObjType,ObjName,dwEquipedPos,MadeBy,lCharge);
               if(iResult == 0)
               {
                  //l'item a ete virer du user...
                  //on dois ajouter a l'AH...

                  theApp.AddAHRequest(NULL,NULL,NULL,AH_ADD_ITEM,user->self->GetID(),dwQty,dwEquipedPos,dwBuyNow,dwBid,dwMaxTime,lCharge,ObjType,ObjName,MadeBy,dwBuyNowNMS);
               }
               else
               {
                  CString strErr;
                  switch(iResult)
                  {
                     case -1: strErr = _STR( 12918, user->self->GetLang() ); break;
                     case -2: strErr = _STR( 15124, user->self->GetLang() ); break;
                     case -3: strErr = _STR( 15125, user->self->GetLang() ); break;
                  }
                  user->self->SendInfoMessage(strErr,0x0080FF);
               }
            }
         }
         else
         {
            CString strMsg;
            strMsg.Format(_STR( 15123, user->self->GetLang() ),theApp.dwAHSystemMaxSold);

            user->self->SendInfoMessage(strMsg,0x0080FF);
         }
      }
   }
   RQ_FOOTER( "RQFUNC_NM_AddAHItems" );
}

void TFCMessagesHandler::RQFUNC_NM_BuyAHItems(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;
   if(!user->self)
      return;

   if(theApp.dwAHSystemEnable == 0)
      user->self->SendInfoMessage( _STR( 15121 , user->self->GetLang() ),0x0020FF);
   else
   {
      DWORD dwIndex;
      BYTE  chBuy;
      DWORD dwPrix;
      DWORD dwPrixNMS;
      DWORD dwTS;

      msg->Get((long  *)&dwIndex);
      msg->Get((char  *)&chBuy );
      msg->Get((long  *)&dwPrix);
      msg->Get((long  *)&dwPrixNMS);
      msg->Get((long  *)&dwTS);

      theApp.AddAHRequest(NULL,NULL,NULL,AH_BUY_ITEM,user->self->GetID(),dwIndex,chBuy,dwPrix,dwPrixNMS,dwTS,0,"","","",0);
   }   
   RQ_FOOTER( "RQFUNC_NM_BuyAHItems" );
}

void TFCMessagesHandler::RQFUNC_NM_CancelAHItems(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;
   if(!user->self)
      return;

   if(theApp.dwAHSystemEnable == 0)
      user->self->SendInfoMessage( _STR( 15121 , user->self->GetLang() ),0x0020FF);
   else
   {
      DWORD dwIndex;
      DWORD dwTS;
      msg->Get((long  *)&dwIndex);
      msg->Get((long  *)&dwTS);
      theApp.AddAHRequest(NULL,NULL,NULL,AH_CANCEL_ITEM,user->self->GetID(),dwIndex,dwTS,0,0,0,0,"","","",0);
   }

   RQ_FOOTER( "RQFUNC_NM_CancelAHItems" );
}

void TFCMessagesHandler::RQFUNC_NM_InfoAHItems(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;
   if(!user->self)
      return;

   if(theApp.dwAHSystemEnable == 0)
      user->self->SendInfoMessage( _STR( 15121 , user->self->GetLang() ),0x0020FF);
   else
   {
      DWORD dwIndex;
      msg->Get((long  *)&dwIndex);
      theApp.AddAHRequest(NULL,NULL,NULL,AH_INFO_ITEM,user->self->GetID(),dwIndex,0,0,0,0,0,"","","",0);
   }
   RQ_FOOTER( "RQFUNC_NM_InfoAHItems" );
}

void TFCMessagesHandler::RQFUNC_NM_NMSGOLD_Acheter(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   if(!user->self)
      return;

   if(theApp.dwNMSGOLDEnable == 0)
   {
      user->self->SendInfoMessage( _STR( 15343 , user->self->GetLang() ),0x0020FF);
      return;
   }

   DWORD dwOpt;
   DWORD dwItem;
   GET_LONG(dwOpt);
   GET_LONG(dwItem);


   if(user->self)
   {
      if(dwOpt <4)
      {
         //deja le type de list est bon...
         if(dwOpt == 1) //Upgrade player
         {
            if(dwItem < theApp.m_aAchatOpt1.GetSize())
            {
               //1- on check si le player a assez de NMS Gold...
               if(user->GetNMSGold() >=  theApp.m_aAchatOpt1[dwItem].iCost)
               {
                  //on valide que ya pas deja un achat latent sur ce flags...
                  DWORD dwFlagVal = user->GetNMSGoldFlag( theApp.m_aAchatOpt1[dwItem].iFlagMod );
                  if(dwFlagVal == 0)
                  {

                     //On procede a la transanction...
                     int iNbrNMSGOLDRemain = user->GetNMSGold()-theApp.m_aAchatOpt1[dwItem].iCost;

                     user->SetNMSGold(iNbrNMSGOLDRemain);
                     user->SaveAccount();

                     if(!user->SetNMSGoldFlag( theApp.m_aAchatOpt1[dwItem].iFlagMod , theApp.m_aAchatOpt1[dwItem].iFlagValue))
                     {
                        //set un flag Standard...
                        user->self->SetFlag(theApp.m_aAchatOpt1[dwItem].iFlagMod , theApp.m_aAchatOpt1[dwItem].iFlagValue);
                     }

                     user->self->SendInfoMessage( theApp.m_aAchatOpt1[dwItem].strName,CL_YELLOW);
                     user->self->SendInfoMessage( _STR( theApp.m_aAchatOpt1[dwItem].iMessageID , user->self->GetLang() ),CL_YELLOW);
                     user->SaveAccount();
                     user->ForceSave(); //NMSGold

                     _LOG_ACHAT_NMS
                        LOG_ALWAYS,
                        "Player %s (%s) BUY[1] %s",
                        (LPCTSTR)user->self->GetTrueName(),
                        (LPCTSTR)user->GetFullAccountName(),
                        theApp.m_aAchatOpt1[dwItem].strName
                        LOG_


                        //Reupdate the Opt1 List
                        RQFUNC_NM_NMSGOLD_AchatOpt1(NULL, user, rqRequestID,sockAddrO,sockAddrI);
                  }
                  else
                  {
                     user->self->SendInfoMessage( _STR( 15296 , user->self->GetLang() ),CL_RED);
                  }
               }
               else
               {
                  user->self->SendInfoMessage( _STR( 15295 , user->self->GetLang() ),CL_RED);
               }
            }
            else
            {
               //Item Imnvalides...
               user->self->SendInfoMessage( _STR( 15294 , user->self->GetLang() ),CL_RED);

               _LOG_DEBUG
                  LOG_DEBUG_LVL1,
                  "Player (%s) , Account (%s) error NMSGOLD (Invalid Item)...", 
                  user->self->GetTrueName(),
                  user->GetAccount()
                  LOG_
            }
         }
         else if(dwOpt == 2) //Item Fixe
         {
            if(dwItem < theApp.m_aAchatOpt2.GetSize())
            {
               //1- on check si le player a assez de NMS Gold...
               if(user->GetNMSGold() >=  theApp.m_aAchatOpt2[dwItem].iCost)
               {
                  //On procede a la transanction...
                  int iNbrNMSGOLDRemain = user->GetNMSGold()-theApp.m_aAchatOpt2[dwItem].iCost;

                  user->SetNMSGold(iNbrNMSGOLDRemain);
                  user->SaveAccount();

                  //oki rendu ici suffit de summon l'item dans l'inventaire du gus...
                  //
                  BOOL boInvalidItem = FALSE;
                  DWORD dwID       = theApp.m_aAchatOpt2[dwItem].iItemID;
                  DWORD dwQty      = theApp.m_aAchatOpt2[dwItem].iNbrItem;
                  DWORD dwIDBonus  = theApp.m_aAchatOpt2[dwItem].iItemIDBonus;
                  DWORD dwQtyBonus = theApp.m_aAchatOpt2[dwItem].iNbrItemBonus;
                  if( dwID != 0)
                  {
                     Objects *lpItem = new Objects;
                     if( lpItem->Create( U_OBJECT, dwID ) )
                     {
                        _item *item = NULL;
                        lpItem->SendUnitMessage( MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &item );

                        if ( ! lpItem->IsUnique() && dwQty > 0) 
                        {
                           lpItem->SetQty(dwQty);
                        }
                        user->self->AddToBackpack( lpItem );

                        //If Bonus item need to create and summon it to....
                        if(dwIDBonus != 0 && dwQtyBonus >0)
                        {
                           Objects *lpItemBonus = new Objects;
                           if( lpItemBonus->Create( U_OBJECT, dwIDBonus ) )
                           {
                              _item *item = NULL;
                              lpItemBonus->SendUnitMessage( MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &item );

                              if ( ! lpItemBonus->IsUnique() && dwQtyBonus > 0) 
                              {
                                 lpItemBonus->SetQty(dwQtyBonus);
                              }
                              user->self->AddToBackpack( lpItemBonus );
                           }
                        }
                       


                        TFCPacket sending;
                        sending << (RQ_SIZE)RQ_ViewBackpack2;
                        sending << (char)0;	// Don't show backpack..!!
                        sending << (long)user->self->GetID();
                        user->self->PacketBackpack( sending );
                        user->self->SendPlayerMessage( sending );

                        sending.Destroy();
                        user->self->packet_equiped( sending );
                        user->self->SendPlayerMessage( sending );

                        user->self->SendInfoMessage( theApp.m_aAchatOpt2[dwItem].strName,CL_YELLOW);
                        if(dwIDBonus != 0 && dwQtyBonus >0)
                           user->self->SendInfoMessage( theApp.m_aAchatOpt2[dwItem].strNameBonus,CL_YELLOW);

                        user->self->SendInfoMessage( _STR( 15297 , user->self->GetLang() ),CL_YELLOW);

                        user->SaveAccount();
                        user->ForceSave();//NMSGold

                        _LOG_ACHAT_NMS
                           LOG_ALWAYS,
                           "Player %s (%s) BUY[2] %s Qty %d  [Bonus (%s) Qty (%d)]",
                           (LPCTSTR)user->self->GetTrueName(),
                           (LPCTSTR)user->GetFullAccountName(),
                           theApp.m_aAchatOpt2[dwItem].strName,theApp.m_aAchatOpt2[dwItem].iNbrItem,
                           theApp.m_aAchatOpt2[dwItem].strNameBonus,theApp.m_aAchatOpt2[dwItem].iNbrItemBonus
                           LOG_

                           long lNbrNMSGold = user->GetNMSGold();


                        //Reupdate the Opt2 List
                        RQFUNC_NM_NMSGOLD_AchatOpt2(NULL, user, rqRequestID,sockAddrO,sockAddrI);
                     }
                     else
                     {
                        boInvalidItem = TRUE;
                        lpItem->DeleteUnit();
                     }
                  }
                  else
                  {
                     boInvalidItem = TRUE;	
                  }
               }
               else
               {
                  user->self->SendInfoMessage( _STR( 15295 , user->self->GetLang() ),CL_RED);
               }
            }
            else
            {
               //Item Imnvalides...
               user->self->SendInfoMessage( _STR( 15294 , user->self->GetLang() ),CL_RED);

               _LOG_DEBUG
                  LOG_DEBUG_LVL1,
                  "Player (%s) , Account (%s) error NMSGOLD (Invalid Item)...", 
                  user->self->GetTrueName(),
                  user->GetAccount()
                  LOG_
            }
         }
         else if(dwOpt == 3) //Item Construction
         {
            if(dwItem < theApp.m_aAchatOpt3.GetSize())
            {
               //1- on check si le player a assez de NMS Gold...
               if(user->GetNMSGold() >=  theApp.m_aAchatOpt3[dwItem].iCost)
               {
                  //On procede a la transanction...
                  int iNbrNMSGOLDRemain = user->GetNMSGold()-theApp.m_aAchatOpt3[dwItem].iCost;

                  user->SetNMSGold(iNbrNMSGOLDRemain);
                  user->SaveAccount();

                  user->ForceSave();//NMSGold


                  _LOG_ACHAT_NMS
                     LOG_ALWAYS,
                     "Player %s (%s) BUY[3] %s",
                     (LPCTSTR)user->self->GetTrueName(),
                     (LPCTSTR)user->GetFullAccountName(),
                     theApp.m_aAchatOpt3[dwItem].strName
                     LOG_

                     //Send a message to player with info...
                     user->self->SendInfoMessage( _STR( 15315 , user->self->GetLang() ),CL_YELLOW);

                  //Reupdate the Opt3 List
                  RQFUNC_NM_NMSGOLD_AchatOpt3(NULL, user, rqRequestID,sockAddrO,sockAddrI);
               }
               else
               {
                  user->self->SendInfoMessage( _STR( 15295 , user->self->GetLang() ),CL_RED);
               }
            }
            else
            {
               //Item Imnvalides...
               user->self->SendInfoMessage( _STR( 15294 , user->self->GetLang() ),CL_RED);

               _LOG_DEBUG
                  LOG_DEBUG_LVL1,
                  "Player (%s) , Account (%s) error NMSGOLD (Invalid Item)...", 
                  user->self->GetTrueName(),
                  user->GetAccount()
                  LOG_
            }
         }
      }
      else
      {
         //List invalide...
         user->self->SendInfoMessage( _STR( 15294 , user->self->GetLang() ),CL_RED);

         _LOG_DEBUG
            LOG_DEBUG_LVL1,
            "Player (%s) , Account (%s) error NMSGOLD (Invalid OPT List)...", 
            user->self->GetTrueName(),
            user->GetAccount()
            LOG_
      }
   }

   RQ_FOOTER( "RQFUNC_NM_NMSGOLD_Acheter" );
}


void TFCMessagesHandler::RQFUNC_NM_NMSGOLD_ListPanier(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   //en premier check les flags pour les reroll car le plus souvent utiliser...
   typedef struct _sPanierB
   {
      CString strName;
      CString strInf;
      char    chCanCredite;
      int     iflag;
   }sPanierB;

   CArray<sPanierB,sPanierB> m_Panier; //Upgrade kit

   //remplie la list du panier...
   int iNbrReroll = user->GetNMSGoldReroll();
   if(iNbrReroll >0)
   {
      //ya des reroll...
      sPanierB np;
      np.strName.Format("%s",_STR( 15298, user->self->GetLang() ));
      if(iNbrReroll == 1000)
         np.strInf .Format("%s",_STR( 15300, user->self->GetLang() ));
      else
         np.strInf .Format("%s %d",_STR( 15299, user->self->GetLang() ),iNbrReroll);
      np.iflag        = ciNMSGoldReroll;
      np.chCanCredite = false;
      m_Panier.Add(np);
   }

   //Nbr Seraph Good Level 1
   int iNbrSG1 = user->GetNMSGoldSGLv1();
   if(iNbrSG1 >0)
   {
      //ya des reroll...
      sPanierB np;
      np.strName.Format("%s",_STR( 15301, user->self->GetLang() ));
      np.strInf .Format("X %d",iNbrSG1);
      np.iflag        = ciNMSGoldSGLv1;
      np.chCanCredite = true;
      m_Panier.Add(np);
   }

   //Nbr Seraph Good 
   int iNbrSG = user->GetNMSGoldSG();
   if(iNbrSG >0)
   {
      //ya des reroll...
      sPanierB np;
      np.strName.Format("%s",_STR( 15302, user->self->GetLang() ));
      np.strInf .Format("X %d",iNbrSG);
      np.iflag        = ciNMSGoldSG;
      np.chCanCredite = true;
      m_Panier.Add(np);
   }

   //Nbr Seraph Evil Level 1
   int iNbrSE1 = user->GetNMSGoldSELv1();
   if(iNbrSE1 >0)
   {
      //ya des reroll...
      sPanierB np;
      np.strName.Format("%s",_STR( 15303, user->self->GetLang() ));
      np.strInf .Format("X %d",iNbrSE1);
      np.iflag        = ciNMSGoldSELv1;
      np.chCanCredite = true;
      m_Panier.Add(np);
   }

   //Nbr Seraph Evil
   int iNbrSE = user->GetNMSGoldSE();
   if(iNbrSE >0)
   {
      //ya des reroll...
      sPanierB np;
      np.strName.Format("%s",_STR( 15304, user->self->GetLang() ));
      np.strInf .Format("X %d",iNbrSE);
      np.iflag        = ciNMSGoldSE;
      np.chCanCredite = true;
      m_Panier.Add(np);
   }

   //Nbr Dechu
   int iPassageDechu = user->GetNMSGoldToD();
   if(iPassageDechu >0)
   {
      //ya des reroll...
      sPanierB np;
      np.strName.Format("%s",_STR( 15305, user->self->GetLang() ));
      np.strInf .Format("1");
      np.iflag        = ciNMSGoldToD;
      np.chCanCredite = true;
      m_Panier.Add(np);
   }

   //Nbr DechuLv1
   int iPassageDechuLv1 = user->GetNMSGoldToDLv1();
   if(iPassageDechuLv1 >0)
   {
      //ya des reroll...
      sPanierB np;
      np.strName.Format("%s",_STR( 15336, user->self->GetLang() ));
      np.strInf .Format("1");
      np.iflag        = ciNMSGoldToDLv1;
      np.chCanCredite = true;
      m_Panier.Add(np);
   }


   long lNbrNMSGold = user->GetNMSGold();

   TFCPacket sending;
   sending << (RQ_SIZE)RQ_NM_NMSGOLD_ListPanier;
   sending << (long)lNbrNMSGold;

   //the number of items
   unsigned short nbrItems = m_Panier.GetSize();
   sending << (short)nbrItems;

   for(int i=0;i<nbrItems;i++)
   {
      //send name + Desc + prix
      sending << m_Panier[i].strName;
      sending << m_Panier[i].strInf;
      sending << (long)m_Panier[i].iflag;
      sending << (char)m_Panier[i].chCanCredite;
   }
   user->self->SendPlayerMessage(sending);
   m_Panier.RemoveAll();


   RQ_FOOTER( "RQFUNC_NM_NMSGOLD_ListPanier" );
}


void TFCMessagesHandler::RQFUNC_NM_NMSGOLD_UtiliserPanier(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   if(theApp.dwNMSGOLDEnable == 0)
   {
      user->self->SendInfoMessage( _STR( 15343 , user->self->GetLang() ),0x0020FF);
      return;
   }
   if(!user->self)
      return;

   DWORD dwFlag;
   GET_LONG(dwFlag);


   bool bFound = false;

   switch(dwFlag) 
   {
      case ciNMSGoldSGLv1:
      {
         if(user->GetNMSGoldSGLv1() >0)
         {
            bFound = true;
            //oki rendu ici on peu passer a Déchu...
            int iErr = theApp.NMSGOLD_Remort(user->self,1315, 920, 1 ,0,true);
            if(iErr == 0)
            {
               user->self->SendInfoMessage( _STR( 15313 , user->self->GetLang() ),CL_YELLOW);
               user->SaveAccount();
               user->ForceSave();//NMSGold Utiliser panier

               _LOG_ACHAT_NMS
                  LOG_ALWAYS,
                  "Player %s (%s) use REBORN LVL 1 GOOD",
                  (LPCTSTR)user->self->GetTrueName(),
                  (LPCTSTR)user->GetFullAccountName()
                  LOG_
                  //reenvoie la maj des panier de ce pj...
                  RQFUNC_NM_NMSGOLD_ListPanier(NULL, user, rqRequestID,sockAddrO,sockAddrI);
            }
            else if(iErr == -3)
               user->self->SendInfoMessage( _STR( 15335 , user->self->GetLang() ),CL_RED);
            else if(iErr == -2)
               user->self->SendInfoMessage( _STR( 15316 , user->self->GetLang() ),CL_RED);
            else
               user->self->SendInfoMessage( _STR( 15334 , user->self->GetLang() ),CL_RED);
         }
      }
      break;
      case ciNMSGoldSG:
      {
         if(user->GetNMSGoldSG() >0)
         {
            bFound = true;
            //oki rendu ici on peu passer a Déchu...
            int iErr = theApp.NMSGOLD_Remort(user->self,1315, 920, 1 ,0,false);
            if(iErr == 0)
            {
               user->self->SendInfoMessage( _STR( 15313 , user->self->GetLang() ),CL_YELLOW);
               user->SaveAccount();
               user->ForceSave();//NMSGold Utiliser panier

               _LOG_ACHAT_NMS
                  LOG_ALWAYS,
                  "Player %s (%s) use REBORN GOOD",
                  (LPCTSTR)user->self->GetTrueName(),
                  (LPCTSTR)user->GetFullAccountName()
                  LOG_
                  //reenvoie la maj des panier de ce pj...
                  RQFUNC_NM_NMSGOLD_ListPanier(NULL, user, rqRequestID,sockAddrO,sockAddrI);
            }
            else if(iErr == -3)
               user->self->SendInfoMessage( _STR( 15335 , user->self->GetLang() ),CL_RED);
            else if(iErr == -2)
               user->self->SendInfoMessage( _STR( 15316 , user->self->GetLang() ),CL_RED);
            else
               user->self->SendInfoMessage( _STR( 15334 , user->self->GetLang() ),CL_RED);
         }
      }
      break;
      case ciNMSGoldSELv1:
      {
         if(user->GetNMSGoldSELv1() >0)
         {
            bFound = true;
            //oki rendu ici on peu passer a Déchu...
            int iErr = theApp.NMSGOLD_Remort(user->self,1315, 920, 1 ,1,true);
            if(iErr == 0)
            {
               user->self->SendInfoMessage( _STR( 15314 , user->self->GetLang() ),CL_YELLOW);
               user->SaveAccount();
               user->ForceSave();//NMSGold Utiliser panier

               _LOG_ACHAT_NMS
                  LOG_ALWAYS,
                  "Player %s (%s) use REBORN LVL 1 EVIL",
                  (LPCTSTR)user->self->GetTrueName(),
                  (LPCTSTR)user->GetFullAccountName()
                  LOG_
                  //reenvoie la maj des panier de ce pj...
                  RQFUNC_NM_NMSGOLD_ListPanier(NULL, user, rqRequestID,sockAddrO,sockAddrI);
            }
            else if(iErr == -3)
               user->self->SendInfoMessage( _STR( 15335 , user->self->GetLang() ),CL_RED);
            else if(iErr == -2)
               user->self->SendInfoMessage( _STR( 15316 , user->self->GetLang() ),CL_RED);
            else
               user->self->SendInfoMessage( _STR( 15334 , user->self->GetLang() ),CL_RED);
         }
      }
      break;
      case ciNMSGoldSE:
      {
         if(user->GetNMSGoldSE() >0)
         {
            bFound = true;
            //oki rendu ici on peu passer a Déchu...
            int iErr = theApp.NMSGOLD_Remort(user->self,1315, 920, 1 ,1,false);
            if(iErr == 0)
            {
               user->self->SendInfoMessage( _STR( 15314 , user->self->GetLang() ),CL_YELLOW);
               user->SaveAccount();
               user->ForceSave();//NMSGold Utiliser panier

               _LOG_ACHAT_NMS
                  LOG_ALWAYS,
                  "Player %s (%s) use REBORN EVIL",
                  (LPCTSTR)user->self->GetTrueName(),
                  (LPCTSTR)user->GetFullAccountName()
                  LOG_
                  //reenvoie la maj des panier de ce pj...
                  RQFUNC_NM_NMSGOLD_ListPanier(NULL, user, rqRequestID,sockAddrO,sockAddrI);
            }
            else if(iErr == -3)
               user->self->SendInfoMessage( _STR( 15335 , user->self->GetLang() ),CL_RED);
            else if(iErr == -2)
               user->self->SendInfoMessage( _STR( 15316 , user->self->GetLang() ),CL_RED);
            else
               user->self->SendInfoMessage( _STR( 15334 , user->self->GetLang() ),CL_RED);
         }
      }
      break;
      case ciNMSGoldReroll:
      {
         if(user->GetNMSGoldReroll() >0)
         {
            bFound = true;
            //oki rendu ici on peu passer a Déchu...
            int iErr = theApp.NMSGOLD_Reroll(user->self,1315, 920, 1 );
            if(iErr == 0)
            {
               user->self->SendInfoMessage( _STR( 15312 , user->self->GetLang() ),CL_YELLOW);
               user->SaveAccount();
               user->ForceSave();//NMSGold Utiliser panier


               _LOG_ACHAT_NMS
                  LOG_ALWAYS,
                  "Player %s (%s) use REROLL...",
                  (LPCTSTR)user->self->GetTrueName(),
                  (LPCTSTR)user->GetFullAccountName()
                  LOG_
                  //reenvoie la maj des panier de ce pj...
                  RQFUNC_NM_NMSGOLD_ListPanier(NULL, user, rqRequestID,sockAddrO,sockAddrI);
            }
            else
               user->self->SendInfoMessage( _STR( 15334 , user->self->GetLang() ),CL_RED);
         }
      }
      break;
      case ciNMSGoldToD:
      {
         if(user->GetNMSGoldToD() > 0)
         {
            bFound = true;
            if(user->self->ViewFlag( __FLAG_NUMBER_OF_REMORTS ) ==5)
            {
               //oki rendu ici on peu passer a Déchu...
               int iErr = theApp.NMSGOLD_PassageDechu(user->self,1315, 920, 1 );
               if(iErr == 0)
               {

                  user->self->SendInfoMessage( _STR( 15311 , user->self->GetLang() ),CL_YELLOW);
                  user->SaveAccount();
                  user->ForceSave();//NMSGold Utiliser panier

                  _LOG_ACHAT_NMS
                     LOG_ALWAYS,
                     "Player %s (%s) use PASSAGE à Déchu...",
                     (LPCTSTR)user->self->GetTrueName(),
                     (LPCTSTR)user->GetFullAccountName()
                     LOG_
                     //reenvoie la maj des panier de ce pj...
                     RQFUNC_NM_NMSGOLD_ListPanier(NULL, user, rqRequestID,sockAddrO,sockAddrI);
               }
               else if(iErr == -2)
                  user->self->SendInfoMessage( _STR( 15309 , user->self->GetLang() ),CL_RED);
               else
                  user->self->SendInfoMessage( _STR( 15334 , user->self->GetLang() ),CL_RED);
            }
            else
            {
               user->self->SendInfoMessage( _STR( 15310 , user->self->GetLang() ),CL_RED);
            }
         }
      }
      break;
      case ciNMSGoldToDLv1:
      {
         if(user->GetNMSGoldToDLv1() > 0)
         {
            bFound = true;

            if(user->self->ViewFlag( __FLAG_NUMBER_OF_REMORTS ) ==5)
            {
               //oki rendu ici on peu passer a Déchu...
               //on set manuekllement le pj en dechu pour le faire renaitre par la suite en dechu...

               user->SetNMSGoldSDLv1(user->GetNMSGoldToDLv1()+user->GetNMSGoldSDLv1());
               user->SetNMSGoldToDLv1(0);
               int iNbrSeraphBefore = user->self->ViewFlag( __FLAG_NUMBER_OF_REMORTS  );
               int iAlignBefore     = user->self->ViewFlag(__FLAG_USER_ALIGNMENT);

               user->self->SetFlag( __FLAG_NMS_DECHU         ,5); //Set le flag dechu a Dechu X5...
               user->self->SetFlag( __FLAG_NUMBER_OF_REMORTS ,0); //Set 0 Seraph
               user->self->SetFlag( __FLAG_USER_ALIGNMENT    ,0); //Set 0 a alignement car neutre en dechu...


               int iErr = theApp.NMSGOLD_Remort(user->self,1315, 920, 1 ,2,true);
               if(iErr == 0)
               {
                  user->self->SendInfoMessage( _STR( 15341 , user->self->GetLang() ),CL_YELLOW);
                  user->SaveAccount();
                  user->ForceSave();//NMSGold Utiliser panier

                  _LOG_ACHAT_NMS
                     LOG_ALWAYS,
                     "Player %s (%s) use PASSAGE à Déchu Lvl 1...",
                     (LPCTSTR)user->self->GetTrueName(),
                     (LPCTSTR)user->GetFullAccountName()
                     LOG_

                     //reenvoie la maj des panier de ce pj...
                     RQFUNC_NM_NMSGOLD_ListPanier(NULL, user, rqRequestID,sockAddrO,sockAddrI);
               }
               else
               {
                  user->SetNMSGoldSDLv1(user->GetNMSGoldSDLv1()-1);
                  user->SetNMSGoldToDLv1(1);
                  //reset les flag a ce quil etait avant...
                  user->self->SetFlag( __FLAG_NMS_DECHU         ,0);
                  user->self->SetFlag( __FLAG_NUMBER_OF_REMORTS ,iNbrSeraphBefore);
                  user->self->SetFlag( __FLAG_USER_ALIGNMENT    ,iAlignBefore);

                  if(iErr == -3)
                     user->self->SendInfoMessage( _STR( 15335 , user->self->GetLang() ),CL_RED);
                  else if(iErr == -2)
                     user->self->SendInfoMessage( _STR( 15316 , user->self->GetLang() ),CL_RED);
                  else
                     user->self->SendInfoMessage( _STR( 15334 , user->self->GetLang() ),CL_RED);
               }
            }
            else
            {
               user->self->SendInfoMessage( _STR( 15310 , user->self->GetLang() ),CL_RED);
            }
         }
      }
      break;
   }

   if(!bFound)
   {
      //ce flag n'est pas creditable...
      //List invalide...
      user->self->SendInfoMessage( _STR( 15308 , user->self->GetLang() ),CL_RED);

      _LOG_DEBUG
         LOG_DEBUG_LVL1,
         "Player (%s) , Account (%s) error NMSGOLD (Try credit invalid flag ID [%d])...", 
         user->self->GetTrueName(),
         user->GetAccount(),
         dwFlag
         LOG_
   }


   RQ_FOOTER( "RQFUNC_NM_NMSGOLD_UtiliserPanier" );
}

void TFCMessagesHandler::RQFUNC_NM_NMSGOLD_CrediterPanier(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   if(theApp.dwNMSGOLDEnable == 0)
   {
      user->self->SendInfoMessage( _STR( 15343 , user->self->GetLang() ),0x0020FF);
      return;
   }
   if(!user->self)
      return;


   DWORD dwFlag;
   GET_LONG(dwFlag);


   bool bFound = false;

   //1 check if the flag are a creditable flag ID....
   if(dwFlag == ciNMSGoldSGLv1 ||
      dwFlag == ciNMSGoldSG    ||
      dwFlag == ciNMSGoldSELv1 ||
      dwFlag == ciNMSGoldSE    ||
      dwFlag == ciNMSGoldToD   ||
      dwFlag == ciNMSGoldToDLv1    )
   {
      //oki on peu crediter cet achat...
      //on scan la liste des achat upgrade et rembourse selon le prix courant...
      int iFlagValue = user->GetNMSGoldFlag( dwFlag );
      for(int i=0;i<theApp.m_aAchatOpt1.GetSize();i++)
      {
         if(theApp.m_aAchatOpt1[i].iFlagMod == dwFlag)
         {
            if(theApp.m_aAchatOpt1[i].iFlagValue == iFlagValue || dwFlag == ciNMSGoldToD || dwFlag == ciNMSGoldToDLv1)
            {
               //Found the good value...
               int iNeedCredite = theApp.m_aAchatOpt1[i].iCost;
               if(user->GetNMSGoldFlag( dwFlag+10 ) >0)
                  iNeedCredite -=100; // il a  utiliser uen  partie des points on retire la penalite de 100 nmsgold...

               int iNbrNMSGold = user->GetNMSGold();

               //Redonne les NMSGold
               user->SetNMSGold(iNbrNMSGold+iNeedCredite);

               //reset le flags...
               user->SetNMSGoldFlag( dwFlag    ,0);
               user->SetNMSGoldFlag( dwFlag+10 ,0);

               //Avertie le player
               CString strInfo;
               strInfo.Format(_STR( 15307, user->self->GetLang() ),theApp.m_aAchatOpt1[i].strName,iNeedCredite);
               user->self->SendInfoMessage( strInfo,CL_YELLOW);

               //Force un save de ce player
               user->SaveAccount();
               user->ForceSave();//NMSGold Crediter

               _LOG_ACHAT_NMS
                  LOG_ALWAYS,
                  "Player %s (%s) receive CREDIT %s [%d NMSGold]",
                  (LPCTSTR)user->self->GetTrueName(),
                  (LPCTSTR)user->GetFullAccountName(),
                  theApp.m_aAchatOpt1[i].strName,
                  iNeedCredite
                  LOG_

                  bFound = true;
               i = theApp.m_aAchatOpt1.GetSize();


               //reenvoie la maj des panier de ce pj...
               RQFUNC_NM_NMSGOLD_ListPanier(NULL, user, rqRequestID,sockAddrO,sockAddrI);
            }
         }
      }
   }

   if(!bFound)
   {
      //ce flag n'est pas creditable...
      //List invalide...
      user->self->SendInfoMessage( _STR( 15306 , user->self->GetLang() ),CL_RED);

      _LOG_DEBUG
         LOG_DEBUG_LVL1,
         "Player (%s) , Account (%s) error NMSGOLD (Try credit invalid flag ID [%d])...", 
         user->self->GetTrueName(),
         user->GetAccount(),
         dwFlag
         LOG_
   }


   RQ_FOOTER( "RQFUNC_NM_NMSGOLD_CrediterPanier" );
}

void TFCMessagesHandler::RQFUNC_RQ_RP_BroadCastPVP(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;
   if(!theApp.m_dwRPSystem)
      return;

   if(!user->self)
      return;

   //additionner tous les type d arene
   short shNbr = theApp.CombatArenaLocationList1.size();
   shNbr += theApp.CombatArenaLocationList2.size();

   TFCPacket sending;
   sending << (RQ_SIZE)RQ_RP_BroadCastPVP;
   sending << (short)shNbr;

   //shoot le type 1...
   list< sCombatArenaLocation >::iterator itA = theApp.CombatArenaLocationList1.begin();
   for(int i=0;i<theApp.CombatArenaLocationList1.size();i++)
   {
      sending << (char)i;
      sending << (char)ARENE1_TYPE;
      sending << (*itA).strZOneName;
      CString strDesc;
      strDesc.Format(_STR( (*itA).iDescMsgID , user->self->GetLang() ),Arena1Master::GetMaxPoints(i),Arena1Master::GetMaxMinutes(i),Arena1Master::GetDeathWaitTimeS(i));
      sending << strDesc;
      sending << (char)theApp.m_dwArenaSystem1[i];
      itA++;
   }
   //shoot type 2...
   itA = theApp.CombatArenaLocationList2.begin();
   for(int i=0;i<theApp.CombatArenaLocationList2.size();i++)
   {
      sending << (char)i;
      sending << (char)ARENE2_TYPE;
      sending << (*itA).strZOneName;
      CString strDesc;
      strDesc.Format(_STR( (*itA).iDescMsgID , user->self->GetLang() ),Arena2Master::GetMaxPoints(i),Arena2Master::GetMaxMinutes(i),Arena2Master::GetDeathWaitTimeS(i));
      sending << strDesc;
      sending << (char)theApp.m_dwArenaSystem2[i];
      itA++;
   }
   user->self->SendPlayerMessage(sending);
   RQ_FOOTER( "RQFUNC_RQ_RP_BroadCastPVP" );
}

void TFCMessagesHandler::RQFUNC_RQ_RP_BroadCastPVPStat(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;
   if(!theApp.m_dwRPSystem)
      return;

   if(!user->self)
      return;

   int iNbrSec,iNbrMin;

   //additionner tous les type d arene
   short shNbr = theApp.CombatArenaLocationList1.size();
   shNbr += theApp.CombatArenaLocationList2.size();

   TFCPacket sending;
   sending << (RQ_SIZE)RQ_RP_BroadCastPVPStat;
   sending << (short)shNbr;

   //shoot le type 1...
   list< sCombatArenaLocation >::iterator itA = theApp.CombatArenaLocationList1.begin();
   for(int i=0;i<theApp.CombatArenaLocationList1.size();i++)
   {
      sending << (char)theApp.m_dwArenaSystem1[i];
      sending << (char)Arena1Master::GetArenaStatus(i);
      sending << (short)Arena1Master::GetArenaMinLevel(i);
      sending << (short)Arena1Master::GetArenaMaxLevel(i);
      Arena1Master::GetArenaWaitTimeGen(i,iNbrSec,iNbrMin);
      sending << (char)iNbrSec;
      sending << (char)iNbrMin;
      itA++;
   } 
   //shoot type 2...
   itA = theApp.CombatArenaLocationList2.begin();
   for(int i=0;i<theApp.CombatArenaLocationList2.size();i++)
   {
      sending << (char)theApp.m_dwArenaSystem2[i];
      sending << (char)Arena2Master::GetArenaStatus(i);
      sending << (short)Arena2Master::GetArenaMinLevel(i);
      sending << (short)Arena2Master::GetArenaMaxLevel(i);
      Arena2Master::GetArenaWaitTimeGen(i,iNbrSec,iNbrMin);
      sending << (char)iNbrSec;
      sending << (char)iNbrMin;
      itA++;
   }

   user->self->SendPlayerMessage(sending);
   RQ_FOOTER( "RQFUNC_RQ_RP_BroadCastPVPStat" );
}




void TFCMessagesHandler::RQFUNC_RQ_RP_BroadCastRP(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;
   if(!theApp.m_dwRPSystem)
      return;

   if(!user->self)
      return;

   // Create new player message structure
   LPASYNC_PACKET_FUNC_PARAMSEX lpParams = new ASYNC_PACKET_FUNC_PARAMSEX;
   lpParams->user        = user;

   // Call asynchronous loading function
   AsyncFuncQueue::GetMainQueue()->Call( AsyncRQFUNC_RQ_RP_BroadCastRP, lpParams );

   RQ_FOOTER( "RQFUNC_RQ_RP_BroadCastRP" );
}

void TFCMessagesHandler::RQFUNC_RQ_RP_CreerRP(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;
   if(!theApp.m_dwRPSystem)
   {
      user->self->SendInfoMessage( _STR( 15417 , user->self->GetLang() ),CL_RED);
      return;
   }

   if(!user->self)
      return;

   CString strMessage;
   LPBYTE lpMessage = NULL;
   GET_STRING( lpMessage );
   if(strlen((char*)lpMessage) >255)
      lpMessage[254] = 0x00;
   strMessage.Format("%s",lpMessage);
   if(lpMessage)
      delete []lpMessage;
   lpMessage = NULL;

   // Create new player message structure
   LPASYNC_PACKET_FUNC_PARAMSEX lpParams = new ASYNC_PACKET_FUNC_PARAMSEX;
   lpParams->user        = user;
   lpParams->strParam1   = strMessage;

   // Call asynchronous loading function
   AsyncFuncQueue::GetMainQueue()->Call( AsyncRQFUNC_RQ_RP_CreerRP, lpParams );


   RQ_FOOTER( "RQFUNC_RQ_RP_CreerRP" );
}


void TFCMessagesHandler::RQFUNC_RQ_RP_TerminerRP(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;
   if(!theApp.m_dwRPSystem)
      return;

   if(!user->self)
      return;

   // Create new player message structure
   LPASYNC_PACKET_FUNC_PARAMSEX lpParams = new ASYNC_PACKET_FUNC_PARAMSEX;
   lpParams->user        = user;

   // Call asynchronous loading function
   AsyncFuncQueue::GetMainQueue()->Call( AsyncRQFUNC_RQ_RP_TerminerRP, lpParams );

   RQ_FOOTER( "RQFUNC_RQ_RP_TerminerRP" );
}

void TFCMessagesHandler::RQFUNC_RQ_RP_RejoindreRP(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;
   if(!theApp.m_dwRPSystem)
      return;

   if(!user->self)
      return;

   long lID;
   GET_LONG( lID );

   // Create new player message structure
   LPASYNC_PACKET_FUNC_PARAMSEX lpParams = new ASYNC_PACKET_FUNC_PARAMSEX;
   lpParams->user        = user;
   lpParams->iParam1     = lID;

   // Call asynchronous loading function
   AsyncFuncQueue::GetMainQueue()->Call( AsyncRQFUNC_RQ_RP_RejoindreRP, lpParams );

   RQ_FOOTER( "RQFUNC_RQ_RP_RejoindreRP" );
}

void TFCMessagesHandler::RQFUNC_RQ_RP_RejoindreAnswerRP(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;
   if(!theApp.m_dwRPSystem)
      return;

   if(!user->self)
      return;

   char chAccept;
   GET_BYTE( chAccept );

   // Create new player message structure
   LPASYNC_PACKET_FUNC_PARAMSEX lpParams = new ASYNC_PACKET_FUNC_PARAMSEX;
   lpParams->user        = user;
   lpParams->iParam1     = chAccept;

   // Call asynchronous loading function
   AsyncFuncQueue::GetMainQueue()->Call( AsyncRQFUNC_RQ_RP_RejoindreAnswerRP, lpParams );

   RQ_FOOTER( "RQFUNC_RQ_RP_RejoindreAnswerRP" );
}

void TFCMessagesHandler::RQFUNC_RQ_RP_ExpulserRP(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;
   if(!theApp.m_dwRPSystem)
      return;

   if(!user->self)
      return;

   long lPLID;
   GET_LONG( lPLID );

   // Create new player message structure
   LPASYNC_PACKET_FUNC_PARAMSEX lpParams = new ASYNC_PACKET_FUNC_PARAMSEX;
   lpParams->user        = user;
   lpParams->iParam1     = lPLID;

   // Call asynchronous loading function
   AsyncFuncQueue::GetMainQueue()->Call( AsyncRQFUNC_RQ_RP_ExpulserRP, lpParams );


   RQ_FOOTER( "RQFUNC_RQ_RP_ExpulserRP" );
}

void TFCMessagesHandler::RQFUNC_RQ_RP_InviteRP(PACKET_FUNC_PROTOTYPE)
{
   CString strMsgTmp;

   RQ_HEADER;
   if(!theApp.m_dwRPSystem)
      return;

   if(!user->self)
      return;

   DWORD dwID;
   WorldPos wlPos = { 0, 0, user->self->GetWL().world };
   GET_LONG ( dwID );
   GET_WORD ( wlPos.X );
   GET_WORD ( wlPos.Y );

   if( dwID == user->self->GetID() )
   {
      //invite lui meme....
      user->self->SendInfoMessage(_STR( 7502, user->self->GetLang() ),0x0080FF);
      return;
   }

   WorldMap *wlWorld = TFCMAIN::GetWorld( user->self->GetWL().world );
   if( wlWorld != NULL )
   {
      // Try to find the unit.
      Unit *lpUnit = wlWorld->FindNearUnit( wlPos, dwID );
      if( lpUnit != NULL )
      {
         // If the unit is a PC.
         if( lpUnit->GetType() == U_PC )
         {

            // Cast its character.
            Character *lpCharacter = static_cast< Character * >( lpUnit );
            if(lpCharacter->GetNMModeRPPhaseID()>=0)
            {
               //invite lui meme....
               strMsgTmp.Format(_STR(15433, lpCharacter->GetLang()),lpCharacter->GetTrueName());
               user->self->SendInfoMessage(strMsgTmp.GetBuffer(0),0x0080FF);
               return;
            }

            // Create new player message structure
            LPASYNC_PACKET_FUNC_PARAMSEX lpParams = new ASYNC_PACKET_FUNC_PARAMSEX;
            lpParams->user        = user;
            lpParams->iParam1     = lpCharacter->GetID();

            // Call asynchronous loading function
            AsyncFuncQueue::GetMainQueue()->Call( AsyncRQFUNC_RQ_RP_InviteRP, lpParams );


            /*
            user->self->SetRPInvitedID(lpCharacter->GetID());
            TFCPacket sending;
            sending << (RQ_SIZE)RQ_RP_InviteRP;
            sending << user->self->GetTrueName();
            lpCharacter->SendPlayerMessage( sending );
            user->self->SendInfoMessage( _STR( 15069, user->self->GetLang() ) ,CL_ORANGE);

            
            strMsgTmp.Format(_STR(15429, lpCharacter->GetLang()),user->self->GetTrueName());
            lpCharacter->SendInfoMessage(strMsgTmp.GetBuffer(0) ,CL_ORANGE);
            */

            
         }
         else
         {
            // Otherwise notify player.
            user->self->SendInfoMessage( _STR( 15050, user->self->GetLang() ) ,CL_ORANGE);
         }
      }
   }
   RQ_FOOTER( "RQFUNC_RQ_RP_InviteRP" );
}

void TFCMessagesHandler::RQFUNC_RQ_RP_InviteAnswerRP(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;
   if(!theApp.m_dwRPSystem)
      return;

   if(!user->self)
      return;

   char chAccept;
   GET_BYTE( chAccept );

   // Create new player message structure
   LPASYNC_PACKET_FUNC_PARAMSEX lpParams = new ASYNC_PACKET_FUNC_PARAMSEX;
   lpParams->user        = user;
   lpParams->iParam1     = chAccept;

   // Call asynchronous loading function
   AsyncFuncQueue::GetMainQueue()->Call( AsyncRQFUNC_RQ_RP_InviteAnswerRP, lpParams );

   
   RQ_FOOTER( "RQFUNC_RQ_RP_InviteRP" );
}

void TFCMessagesHandler::RQFUNC_RQ_RP_SignalerRP(PACKET_FUNC_PROTOTYPE)
{
	RQ_HEADER;
	if(!theApp.m_dwRPSystem)
		return;

	if(!user->self)
		return;

	long lID;
	GET_LONG( lID );

	// Create new player message structure
	LPASYNC_PACKET_FUNC_PARAMSEX lpParams = new ASYNC_PACKET_FUNC_PARAMSEX;
	lpParams->user        = user;
	lpParams->iParam1     = lID;

	// Call asynchronous loading function
	AsyncFuncQueue::GetMainQueue()->Call( AsyncRQFUNC_RQ_RP_SignalerRP, lpParams );

	RQ_FOOTER( "RQFUNC_RQ_RP_SignalerRP" );
}



void TFCMessagesHandler::RQFUNC_RQ_QB_GetQuestList(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;
   //if(!theApp.m_dwRPSystem)
   //   return;

   if(!user->self)
      return;

   CString strTmp;
   TFCPacket sending;
   sending << (RQ_SIZE)RQ_QB_GetQuestList;

   int iNbrQuestT = QuestBook::GetNbrQuest();
   int iNbrQuestE = QuestBook::GetNbrQuestEnabled();

   int iNbrQuestNotDone = 0;
   //Get NBr quest Not COmpleted
   for(int i=0;i<iNbrQuestT;i++)
   {
      sQuestBook *pQuest =  QuestBook::GetQuestByIndex(i);
      if(pQuest->chEnable)
      {
         int iFlagVal = user->self->ViewFlag(pQuest->dwFlagID);
         if(iFlagVal <= pQuest->dwNbrQuestStep)
            iNbrQuestNotDone++; // quest completed
      }
   }

   sending << (long)iNbrQuestNotDone; //Send que les quete pas completer
   for(int i=0;i<iNbrQuestT;i++)
   {
      sQuestBook *pQuest =  QuestBook::GetQuestByIndex(i);
      if(pQuest->chEnable)
      {
         //Check the quest status...
         BYTE chStatus = 0;
         int iFlagVal = user->self->ViewFlag(pQuest->dwFlagID);
         if(iFlagVal > pQuest->dwNbrQuestStep)
            chStatus = 2; // quest completed
         else if(iFlagVal >0)
            chStatus = 1; // quest in progress

         if(chStatus != 2)
         {
            sending << (short)pQuest->ushUniqueID;
            sending << (short)pQuest->ushQuestLevel;
            sending << (char)chStatus;
            strTmp.Format("%s",pQuest->pStrName);
            sending << strTmp;
         }
      }
   }
   user->self->SendPlayerMessage(sending);

   RQ_FOOTER( "RQFUNC_RQ_QB_GetQuestList" );
}

void TFCMessagesHandler::RQFUNC_RQ_QB_GetQuestMsg(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;
   //if(!theApp.m_dwRPSystem)
   //   return;

   if(!user->self)
      return;

   USHORT ushID;
   GET_WORD(ushID);

   
   int iNbrQuest = QuestBook::GetNbrQuest();
   for(int i=0;i<iNbrQuest;i++)
   {
      sQuestBook *pQuest =  QuestBook::GetQuestByID(ushID);
      if(pQuest && pQuest->ushUniqueID == ushID)
      {
         CString strTmp;
         strTmp.Format("%s",pQuest->pStrMsg);

         TFCPacket sending;
         sending << (RQ_SIZE)RQ_QB_GetQuestMsg;
         sending << (short)pQuest->ushUniqueID;
         sending << strTmp;
         user->self->SendPlayerMessage(sending);

         return;
      }
   }
   RQ_FOOTER( "RQFUNC_RQ_QB_GetQuestMsg" );
}

void TFCMessagesHandler::RQFUNC_RQ_QB_GetActiveQuest(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;
   //if(!theApp.m_dwRPSystem)
   //   return;

   if(!user->self)
      return;

   int iNbrACtiveQuest = 0;
   CString strTmp;
   CString strRepTmp;

   //1:  On commence par ciompter le nombre de quest active
   int iNbrQuest = QuestBook::GetNbrQuest();
   for(int i=0;i<iNbrQuest;i++)
   {
      sQuestBook *pQuest =  QuestBook::GetQuestByIndex(i);
      int iFlagVal = user->self->ViewFlag(pQuest->dwFlagID);
      if(iFlagVal > 0 && iFlagVal <= pQuest->dwNbrQuestStep)
         iNbrACtiveQuest++;
   }

   TFCPacket sending;
   sending << (RQ_SIZE)RQ_QB_GetActiveQuest;
   sending << (long)iNbrACtiveQuest;

   int iSendQuest = 0;
   for(int i=0;i<iNbrQuest;i++)
   {
      sQuestBook *pQuest =  QuestBook::GetQuestByIndex(i);
      int iFlagVal = user->self->ViewFlag(pQuest->dwFlagID);
      if(iFlagVal > 0 && iFlagVal <= pQuest->dwNbrQuestStep)
      {
         sending << (short)pQuest->ushUniqueID;
         sending << (short)pQuest->ushQuestLevel;
         sending << (char) iFlagVal;

         
         strTmp.Format("%s",pQuest->pStrName);
         sending << strTmp;

         BYTE chNbrStep = iFlagVal; //pQuest->dwNbrQuestStep;

         if(chNbrStep > pQuest->dwNbrQuestStep)
            chNbrStep = pQuest->dwNbrQuestStep;

         sending << (char)chNbrStep;

         for(int s=0;s<chNbrStep;s++)
         {
            strTmp.Format("%s",pQuest->pStepMsg[s].pstrStep);

            CStringArray aFlagKW;
            CStringArray aItemCKW;
            CStringArray aFlagKWRep;
            CStringArray aItemCKWRep;
            ExtractKeyworld(strTmp.GetBuffer(0),"$flag(",")",aFlagKW);
            ExtractKeyworld(strTmp.GetBuffer(0),"$itemcount(",")",aItemCKW);

            //FLAG
            for(int i=0;i<aFlagKW.GetCount();i++)
            {
               int iFlagID = atoi(aFlagKW[i].GetBuffer(0)+6);
               strRepTmp.Format("%d",user->self->ViewFlag(iFlagID));
               strTmp.Replace(aFlagKW[i],strRepTmp);
            }

            //ITEMCOUNT
            for(int i=0;i<aItemCKW.GetCount();i++)
            {
               int iItemID = atoi(aItemCKW[i].GetBuffer(0)+11);
               strRepTmp.Format("%d",user->self->BackCount(iItemID));
               strTmp.Replace(aItemCKW[i],strRepTmp);
            }
            sending << strTmp;
         }

         iSendQuest++;
         if(iSendQuest == iNbrACtiveQuest)
         {
            i = iNbrQuest;
         }
      }
   }

   if(iSendQuest < iNbrACtiveQuest)
   {
      int iOff = iNbrACtiveQuest - iSendQuest;
      //Send dummy quest quest stat change between count and send...
      for(int i=0;i<iOff;i++)
      {
         sending << (short)0;
         sending << (short)0;
         strTmp.Format("%s","---");
         sending << strTmp;
         sending << (char)0;
      }
   }

   user->self->SendPlayerMessage(sending);

   RQ_FOOTER( "RQFUNC_RQ_QB_GetActiveQuest" );
}


void TFCMessagesHandler::RQFUNC_RQ_QB_GetQuestListComplete(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;
   //if(!theApp.m_dwRPSystem)
   //   return;

   if(!user->self)
      return;

   CString strTmp;
   TFCPacket sending;
   sending << (RQ_SIZE)RQ_QB_GetQuestListComplete;

   int iNbrQuestT = QuestBook::GetNbrQuest();

   int iNbrQuestDone = 0;
   //Get NBr quest Not COmpleted
   for(int i=0;i<iNbrQuestT;i++)
   {
      sQuestBook *pQuest =  QuestBook::GetQuestByIndex(i);
      if(pQuest->chEnable)
      {
         int iFlagVal = user->self->ViewFlag(pQuest->dwFlagID);
         if(iFlagVal > pQuest->dwNbrQuestStep)
            iNbrQuestDone++; // quest completed
      }
   }

   sending << (long)iNbrQuestDone; //Send que les quete pas completer
   for(int i=0;i<iNbrQuestT;i++)
   {
      sQuestBook *pQuest =  QuestBook::GetQuestByIndex(i);
      if(pQuest->chEnable)
      {
         //Check the quest status...
         if(user->self->ViewFlag(pQuest->dwFlagID) > pQuest->dwNbrQuestStep)
         {
            sending << (short)pQuest->ushUniqueID;
            sending << (short)pQuest->ushQuestLevel;
            strTmp.Format("%s",pQuest->pStrName);
            sending << strTmp;
         }
      }
   }
   user->self->SendPlayerMessage(sending);

   RQ_FOOTER( "RQFUNC_RQ_QB_GetQuestListComplete" );
}

void TFCMessagesHandler::RQFUNC_RQ_QB_StopQuest(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;
   //if(!theApp.m_dwRPSystem)
   //   return;

   if(!user->self)
      return;

   USHORT ushID;
   GET_WORD(ushID);


   int iNbrQuest = QuestBook::GetNbrQuest();
   for(int i=0;i<iNbrQuest;i++)
   {
      sQuestBook *pQuest =  QuestBook::GetQuestByID(ushID);
      if(pQuest && pQuest->ushUniqueID == ushID)
      {
         //Valide que le player a la quest...
         BYTE chStatus = 0;
         int iFlagVal = user->self->ViewFlag(pQuest->dwFlagID);
         if(iFlagVal > pQuest->dwNbrQuestStep)
            chStatus = 2; // quest completed
         else if(iFlagVal >0)
            chStatus = 1; // quest in progress

         TFCPacket sending;
         sending << (RQ_SIZE)RQ_QB_StopQuest;
         if(chStatus == 1) //quest en cours mais pas terminer...
         {
            //Reset le flag de la quete
            user->self->SetFlag(pQuest->dwFlagID,0);
            sending << (char)1;
         }
         else
         {
            //on ne reset pas une quete pas commencer opu deja fini
            sending << (char)0;
         }
         user->self->SendPlayerMessage(sending);
         return;
      }
   }
   RQ_FOOTER( "RQFUNC_RQ_QB_StopQuest" );
}






void TFCMessagesHandler::RQFUNC_GMMSG_Post(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;
   
   if(!theApp.m_dwGMMsgSystem)
      return;

   if(!user->self)
      return;

   CString strMessage,strMsgTmp;
   LPBYTE lpMessage = NULL;
   GET_STRING( lpMessage );
   if(strlen((char*)lpMessage) >254)
      lpMessage[254] = 0x00;
   strMessage.Format("%s",lpMessage);
   if(lpMessage)
      delete []lpMessage;
   lpMessage = NULL;

   int iRet = GMMsgMaster::PostGMMessage(user, strMessage);
   if(iRet == -1)
      strMsgTmp.Format(_STR(15195, user->self->GetLang()));
   else if(iRet == -2)
      strMsgTmp.Format(_STR(15194, user->self->GetLang()));
   else
   {
      strMsgTmp.Format(_STR(15193, user->self->GetLang()));
      CPlayerManager::SendMessagetoAllGOD(_STR(15372, user->self->GetLang()));
   }
   user->self->SendSystemMessage(strMsgTmp.GetBuffer(0));
  
   RQ_FOOTER( "RQFUNC_GMMSG_Post" );
}

void TFCMessagesHandler::RQFUNC_GMMSG_Get(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;
   if(!theApp.m_dwGMMsgSystem)
      return;

   if(!user->self)
      return;

   if(user->GetGodFlags() & GOD_CAN_EDIT_USER)
   {
      GMMsgMaster::SendAllOpenedGMMessage(user);
   }
   RQ_FOOTER( "RQFUNC_GMMSG_Get" );
}

void TFCMessagesHandler::RQFUNC_GMMSG_Close(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;
   
   if(!theApp.m_dwGMMsgSystem)
      return;

   if(!user->self)
      return;

   if(user->GetGodFlags() & GOD_CAN_EDIT_USER)
   {
      DWORD udwID;
      GET_LONG(udwID);

      if(GMMsgMaster::CloseGMMessage(udwID) != 0)
         user->self->SendSystemMessage(_STR(15373, user->self->GetLang()));
     
   }
   RQ_FOOTER( "RQFUNC_GMMSG_Close" );
}



void TFCMessagesHandler::RQFUNC_GetAllPlayerPos(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;
   if(!user->self)
      return;

   _LOG_GAMEOP
      LOG_SYSOP,
      "Receive Server Options GetAllPosition %s  [%s]",user->self->GetTrueName(),user->GetAccount()
      LOG_

   if(user->GetGodFlags() & GOD_CAN_TELEPORT)
   {
      CPlayerManager::PacketUserPos( user );
   }
   RQ_FOOTER( "RQFUNC_GetAllPlayerPos" );
}


void TFCMessagesHandler::RQFUNC_SvrNPC(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;
   if(!user->self)
      return;

   _LOG_GAMEOP
      LOG_SYSOP,
      "Receive Server NPC List %s  [%s]",user->self->GetTrueName(),user->GetAccount()
   LOG_

   if(user->GetGodFlags() & GOD_CAN_SUMMON_MONSTERS)
   {
      int iNbrPerPacket = 200;
      int iNbrPart = theApp.m_aServerNPCList.GetSize()/iNbrPerPacket;
      int iRest = theApp.m_aServerNPCList.GetSize()%iNbrPerPacket;
      if(iRest)
         iNbrPart++;

      int partCnt = 0;
      int iNbr = iNbrPerPacket;
      int iIndex = 0;
      for(int i=0;i<iNbrPart;i++)
      {
         TFCPacket sending;
         sending << (RQ_SIZE)RQ_SvrNPC;
         sending << (long)i;
         sending << (long)iNbrPart;
         if(i < iNbrPart-1)
            sending << (long)iNbr;
         else
         {
            iNbr = iRest;
            sending << (long)iRest;
         }
         sending << (long)iNbrPerPacket;

         for(int i=0;i<iNbr;i++)
         {
            sending << (short)theApp.m_aServerNPCList[iIndex].ushID;
            sending << theApp.m_aServerNPCList[iIndex].SummonName;
            iIndex++;
         }
         user->self->SendPlayerMessage(sending);
      }
   }

   RQ_FOOTER( "RQFUNC_SvrNPC" );
}


void TFCMessagesHandler::RQFUNC_SvrSpellList(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   if(!user->self)
      return;

   _LOG_GAMEOP
      LOG_SYSOP,
      "Receive Server Spell List %s  [%s]",user->self->GetTrueName(),user->GetAccount()
   LOG_

   if(user->GetGodFlags() & GOD_CAN_EDIT_USER_SPELLS)
   {
      int iNbrPerPacket = 200;
      int iNbrPart = theApp.m_aServerSpellsList.GetSize()/iNbrPerPacket;
      int iRest = theApp.m_aServerSpellsList.GetSize()%iNbrPerPacket;
      if(iRest)
         iNbrPart++;

      int partCnt = 0;
      int iNbr = iNbrPerPacket;
      int iIndex = 0;
      for(int i=0;i<iNbrPart;i++)
      {
         TFCPacket sending;
         sending << (RQ_SIZE)RQ_SvrSpellList;
         sending << (long)i;
         sending << (long)iNbrPart;
         if(i < iNbrPart-1)
            sending << (long)iNbr;
         else
         {
            iNbr = iRest;
            sending << (long)iRest;
         }
         sending << (long)iNbrPerPacket;

         for(int i=0;i<iNbr;i++)
         {
            sending << (short)theApp.m_aServerSpellsList[iIndex].ushID;
            sending << theApp.m_aServerSpellsList[iIndex].SummonName;
            iIndex++;
         }
         user->self->SendPlayerMessage(sending);
      }
   }

   RQ_FOOTER( "RQFUNC_SvrSpellList" );
}

void TFCMessagesHandler::RQFUNC_SvrMonsterList(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   if(!user->self)
      return;

   _LOG_GAMEOP
      LOG_SYSOP,
      "Receive Server Monsters List %s  [%s]",user->self->GetTrueName(),user->GetAccount()
   LOG_

   if(user->GetGodFlags() & GOD_CAN_SUMMON_MONSTERS)
   {
      int iNbrPerPacket = 200;
      int iNbrPart = theApp.m_aServerMonstersList.GetSize()/iNbrPerPacket;
      int iRest = theApp.m_aServerMonstersList.GetSize()%iNbrPerPacket;
      if(iRest)
         iNbrPart++;

      int partCnt = 0;
      int iNbr = iNbrPerPacket;
      int iIndex = 0;
      for(int i=0;i<iNbrPart;i++)
      {
         TFCPacket sending;
         sending << (RQ_SIZE)RQ_SvrMonsterList;
         sending << (long)i;
         sending << (long)iNbrPart;
         if(i < iNbrPart-1)
            sending << (long)iNbr;
         else
         {
            iNbr = iRest;
            sending << (long)iRest;
         }
         sending << (long)iNbrPerPacket;

         for(int i=0;i<iNbr;i++)
         {
            sending << (short)theApp.m_aServerMonstersList[iIndex].ushID;
            sending << theApp.m_aServerMonstersList[iIndex].SummonName;
            iIndex++;
         }
         user->self->SendPlayerMessage(sending);
      }
   }

   RQ_FOOTER( "RQFUNC_SvrMonsterList" );
}

void TFCMessagesHandler::RQFUNC_SvrItemsList(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   if(!user->self)
      return;

   _LOG_GAMEOP
      LOG_SYSOP,
      "Receive Server Items List %s  [%s]",user->self->GetTrueName(),user->GetAccount()
   LOG_

   if(user->GetGodFlags() & GOD_CAN_SUMMON_ITEMS)
   {

      int iNbrPerPacket = 200;

      int iNbrPart = theApp.m_aServerItemsList.GetSize()/iNbrPerPacket;
      int iRest = theApp.m_aServerItemsList.GetSize()%iNbrPerPacket;
      if(iRest)
         iNbrPart++;

      int partCnt = 0;
      int iNbr = iNbrPerPacket;
      int iIndex = 0;
      for(int i=0;i<iNbrPart;i++)
      {
         TFCPacket sending;
         sending << (RQ_SIZE)RQ_SvrItemsList;
         sending << (long)i;
         sending << (long)iNbrPart;
         if(i < iNbrPart-1)
            sending << (long)iNbr;
         else
         {
            iNbr = iRest;
            sending << (long)iRest;
         }
         sending << (long)iNbrPerPacket;

         for(int i=0;i<iNbr;i++)
         {
            sending << (short)theApp.m_aServerItemsList[iIndex].ushID;
            sending << theApp.m_aServerItemsList[iIndex].SummonName;
            iIndex++;
         }
         user->self->SendPlayerMessage(sending);
      }
   }

   RQ_FOOTER( "RQFUNC_SvrItemsList" );
}



void TFCMessagesHandler::RQFUNC_SvrMonsterSkin(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   if(!user->self)
      return;

   _LOG_GAMEOP
      LOG_SYSOP,
      "Receive Server Monster skin List %s  [%s]",user->self->GetTrueName(),user->GetAccount()
   LOG_
   if(user->GetGodFlags() & GOD_CAN_EMULATE_MONSTER)
   {
      int iNbrPerPacket = 200;

      int iNbrPart = theApp.m_aServerMonsterSkinList.GetSize()/iNbrPerPacket;
      int iRest = theApp.m_aServerMonsterSkinList.GetSize()%iNbrPerPacket;
      if(iRest)
         iNbrPart++;

      int partCnt = 0;
      int iNbr = iNbrPerPacket;
      int iIndex = 0;
      for(int i=0;i<iNbrPart;i++)
      {
         TFCPacket sending;
         sending << (RQ_SIZE)RQ_SvrMonsterSkin;
         sending << (long)i;
         sending << (long)iNbrPart;
         if(i < iNbrPart-1)
            sending << (long)iNbr;
         else
         {
            iNbr = iRest;
            sending << (long)iRest;
         }
         sending << (long)iNbrPerPacket;

         for(int i=0;i<iNbr;i++)
         {
            sending << (short)theApp.m_aServerMonsterSkinList[iIndex].ushSkinID;
            sending << theApp.m_aServerMonsterSkinList[iIndex].SkinName;
            iIndex++;
         }
         user->self->SendPlayerMessage(sending);
      }
   }

   RQ_FOOTER( "RQFUNC_SvrMonsterSkin" );
}

void TFCMessagesHandler::RQFUNC_SvrQuestFlagList(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   if(!user->self)
      return;

   _LOG_GAMEOP
      LOG_SYSOP,
      "Receive Server flag Quest List %s  [%s]",user->self->GetTrueName(),user->GetAccount()
   LOG_

   if(user->GetGodFlags() & GOD_CAN_EDIT_USER)
   {
      int iNbrPerPacket = 200;
      int iNbrPart = theApp.m_aServerQuestFlagList.GetSize()/iNbrPerPacket;
      int iRest = theApp.m_aServerQuestFlagList.GetSize()%iNbrPerPacket;
      if(iRest)
         iNbrPart++;

      int partCnt = 0;
      int iNbr = iNbrPerPacket;
      int iIndex = 0;
      for(int i=0;i<iNbrPart;i++)
      {
         TFCPacket sending;
         sending << (RQ_SIZE)RQ_SvrQuestFlagList;
         sending << (long)i;
         sending << (long)iNbrPart;
         if(i < iNbrPart-1)
            sending << (long)iNbr;
         else
         {
            iNbr = iRest;
            sending << (long)iRest;
         }
         sending << (long)iNbrPerPacket;


         for(int i=0;i<iNbr;i++)
         {
            sending << (short)theApp.m_aServerQuestFlagList[iIndex].ushFlagID;
            sending << theApp.m_aServerQuestFlagList[iIndex].FlagName;
            iIndex++;
         }
         user->self->SendPlayerMessage(sending);
      }
   }
   RQ_FOOTER( "RQFUNC_SvrQuestFlagList" );
}

void TFCMessagesHandler::RQFUNC_NM_DeathResurect(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   if(!user->self)
      return;

   user->self->NMResurect(FALSE);

   RQ_FOOTER( "RQ_NM_DeathResurect" );
}


void TFCMessagesHandler::RQFUNC_NM_GetProfession(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   if(!user->self)
      return;

   user->self->NMGetProfession();

   RQ_FOOTER( "RQFUNC_NM_GetProfession" );
}

void TFCMessagesHandler::RQFUNC_NM_SendMakeFormule(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;
   WORD ushID;
   GET_WORD(ushID);

   if(!user->self)
      return;

   user->self->NMMakeFormule(ushID);

   RQ_FOOTER( "RQFUNC_NM_SendMakeFormule" );
}






/******************************************************************************/
// Returns the max users according to an IP address.
DWORD TFCMessagesHandler::GetUserMax(
 sockaddr_in sockAddr // The IP address.
)
/******************************************************************************/
{
    BOOL boLocal = FALSE;
    DWORD dwMax = 65536;
    
    tlLocalUsers.Lock();
    // Determine if this address is part of the local addresses.        

    tlLocalUsers.ToHead();
    while( tlLocalUsers.QueryNext() && !boLocal )
	{
        LPLOCAL_USER lpLocal = tlLocalUsers.Object();

        // If the IP & netmask match
        TRACE( "\r\nIP %u vs %u, netmasked %u vs %u.", 
            sockAddr.sin_addr.S_un.S_addr,
            lpLocal->dwIP,
            sockAddr.sin_addr.S_un.S_addr & lpLocal->dwNetmask,
            lpLocal->dwIP & lpLocal->dwNetmask
        );
        if( ( sockAddr.sin_addr.S_un.S_addr & lpLocal->dwNetmask ) == ( lpLocal->dwIP & lpLocal->dwNetmask ) )
		{
            boLocal = TRUE;
        }
    }
    
    if( boLocal )
	{
        dwMax = dwMaxLocalUsers;
    }
	else
	{
        dwMax = dwMaxRemoteUsers;
    }

    tlLocalUsers.Unlock();

    return dwMax;
}
/******************************************************************************/
//  Determines of an IP is local or remote.
bool TFCMessagesHandler::IsLocalIP( sockaddr_in sockAddr ) // The IP.
/******************************************************************************/
{
    bool boLocal = false;
    tlLocalUsers.Lock();
    
    // Determine if this address is part of the local addresses.
    tlLocalUsers.ToHead();
    while( tlLocalUsers.QueryNext() && !boLocal )
	{
        LPLOCAL_USER lpLocal = tlLocalUsers.Object();

        if( ( sockAddr.sin_addr.S_un.S_addr & lpLocal->dwNetmask ) == ( lpLocal->dwIP & lpLocal->dwNetmask ) )
		{
            boLocal = true;
        }
    }
    tlLocalUsers.Unlock();

    return boLocal;
}

















































































































































void TFCMessagesHandler::RQFUNC_PlayerMove( PACKET_FUNC_PROTOTYPE)
/******************************************************************************/
{
   RQ_HEADER;
   if(user && user->self && user->in_game)
   {
      if( user->self->ViewFlag( __FLAG_BERSERK ) == 0 )
      {
         const int MoveExhaust  = 200 MILLISECONDS;
         const int MoveExhaustX = 100 MILLISECONDS;

         unsigned long ulLastMove = user->self->GetLastMoveTime();
         if(rqRequestID >210 && rqRequestID <219)
         {
            if(TFCMAIN::GetRound() < ulLastMove+MoveExhaustX)
               return; // on processe rien trop vite ce pack de move...
         }
         
         user->self->SetLastMoveTime();

         short shDirection = rqRequestID;
         if(rqRequestID >210 && rqRequestID <219)
            shDirection -=210;


         EXHAUST newExhaust =  user->self->GetExhaust();
 
         // If user isn't move exhaust
         // Or if user sends an advanced move exhaust.
         user->self->StopAutoCombat();
         if( newExhaust.move <= TFCMAIN::GetRound() || ( newExhaust.boWalking && newExhaust.move - TFCMAIN::GetRound() <= MoveExhaust ))
         {
            user->self->StartMove(shDirection);
            
         }
         else
         {
            // If player is more than 2 seconds exhaust.
            if( newExhaust.move >= TFCMAIN::GetRound() + 2 SECONDS )
            {
               // Send a system message telling the player that he's exhaust.
               // This might flood a player with these messages if he keeps his finger on the move button.
                user->self->SendSystemMessage( _STR( 2776, user->self->GetLang() ) );
            }                
         }
      }
   }
   RQ_FOOTER( "RQ_MovePlayer" );
}

void TFCMessagesHandler::RQFUNC_QueryPlayerPos( PACKET_FUNC_PROTOTYPE )
{
   RQ_HEADER;
   if(user && user->self)
   {
      TFCPacket sending;
      WorldPos wlPos = user->self->GetWL();
      sending << (RQ_SIZE)RQ_GetPlayerPos; // GetPlayerPos request
      sending << (short)wlPos.X;
      sending << (short)wlPos.Y;
      sending << (short)wlPos.world;					
      // we send the packet in the player's box	
      user->self->SendPlayerMessage( sending );
   }
   RQ_FOOTER( "RQ_GetPlayerPos" );
}







/******************************************************************************/
// From out of game to PreInGame.
void TFCMessagesHandler::RQFUNC_PutPlayerInGame(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   if( user->boLockedOut  || !user->CanPaidPlay() || !user->CanConnectGMOnly())
      return;

   // if player isn't in game (for a change..!!! :))
   if(!user->in_game && !user->boPreInGame && user->registred)
   {
      if( user->UsePicklock(__FILE__, __LINE__) )
      {
         try
         {
            unsigned char temp_length;
            unsigned char temp_byte;
            unsigned char dummy;
            long lKey = 0;

            CString name;
            name.Empty();

            msg->Get((char *)(&temp_length));		

            // Retrieves the name
            for(dummy = 0; dummy < temp_length; dummy++)
            {
               msg->Get((char *)(&temp_byte));
               name += (TCHAR)temp_byte;
            }

            msg->Get((long  *)(&lKey));

            if(user->GetKeyCode() == 0 || user->GetKeyCode() != lKey)
            {
               _LOG_CHEAT
                  LOG_MISC_1,
                  "Player %s Account %s (IP:%s) Invalid Validation Key",
                  name,
                  (LPCTSTR)user->GetAccount(),
                  user->GetIP()
                  LOG_

                  user->dwKickoutTime = 10 SECONDS TDELAY;
            }

            // Create new player message structure
            LPASYNC_PACKET_FUNC_PARAMS lpParams = new ASYNC_PACKET_FUNC_PARAMS;
            lpParams->user        = user;
            lpParams->rqRequestID = rqRequestID;
            lpParams->msg         = NULL;
            lpParams->strParam    = name;

            // Call asynchronous loading function
            AsyncFuncQueue::GetMainQueue()->Call( AsyncRQFUNC_PutPlayerInGame, lpParams );


         }
         catch(...)
         {
            LOG_PACKET_ERROR( "RQ_PutPlayerInGame" );
         }

      }
   }
   else
   {
      if( user->boPreInGame )// If user is between states, then and then only send the stats.
         user->self->StartPutPlayerInGame();
   }
   RQ_FOOTER( "RQ_PutPlayerInGame" );
}

// Message handler of packet type RQFUNC_RegisterAccount
void TFCMessagesHandler::RQFUNC_RegisterAccount(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   char *lpcStr = NULL;

   if(CPlayerManager::IsPlayerResourceExist(sockAddrO))
   {
      _LOG_DEBUG
         LOG_DEBUG_LVL3,
         "Player was already authenticated. Resending authentication agreement."
         LOG_

      return;
   }

   // Register account
   // If we're not already registred and if we can read the message
   unsigned char temp_length1;
   unsigned char temp_length2;
   unsigned short hi_version;
   int dummy;		

   CString account;
   CString password;
   CString full_directory;

   msg->Get((char *)(&temp_length1));		
   if( temp_length1 )
   {
      lpcStr = new char[ temp_length1 + 1 ];
      // Retrieves the name
      for(dummy = 0; dummy < temp_length1; dummy++)
         msg->Get( (char *)( &lpcStr[ dummy ] ) );
      lpcStr[ dummy ] = 0;

      account = lpcStr;
      if (lpcStr != NULL)
      {
         delete lpcStr;
         lpcStr = NULL;
      }
   }
   else
   {
      account = "";
   }
   // Remove beginning and trailing white spaces.
   account.TrimRight();
   account.TrimLeft();

   msg->Get((char *)(&temp_length2));	
   if( temp_length2 )
   {
      lpcStr = new char[ temp_length2 + 1 ];
      for(dummy = 0; dummy < temp_length2; dummy++)
         msg->Get((char *)( &lpcStr[ dummy ] ));	
      lpcStr[ dummy ] = 0;	// Null term string.
      password = lpcStr;
      if (lpcStr != NULL)
      {
         delete lpcStr;
         lpcStr = NULL;
      }
   }
   else
   {
      password = "";
   }
   msg->Get((short *)(&hi_version));

   if( !account.IsEmpty() )
   {
      _LOG_DEBUG
         LOG_DEBUG_HIGH,
            "Sending account %s for authentication.",
            (LPCTSTR)account
         LOG_

      // Create and fill structure to pass to async registering function.
      LPRQSTRUCT_REGISTER_ACCOUNT lpStruct = new RQSTRUCT_REGISTER_ACCOUNT;

      lpStruct->sParams.msg  = NULL;
      lpStruct->sParams.user = user;
      lpStruct->sParams.rqRequestID = rqRequestID;

      lpStruct->csAccount = account;
      lpStruct->csPassword = password;
      lpStruct->sockAddrO = sockAddrO;
      lpStruct->sockAddrI = sockAddrI;
      msg->Get((short *)(&lpStruct->usLangUsed)); // Ajout du système multilingue.
      lpStruct->packetSeedID = msg->GetPacketSeedID();

      // Call asynchronous registering function for ODBC authentification.
      AsyncFuncQueue::GetMainQueue()->Call( AsyncRQFUNC_RegisterAccountODBC, lpStruct );
   }

   RQ_FOOTER( "RQ_RegisterAccount" );
}

void TFCMessagesHandler::RQFUNC_QueryNameExistence( PACKET_FUNC_PROTOTYPE )
{
   RQ_HEADER;

   LPBYTE name;
   GET_STRING( name );

   RQSTRUCT_QueryNameExistence *rq = new RQSTRUCT_QueryNameExistence;
   rq->name     = reinterpret_cast< const char * >( name );
   rq->sockAddrO = sockAddrO;        
   rq->sockAddrI = sockAddrI;        
   FormatPlayerName::Format( &rq->name );
   AsyncFuncQueue::GetMainQueue()->Call( AsyncRQFUNC_QueryNameExistence, rq );

   if (name != NULL)
      delete name;
   name = NULL;
   RQ_FOOTER( "RQ_QueryNameExistence" );
}

// Message handler of packet type RQFUNC_DeletePlayer
void TFCMessagesHandler::RQFUNC_DeletePlayer(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   TFCPacket sending;

   if( user->UsePicklock(__FILE__, __LINE__) )
   {
      // Delete player	
      if(user->registred && !user->in_game) // deletes only if you are registred
      {
         //CString name;
         unsigned char temp_length;
         //	unsigned char temp_byte;
         LPBYTE lpbName = NULL;

         int dummy;

         // If user was previously in the reroll menu.
         if( user->boRerolling)
         {
            // Not rerolling anymore.
            user->boRerolling = FALSE;
            // Save rerolled data.
            user->self->SaveCharacter(FALSE,"RQ_DeletePlayer"); //On Delete Player in REROLL menu... (not in game)
         }

         try
         {
            msg->Get((char *)(&temp_length));		
            lpbName = new BYTE[ temp_length + 1 ];
            // Retrieves the name
            for(dummy = 0; dummy < temp_length; dummy++)
               GET_BYTE( lpbName[ dummy ] );
            lpbName[ temp_length ] = 0;

            // Prepare async deletion
            LPASYNC_PACKET_FUNC_PARAMS lpStruct = new ASYNC_PACKET_FUNC_PARAMS;			
            lpStruct->msg         = NULL;
            lpStruct->user        = user;
            lpStruct->rqRequestID = rqRequestID;
            lpStruct->strParam    = lpbName;

            Players *user = lpStruct->user;
            //on delete de la liste ce player meme si il sera detruit de la bd plus tard...
            user->RemoveAPlList(lpbName);

            if (lpbName != NULL)
            {
               delete lpbName;
               lpbName = NULL;
            }

            AsyncFuncQueue::GetMainQueue()->Call( AsyncRQFUNC_DeletePlayer, lpStruct );

         }
         catch(...)
         {
            user->UseUnlock(__FILE__, __LINE__);
            if (lpbName != NULL)
            {
               delete lpbName;
               lpbName = NULL;
            }
            LOG_PACKET_ERROR( "RQ_DeletePlayer" );
         }
      }
      else
      {
         user->UseUnlock(__FILE__, __LINE__);
      }
   }

   RQ_FOOTER( "RQ_DeletePlayer" );
}



// Message handler of packet type RQ_GetObject
void TFCMessagesHandler::RQFUNC_GetObject(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;
   if(user->self->ViewFlag( __FLAG_NMS_PLAYER_DEATH ) != 0)
      return;

   if(user->in_game) // If we can process this message..
   {
      DWORD dwID = 0;
      WorldPos wlPos = { 0, 0, 0 };


      GET_WORD( wlPos.X );				
      GET_WORD( wlPos.Y );
      GET_LONG( dwID );

      user->self->StartGetObject(wlPos,dwID);
   }
   RQ_FOOTER( "RQ_GetObject" );
}

// Message handler of packet type RQ_FUNC
void TFCMessagesHandler::RQFUNC_DepositObject(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   if( user->in_game )
   {
      WorldMap *wl = TFCMAIN::GetWorld( user->self->GetWL().world );
      if(wl)
      {
         WorldPos where = {0,0,0};		
         DWORD itemId;
         DWORD qty;

         GET_WORD( where.X );
         GET_WORD( where.Y );
         GET_LONG( itemId );
         GET_LONG( qty );
         where.world = user->self->GetWL().world;
         Unit *obj = user->self->DropUnit( where, itemId, qty );

         if( obj != NULL )
            obj->BroadcastPopup( where );
      }
   }
   RQ_FOOTER( "RQ_DepositObject" );
}

// Message handler of packet type RQFUNC_
void TFCMessagesHandler::RQFUNC_ExitGame(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   _LOG_DEBUG
      LOG_DEBUG_HIGH,
         "Flagging delete true for %s (RQ_20).",
         (LPCTSTR)user->GetFullAccountName()
      LOG_

   if(user->in_game)
   {
      if(theApp.dwAntiplugSystem == 1)
      {
         bool bSafe = false;
         if(user->self->GetUnderBlockMap()  == __SAFE_HAVEN   || user->self->GetUnderBlockMap()  == __INDOOR_SAFE_HAVEN || user->IsGod())
            bSafe = true;

         if(bSafe)
            user->dwExitDecompte = 0;//*****//Quitte maintenant
         else
         {
            if(user->dwExitDecompte == 0xFFFF)//*****//check if another exit altready in progress
            {
               //Set the decompt Timout
               user->dwExitDecompte = theApp.dwAntiplugSystemSec;//*****//
               CString csText = _STR( 15234, user->self->GetLang() );
               user->self->SendSystemMessage(csText);
            }
         }
      }
      else
         user->dwExitDecompte = 0;//*****//maintenant
   }
   else
      user->SetDeletePlayerFlags();
   RQ_FOOTER( "RQ_ExitGame" );
}



void TFCMessagesHandler::RQFUNC_UseObject(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   WorldPos WL = {0,0,0};
   unsigned long itemID;

   WorldPos ppos = user->self->GetWL();
   WorldMap *world = TFCMAIN::GetWorld(ppos.world);
   if(world)
   {
      msg->Get((short *)(&WL.X));
      msg->Get((short *)(&WL.Y));
      msg->Get((long  *)(&itemID));

      if(WL.X != 0 && WL.Y != 0)
      { 
         WorldPos wlUserPos = user->self->GetWL();// check on the floor
         int nTouchRange = user->self->ViewFlag( __FLAG_ARM_EXTENT );// If door in usage range.

         if( nTouchRange == 0 )
            nTouchRange = _DEFAULT_TOUCH_RANGE;

         Unit *item = world->FindNearUnit( WL, itemID );
         if( item != NULL )
         {
            DWORD useRange = item->ViewFlag( __FLAG_USE_RANGE ) + nTouchRange;

            if( abs( WL.X - wlUserPos.X ) <= useRange && abs( WL.Y - wlUserPos.Y ) <= useRange )
            {
               bool bCanExecute = true;
               for(int i=0;i<theApp.m_aDelayItems.GetSize();i++)
               {
                  if(item->GetStaticReference() == theApp.m_aDelayItems[i].uiID)
                  {
                     time_t tCurTime  =  time(NULL);
                     time_t tLastTime =  user->self->ViewFlag(theApp.m_aDelayItems[i].uiFlag);
                     if(tCurTime - tLastTime > theApp.m_aDelayItems[i].uiDelay)
                     {
                        user->self->SetFlag(theApp.m_aDelayItems[i].uiFlag,tCurTime);
                        i = theApp.m_aDelayItems.GetSize();
                     }
                     else
                     {
                        CString strMessage;
                        strMessage.Format(_STR( 15371, user->self->GetLang() ));
                        user->self->SendSystemMessage(strMessage);
                        bCanExecute = false;
                        i = theApp.m_aDelayItems.GetSize();
                     }
                  }
               }
               if(bCanExecute)
               {
                  DWORD itemUsed = 0;
                  item->SendUnitMessage( MSG_OnDisturbed, item, user->self, user->self );
                  item->SendUnitMessage( MSG_OnUse, item, user->self, user->self, NULL, &itemUsed );
               }
            }
            else
               user->self->SendSystemMessage( _STR( 469, user->self->GetLang() ) );
         }
         else
            user->self->SendSystemMessage( _STR( 469, user->self->GetLang() ) );
      }
      else
         user->self->use_item(itemID, user->self);// check in backpack
   }
   RQ_FOOTER( "RQ_UseObject" );
}

void TFCMessagesHandler::RQFUNC_UseObject2(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   unsigned long itemID;
   DWORD dwTargetID;
   BYTE chAttack = 0;
   BYTE chPVP    = 0;
   WorldPos TargetPos = { 0, 0, user->self->GetWL().world };

   WorldPos ppos = user->self->GetWL();
   WorldMap *wl = TFCMAIN::GetWorld( ppos.world );
   if(wl)
   {
      msg->Get((short *)(&TargetPos.X));
      msg->Get((short *)(&TargetPos.Y));
      msg->Get((long  *)(&itemID));
      msg->Get((long *)(&dwTargetID));
      msg->Get((byte *)(&chAttack));
      msg->Get((byte *)(&chPVP));

      if(dwTargetID == user->self->GetID() && chAttack == 0 && chPVP==0)
      {
         user->self->use_item(itemID, user->self);
      }
      else
      {
         Unit *lpuUnit = wl->FindNearUnit( TargetPos, dwTargetID );
         if(lpuUnit)
         {
            UINT uiRetCanAttackPVP = 0;
            //si PVP on valid les regle de PVP en mode combat pour le PVP management
            if(chPVP)
               uiRetCanAttackPVP = GAME_RULES::NMPVPCanAttack( user->self, lpuUnit );

            if( uiRetCanAttackPVP >0 )
            {
               if(uiRetCanAttackPVP == 1)
                  user->self->SendInfoMessage(_STR( 15037 , user->self->GetLang() ),0x0570D5);
               else if(uiRetCanAttackPVP == 2)
                  user->self->SendInfoMessage(_STR( 15038 , user->self->GetLang() ),0x0570D5);
               else if(uiRetCanAttackPVP == 3)
                  user->self->SendInfoMessage(_STR( 15039 , user->self->GetLang() ),0x0570D5);
               else if(uiRetCanAttackPVP == 98)
                  user->self->SendInfoMessage(_STR( 15040 , user->self->GetLang() ),0x0570D5);
               else if(uiRetCanAttackPVP == 99)
                  user->self->SendInfoMessage(_STR( 15345 , user->self->GetLang() ),0x0570D5);
               else if(uiRetCanAttackPVP == 1000)
                  user->self->SendInfoMessage(_STR( 15511 , user->self->GetLang() ),0x0570D5);
               else 
                  user->self->SendInfoMessage(_STR( 15041 , user->self->GetLang() ),0x0570D5);
            }
            else
            {
               BOOL bINPVPBlock = TRUE;
               //Si un PVP on valid si les regle PVP serveur sont respecter
               if(chPVP || chAttack)
                  bINPVPBlock = GAME_RULES::InPVP( user->self,lpuUnit );
               if(!bINPVPBlock)
               {
                  user->self->SendSystemMessage(_STR( 463 , user->self->GetLang() ));
               }
               else
               {
                  BOOL bINSAFEBlock = FALSE;
                  if(chAttack)
                     bINSAFEBlock = GAME_RULES::InSafeHaven(user->self,lpuUnit);
                  if(bINSAFEBlock)
                  {
                     user->self->SendSystemMessage(_STR( 463 , user->self->GetLang() ));
                  }
                  else
                  {
                     bool bOK = true;

                     //si c un spell attack on valid que les mode de combat sont bien activer...
                     if(chAttack)
                     {
                        Character *chSelf = static_cast< Character * >( user->self );
                        if(chSelf)
                        {
                           if(user->self->GetType() == U_PC && lpuUnit->GetType() == U_PC && !chSelf->GetNMCombatMode() && !theApp.IsTargetOnList(user->self->GetID(),lpuUnit->GetID()))
                           {
                              // cannot send spell PVP if combat mode not activated...
                              if(user->self == lpuUnit)
                                 user->self->SendSystemMessage( _STR( 15047, user->self->GetLang() ) );
                              else
                                 user->self->SendSystemMessage( _STR( 15036, user->self->GetLang() ) );
                              bOK = false;
                           }
                        }
                     }

                     if(bOK)
                     {

                        WorldPos tempPos;
                        WorldMap *wlWorld = TFCMAIN::GetWorld( user->self->GetWL().world );
                        Unit *lpCollisionUnit = NULL;
                        if (!wlWorld->GetCollisionPos( user->self->GetWL(), lpuUnit->GetWL(), &tempPos, &lpCollisionUnit, false, false )) 
                        {
                           user->self->use_item2(itemID, lpuUnit);
                        }
                        else
                        {
                           user->self->SendInfoMessage(_STR( 15363 , user->self->GetLang() ),0x0570D5);
                        }
                     }
                  }
               }
            }
         } 
      }
   }
   RQ_FOOTER( "RQ_UseObject2" );
}

void TFCMessagesHandler::RQFUNC_Attack(PACKET_FUNC_PROTOTYPE )
{
   RQ_HEADER;

   // Attack something
   WorldPos where = {0,0,0};	
   WorldPos userpos = {0,0,0};	
   Unit *target;
   BOOL done = FALSE;
   DWORD theID;    

   WorldPos ppos = user->self->GetWL();
   WorldMap *world = TFCMAIN::GetWorld(ppos.world);
   if(world)
   {
      msg->Get((short *)(&where.X));
      msg->Get((short *)(&where.Y));
      msg->Get((long *)(&theID));

      user->self->RemoveFlag( __FLAG_ROBBING );
      if(user->self->GetArenaID() >0 && user->self->GetArenaTeam() >0)
      {
         if(user->self->GetArenaType()  == ARENE1_TYPE)
         {
            Arena1Master::RemTakeList(user,user->self->GetArenaID()-1);
         }
         else if(user->self->GetArenaType()  == ARENE2_TYPE)
         {
            Arena2Master::RemTakeList(0,user,user->self->GetArenaID()-1);
            Arena2Master::RemTakeList(1,user,user->self->GetArenaID()-1);
         }
      }

      // if the ID is 0 then player requests a blind-attack 
      if(theID == 0)
      {
         WorldPos nullWl = { 0, 0, 0 };
         Broadcast::BCMiss( user->self->GetWL(), _DEFAULT_RANGE,user->self->GetID(),0,user->self->GetWL(),nullWl);
      }
      else
      {
         user->self->Do(fighting);

         // then gets who we attack;
         userpos = user->self->GetWL();
         target = world->FindNearUnit(where, theID);
         if( target )
         {
            int reachRange = _DEFAULT_REACH_RANGE;
            int xRange = abs(userpos.X - where.X);
            int yRange = abs(userpos.Y - where.Y);

            UINT uiRetCanAttackPVP = GAME_RULES::NMPVPCanAttack( user->self, target );
            if( uiRetCanAttackPVP >0 )
            {
               if(uiRetCanAttackPVP == 1)
                  user->self->SendInfoMessage(_STR( 15037 , user->self->GetLang() ),0x0570D5);
               else if(uiRetCanAttackPVP == 2)
                  user->self->SendInfoMessage(_STR( 15038 , user->self->GetLang() ),0x0570D5);
               else if(uiRetCanAttackPVP == 3)
                  user->self->SendInfoMessage(_STR( 15039 , user->self->GetLang() ),0x0570D5);
               else if(uiRetCanAttackPVP == 98)
                  user->self->SendInfoMessage(_STR( 15040 , user->self->GetLang() ),0x0570D5);
               else if(uiRetCanAttackPVP == 99)
                  user->self->SendInfoMessage(_STR( 15345 , user->self->GetLang() ),0x0570D5);
               else if(uiRetCanAttackPVP == 1000)
                  user->self->SendInfoMessage(_STR( 15511 , user->self->GetLang() ),0x0570D5);
               else 
                  user->self->SendInfoMessage(_STR( 15041 , user->self->GetLang() ),0x0570D5);
            }
            else
            {
               // If this is a ranged attack.
               if( user->self->RangedAttack() )
               {
                  // Change the reach range to that of a few screens.
                  reachRange = 25;
               }
               if( xRange <= reachRange && yRange <= reachRange )
               {
                  if( GAME_RULES::InPVP( user->self, target ) )
                  {
                     // Then start auto-combat.
                     user->self->StartAutoCombat( Character::Attack( Character::Attack::normal, 0 ),target);
                  }
                  else
                  {
                     CString csText = _STR( 14, user->self->GetLang() );
                     TFCPacket sending;
                     sending << (RQ_SIZE)RQ_ServerMessage;
                     sending << (short)30;
                     sending << (short)3;
                     sending << csText;
                     sending << (long)CL_BLUE_LIGHT;
                     user->self->SendPlayerMessage( sending );
                  }
               }
               else
               {
                  Broadcast::BCMiss( user->self->GetWL(), _DEFAULT_RANGE,user->self->GetID(),target->GetID(),user->self->GetWL(),target->GetWL());
                  if( target->CanAttack() )
                  {
                     if( target->GetType() == U_NPC )
                     {
                        target->Do( fighting );
                        target->SetTarget( user->self );
                     }
                  }
               }
            }
         }
         else
         {
            // Send a 'missing unit' message to the client.
            TFCPacket sending;
            sending << (RQ_SIZE)RQ_MissingUnit;
            sending << (long)theID;
            sending << (RQ_SIZE)RQ_Attack;
            user->self->SendPlayerMessage( sending );
         }
      }
   }
   RQ_FOOTER( "RQ_Attack" );
}

void TFCMessagesHandler::RQFUNC_CreatePlayer(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   if( user->UsePicklock(__FILE__, __LINE__) )
   {
      if(user->registred && !user->in_game)
      {
         // If user was previously in the reroll menu.
         if( user->boRerolling )
         {
            // Not rerolling anymore.
            user->boRerolling = FALSE;
            // Save rerolled data.
            user->self->SaveCharacter(TRUE,"RQ_CreatePlayer"); //On Create Player in REROLL menu... (not in game)
         }

         unsigned int dummy;
         unsigned char temp_length;
         LPBYTE lpbName = NULL;
         CString name;
         name.Empty();

         // Create a structure to pass to the asynchronous function
         LPRQSTRUCT_CREATE_PLAYER lpStruct = new RQSTRUCT_CREATE_PLAYER;

         // Get the packet
         //msg->Get((short *)(&race));	//  10001
         GET_BYTE( lpStruct->lpbAnswers[ 0 ] );
         GET_BYTE( lpStruct->lpbAnswers[ 1 ] );
         GET_BYTE( lpStruct->lpbAnswers[ 2 ] );
         GET_BYTE( lpStruct->lpbAnswers[ 3 ] );
         GET_BYTE( lpStruct->lpbAnswers[ 4 ] );
         GET_BYTE( lpStruct->lpbAnswers[ 5 ] );
         
         msg->Get((char *)(&temp_length));	// 4	
         lpbName = new BYTE[ temp_length + 1 ];
         for(dummy = 0; dummy < temp_length; dummy++)
            GET_BYTE( lpbName[ dummy ] );
         lpbName[ dummy ] = 0;

         // Setup the structure
         lpStruct->sParams.strParam = lpbName;

         // Format the player name to make it legal.
         FormatPlayerName::Format( &lpStruct->sParams.strParam );

         if (lpbName != NULL)
         {
            delete lpbName;
            lpbName = NULL;
         }

         // Fill default parameters
         lpStruct->sParams.msg  = NULL;
         lpStruct->sParams.user = user;
         lpStruct->sParams.rqRequestID = rqRequestID;
         lpStruct->sParams.strParam.TrimLeft();
         lpStruct->sParams.strParam.TrimRight();

         // Call loading function asynchronously to avoid loading jams.
         AsyncFuncQueue::GetMainQueue()->Call( AsyncRQFUNC_CreatePlayer, lpStruct );
      }
      else
      {
         user->UseUnlock(__FILE__, __LINE__);
      }
   }
   RQ_FOOTER( "RQ_CreatePlayer" );
}

void TFCMessagesHandler::RQFUNC_CastSpell(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   WORD  wMana = 0;
   WORD  wSpell = 0;
   DWORD dwID = 0;
   WorldPos TargetPos = { 0, 0, user->self->GetWL().world };

   GET_WORD( wSpell );		
   GET_WORD( TargetPos.X );
   GET_WORD( TargetPos.Y );
   GET_LONG( dwID );

   if( dwID != 0 )
   {
      WorldPos ppos = user->self->GetWL();
      WorldMap *wl = TFCMAIN::GetWorld( ppos.world );
      if(wl)
      {
         if(user->self->GetArenaID() >0 && user->self->GetArenaTeam() >0)
         {
            if(user->self->GetArenaType() == ARENE1_TYPE)
            {
               Arena1Master::RemTakeList(user,user->self->GetArenaID()-1);
            }
            else if(user->self->GetArenaType() == ARENE2_TYPE)
            {
               Arena2Master::RemTakeList(0,user,user->self->GetArenaID()-1);
               Arena2Master::RemTakeList(1,user,user->self->GetArenaID()-1);
            }
         }

         if( dwID == user->self->GetID() )
         {
            // Start auto-combat.
            user->self->StartAutoCombat(Character::Attack( Character::Attack::spell, wSpell ),user->self);
         }
         else
         {
            Unit *lpuUnit = wl->FindNearUnit( TargetPos, dwID );
            if(lpuUnit)
            {
               // Vaporize spell.
               if( wSpell == theApp.m_dwVaporizeSpellID )
               {
                  user->self->CastSpell( wSpell, lpuUnit );
               }
               else
               {
                  // Start auto-combat.
                  user->self->StartAutoCombat(Character::Attack( Character::Attack::spell, wSpell ),lpuUnit);
               }
            }
            else
            {
            }
         }
      }
   }
   else
   {
      // If target position is empty, then this is a self-spell.
      if( TargetPos.X == 0 && TargetPos.Y == 0 )
      {
         // Start auto-combat.
         user->self->StartAutoCombat(Character::Attack( Character::Attack::spell, wSpell ),user->self);
      }
      else
      {
         user->self->CastSpell( wSpell, TargetPos );
      }
   }
   RQ_FOOTER( "RQ_CastSpell" );
}


void TFCMessagesHandler::RQFUNC_HPchanged(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   TFCPacket sending;

   sending << (RQ_SIZE)RQ_HPchanged;
   sending << (long)user->self->GetHP();
   sending << (long)user->self->GetMaxHP();

   user->self->SendPlayerMessage( sending );

   RQ_FOOTER( "RQ_HPchanged" );
}

void TFCMessagesHandler::RQFUNC_BroadcastTextChange(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   // If the user has no target.
   if( user->self->GetTarget() != NULL )
   {
      // If the NPC is a private talk.
      if( user->self->GetTarget()->IsPrivateTalk() )
      {
         // Sent the message only to the player.
         user->self->SendPlayerMessage( *msg );
         return;
      }
   }
   // Broadcast text change.
   Broadcast::BCast(user->self->GetWL(), _DEFAULT_RANGE, *msg);            

   RQ_FOOTER( "RQ_BroadcastTextChange" );
}

void TFCMessagesHandler::RQFUNC_BreakConversation(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;
   // This request is sent by the client when it wants to break a conversation with a NPC
   DWORD dwID = 0;
   WorldPos wlUnitPos = {0,0,0};
   Unit *lpuUnit;
   WorldMap *world;

   msg->Get((long *)&dwID);
   msg->Get((short *)&wlUnitPos.X);
   msg->Get((short *)&wlUnitPos.Y);

   WorldPos ppos = user->self->GetWL();
   world = TFCMAIN::GetWorld(ppos.world);

   if(world)
   {
      lpuUnit = world->FindNearUnit(wlUnitPos, dwID);
      if(lpuUnit)
      {
         // if the unit can actually be stopped for talking (if it's a NPC).
         if(lpuUnit->GetType() == U_NPC)
         {
            // If unit was "talking" to "user->self" (us)
            //if(lpuUnit->IsDoing() == talking && lpuUnit->GetTarget() == user->self)
            {
               lpuUnit->SetTarget(lpuUnit->GetBond());

               if( !lpuUnit->IsPrivateTalk() || lpuUnit->GetBond() == NULL)
               {
                  WorldPos dest = { -1,-1,-1 };
                  lpuUnit->SetDestination(dest);
			         lpuUnit->Do(wandering,"RQFUNC_BreakConversation");
               }
            }
         }
      }
   }
   RQ_FOOTER( "RQ_BreakConversation" );
}

void TFCMessagesHandler::RQFUNC_ReturnToMenu(PACKET_FUNC_PROTOTYPE)
{    
   RQ_HEADER;
   if(theApp.dwReloadEnable == 0)
   {
      user->self->SendInfoMessage( _STR( 15115 , user->self->GetLang() ),0x0020FF);
      return;
   }

   TFCPacket sending; 
   if(user->in_game)
   {
	  if(theApp.dwAntiplugSystem == 1)
	  {
		  bool bSafe = false;
		  if(user->self->GetUnderBlockMap()  == __SAFE_HAVEN   || user->self->GetUnderBlockMap()  == __INDOOR_SAFE_HAVEN || user->IsGod())
			  bSafe = true;

		  if(bSafe)
		  {
			  if(user->in_AskReloadPlayer == FALSE)
				  user->in_AskReloadPlayer = TRUE;
		  }
		  else
		  {
			  if(user->dwReloadDecompte == 0xFFFF)//*****//check if another exit altready in progress
			  {
				  //Set the decompt Timout
				  user->dwReloadDecompte = theApp.dwAntiplugSystemSec;//*****//
				  CString csText = _STR( 15395, user->self->GetLang() );
				  user->self->SendSystemMessage(csText);
			  }
		  }
		  
	  }
	  else
	  {
		  if(user->in_AskReloadPlayer == FALSE)
			 user->in_AskReloadPlayer = TRUE;
	  }
   }
   else
   {
      sending << (RQ_SIZE)RQ_ReturnToMenu;
      sending << (char)1;
   }
   user->self->SendPlayerMessage( sending );
   RQ_FOOTER( "RQ_ReturnToMenu" );
}

void TFCMessagesHandler::RQFUNC_SendTrainSkillList(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   // When sent by the client, this request is used to train a skill.
   WorldPos wlPos = { 0, 0, 0 };
   DWORD dwID;
   Unit *lpuTarget;
   NPC_DATA npcData;
   TRAIN_DATA npcTrainData;
   WorldMap *world = NULL;
   WORD wNbSkills = 0;


   msg->Get( (short *)&wlPos.X );	// Approximative NPC position
   msg->Get( (short *)&wlPos.Y );
   msg->Get( (long  *)&dwID );		// ID of NPC who trains

   world = TFCMAIN::GetWorld( user->self->GetWL().world );

   if( world )
   {
      lpuTarget = world->FindNearUnit( wlPos, dwID );

      if( lpuTarget )
      {
         MultiLock( user->self, lpuTarget );

         msg->Get( (short *)&wNbSkills );	// Quantity of skills to train	

         npcData.DataID = __TRAIN_DATA;	// Train data exchange with NPC

         int i;
         for( i = 0; i < wNbSkills; i++ )
         {
            npcTrainData.wSkillID = npcTrainData.wSkillPnts = 0;

            msg->Get( (short *)&npcTrainData.wSkillID );	// ID of the skill to be trained
            msg->Get( (short *)&npcTrainData.wSkillPnts );	// Number of skill pnts to train

            // Init the NPC data exchange structure
            npcData.Data = &npcTrainData;
            // Then shoot the training request to the NPC
            lpuTarget->SendUnitMessage( MSG_OnNPCDataExchange, lpuTarget, NULL, user->self, &npcData, NULL );
         }
         user->self->Unlock();
         lpuTarget->Unlock();
      }
   }
   RQ_FOOTER( "RQ_SendTrainSkillList" );
}

void TFCMessagesHandler::RQFUNC_UseSkill(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   // This request is used by the client to use a skill
   WorldPos wlTargetPos = { 0, 0, user->self->GetWL().world };
   DWORD dwID = 0;
   WORD wSkill = 0;

   msg->Get( (short *)&wSkill );
   msg->Get( (short *)&wlTargetPos.X );
   msg->Get( (short *)&wlTargetPos.Y );
   msg->Get( (long *) &dwID );

   // if an ID was specified, skill is used on someone
   if( dwID != 0 )
   {
      WorldPos ppos = user->self->GetWL();
      WorldMap *wl = TFCMAIN::GetWorld(ppos.world);
      if( wl != NULL )
      {
         // If position is valid.
         if( wl->IsValidPosition( wlTargetPos ) )
         {
            if( dwID == user->self->GetID() )
            {
               user->self->UseSkill( wSkill, user->self, NULL );
            }
            else
            {
               Unit *lpuUnit = wl->FindNearUnit(wlTargetPos, dwID);
               if( lpuUnit != NULL )
               {
                  user->self->UseSkill(wSkill, lpuUnit, NULL);
               }
            }
         }
      }
   }
   else
   {
      user->self->UseSkill(wSkill, wlTargetPos);
   }

   RQ_FOOTER( "RQ_UseSkill" );
}

void TFCMessagesHandler::RQFUNC_XPchanged(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;
   user->self->SendPlayerXP(true);
   RQ_FOOTER( "RQ_XPchanged" );
}

void TFCMessagesHandler::RQFUNC_GetTime(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   TFCPacket sending;
   // This request sends the current time to the client,
   sending << (RQ_SIZE)RQ_GetTime;
   sending << (char )TFCTime::Second();
   sending << (char )TFCTime::Minute();
   sending << (char )TFCTime::Hour();
   sending << (char )TFCTime::Week();
   sending << (char )TFCTime::Day();
   sending << (char )TFCTime::Month();
   sending << (short)TFCTime::Year();

   WorldPos wlPos = { -1, -1, -1 };
   CPacketManager::SendPacket( sending, sockAddrO,sockAddrI, 0, wlPos, FALSE ); //OK

   RQ_FOOTER( "RQ_GetTime" );
}

void TFCMessagesHandler::RQFUNC_EnterChatterChannel(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   BYTE   chClearAllUserCCList; 
   BYTE   chUpdateList; 
   LPBYTE szChannel;
   LPBYTE szPassword;

   GET_BYTE  ( chClearAllUserCCList );
   GET_BYTE  ( chUpdateList );
   // Get the channel and password from the packet.
   GET_STRING( szChannel );
   auto_ptr< BYTE > cAutoDeleteChannel( szChannel );     // insure auto-deletion.
   GET_STRING( szPassword );                
   auto_ptr< BYTE > cAutoDeletePassword( szPassword );   // insure auto-deletion.

   // Chatter channels need STL strings. Convert.
   string csChannel( reinterpret_cast< const char *>( szChannel ) );
   string csPassword( reinterpret_cast< const char *>( szPassword ) );

   TRACE( "\nChannel='%s', Password='%s'.", csChannel.c_str(), csPassword.c_str() );

   ChatterChannels &cChatter = CPlayerManager::GetChatter();

   if(chClearAllUserCCList)
   {
      //is the first CC to add, we clear all CC associated with this player...
      //cChatter.Remove(user->self->GetPlayer());
   }

   // If the user could be added to the chatter channel.
   if( !cChatter.AddCCPlayer( user, csChannel, csPassword ) )
   {
      CString cstrChannel = csChannel.c_str();

      // Send a RQ_EnterChatterChannel (this is an error).
      TFCPacket sending;
      sending << (RQ_SIZE)RQ_EnterChatterChannel;
      sending << cstrChannel;
      user->self->SendPlayerMessage( sending );
   }
   else
   {
      // Send only the registered list of channels.
      if(chUpdateList == 1)
         cChatter.SendRegisteredChannelList( user );
   }

   RQ_FOOTER( "RQ_EnterChatterChannel" );
}

void TFCMessagesHandler::RQFUNC_SendTeachSkillList(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   // When sent by the client, this request is used to be taught a skill
   WorldPos wlPos = { 0, 0, 0 };
   DWORD dwID;
   short shNbrSkill = 0;
   Unit *lpuTarget;

   WorldMap *world = NULL;

   msg->Get( (short *)&wlPos.X );	 // Approximative NPC position
   msg->Get( (short *)&wlPos.Y );			
   msg->Get( (long  *)&dwID );		 // ID of NPC who trains		
   msg->Get( (short *)&shNbrSkill ); // Nbr ID to read

   // Fetch all trained skills until the packet is empty.
   list< WORD > taughtSkills;
   for(int i=0;i<shNbrSkill;i++)
   {
      WORD skillId = 0;
      GET_WORD( skillId );
      taughtSkills.push_back( skillId );
   }

   wlPos.world = user->self->GetWL().world;
   world = TFCMAIN::GetWorld( user->self->GetWL().world );
   if( world )
   {
      // then shoot the talk to the targetted NPC
      lpuTarget = world->FindNearUnit( wlPos, dwID );
      if( lpuTarget )
      {
         MultiLock( user->self, lpuTarget );

         // For all taught skills.
         list< WORD >::iterator a;
         for( a = taughtSkills.begin(); a != taughtSkills.end(); a++ )
         {
            TEACH_DATA npcTeachData = { *a };
            // Init the NPC data exchange structure
            NPC_DATA npcData = { __TEACH_DATA, &npcTeachData };

            lpuTarget->SendUnitMessage(MSG_OnNPCDataExchange, lpuTarget, NULL, user->self, &npcData, NULL );
         }
         user->self->Unlock();
         lpuTarget->Unlock();
      }
   }
   RQ_FOOTER( "RQ_SendTeachSkillList" );
}

void TFCMessagesHandler::RQFUNC_SendBuyItemList( PACKET_FUNC_PROTOTYPE )
{
   RQ_HEADER;

   DWORD dwID;
   WORD wNbItems;
   int i;
   Unit *lpShopUnit;
   WorldMap *wl;
   NPC_DATA npcData = { 0, NULL };
   SHOP_DATA npcShopData = { 0, 0, 0 };

   // Init the NPC data exchange structure
   npcData.DataID = __SHOP_DATA;
   npcData.Data = &npcShopData;

   WorldPos wlPos = { 0, 0, user->self->GetWL().world };
   wl = TFCMAIN::GetWorld( wlPos.world );
   if( wl )
   {
      npcShopData.Action = __BUY;

      GET_WORD( wlPos.X );
      GET_WORD( wlPos.Y );
      GET_LONG( dwID );

      // Find the unit
      lpShopUnit = wl->FindNearUnit( wlPos, dwID );

      if( lpShopUnit )
      {
         MultiLock( user->self, lpShopUnit );
         GET_WORD( wNbItems );
         for( i = 0; i < wNbItems; i++ )
         {
            GET_WORD( npcShopData.Item );
            GET_WORD( npcShopData.wQuantity );
            lpShopUnit->SendUnitMessage(MSG_OnNPCDataExchange, lpShopUnit, NULL, user->self, &npcData, NULL );
         }

         // Update backpack.
         TFCPacket sending;
         sending << (RQ_SIZE)RQ_ViewBackpack2;
         sending << (char)0;	// Don't show backpack..
         sending << (long)user->self->GetID();
         user->self->PacketBackpack( sending );
         user->self->SendPlayerMessage( sending );

         // Update gold
         user->self->SetGold( user->self->GetGold() );

         user->self->Unlock();
         lpShopUnit->Unlock();
      }
   }
   RQ_FOOTER( "RQ_SendBuyItemList" );
}

void TFCMessagesHandler::RQFUNC_SendPointsItemList( PACKET_FUNC_PROTOTYPE )
{
   RQ_HEADER;

   DWORD dwID;
   WORD wNbItems;
   int i;
   Unit *lpShopUnit;
   WorldMap *wl;
   NPC_DATA npcData = { 0, NULL };
   SHOP_DATA npcShopData = { 0, 0, 0 };

   // Init the NPC data exchange structure
   npcData.DataID = __POINTS_DATA;
   npcData.Data = &npcShopData;

   WorldPos wlPos = { 0, 0, user->self->GetWL().world };
   wl = TFCMAIN::GetWorld( wlPos.world );
   if( wl )
   {
      npcShopData.Action = __BUY;

      GET_WORD( wlPos.X );
      GET_WORD( wlPos.Y );
      GET_LONG( dwID );

      // Find the unit
      lpShopUnit = wl->FindNearUnit( wlPos, dwID );

      if( lpShopUnit )
      {
         MultiLock( user->self, lpShopUnit );
         GET_WORD( wNbItems );
         for( i = 0; i < wNbItems; i++ )
         {
            GET_WORD( npcShopData.Item );
            GET_WORD( npcShopData.wQuantity );
            lpShopUnit->SendUnitMessage(MSG_OnNPCDataExchange, lpShopUnit, NULL, user->self, &npcData, NULL );
         }

         // Update backpack.
         TFCPacket sending;
         sending << (RQ_SIZE)RQ_ViewBackpack2;
         sending << (char)0;	// Don't show backpack..
         sending << (long)user->self->GetID();
         user->self->PacketBackpack( sending );
         user->self->SendPlayerMessage( sending );

         user->self->Unlock();
         lpShopUnit->Unlock();
      }
   }
   RQ_FOOTER( "RQ_SendPointsItemList" );
}

void TFCMessagesHandler::RQFUNC_SendSellItemList( PACKET_FUNC_PROTOTYPE )
{
   RQ_HEADER;
   DWORD dwID;
   WorldMap *wl;
   short shQty = 0;
   Unit *lpShopUnit;
   NPC_DATA npcData = { 0, NULL };
   SHOP_DATA npcShopData = { 0, 0, 0 };

   // Init the NPC data exchange structure
   npcData.DataID = __SHOP_DATA;
   npcData.Data = &npcShopData;

   WorldPos wlPos = { 0, 0, user->self->GetWL().world };		
   wl = TFCMAIN::GetWorld( wlPos.world );
   if( wl )
   {
      npcShopData.Action = __SELL;
      GET_WORD( wlPos.X );	// NPC pos
      GET_WORD( wlPos.Y );
      GET_LONG( dwID );		// NPC ID
      GET_WORD( shQty );	// Qty
      // Find the unit
      lpShopUnit = wl->FindNearUnit( wlPos, dwID );
      if( lpShopUnit )
      {
         MultiLock( user->self, lpShopUnit );
         // Fetch shop datas until packet ends.
         for(int i=0;i<shQty;i++)
         {
            GET_LONG( npcShopData.ID );	// ID of the backpack item to sell.
            GET_LONG( npcShopData.wQuantity );

            lpShopUnit->SendUnitMessage(MSG_OnNPCDataExchange, lpShopUnit, NULL, user->self, &npcData, NULL );
         }

         user->self->Unlock();
         lpShopUnit->Unlock();
      }
   }
   RQ_FOOTER( "RQ_SendSellItemList" );
}

void TFCMessagesHandler::RQFUNC_SendStatTrain( PACKET_FUNC_PROTOTYPE )
{
   RQ_HEADER;

   WORD wTrain;
   DWORD dwStat;
   CString csReport;
   CString csText;

   csReport.Format( "Player %s trains stats.", (LPCTSTR)user->self->GetTrueName() );

   user->self->Lock();
   RQTRAIN_STAT( STR, "STR" );
   
   if(theApp.dwEquilibrageNewCourbeXPEnable == 0) //StatTrain END
   {
      RQTRAIN_STAT( END, "END" );	
   }
   else
   {
      GET_WORD( wTrain );
      if( wTrain != 0 )
      {
         dwStat = user->self->GetTrueEND();
         if( user->self->UseStatPnts( wTrain ) )
         {
            user->self->SetEND( (WORD)( dwStat + wTrain ) );
            csText.Format( "Trained %u points in ""END"". %u -> %u.", wTrain, dwStat, user->self->GetTrueEND() );
            csReport += csText;

            DWORD dwHPGain  = 0;
            DWORD dwOldEnd  = dwStat;
            DWORD dwNewEnd  = dwStat + wTrain;
            DWORD dwLv1End  = 20 + 5*user->self->ViewFlag(__FLAG_NUMBER_OF_REMORTS) + 5*user->self->ViewFlag(__FLAG_NMS_DECHU);
            WORD wLevel     = user->self->GetLevel();
            int iNbrIT=(dwNewEnd/20)-(dwOldEnd/20);
            for (int i=1;i<=iNbrIT;i++)
            {
               dwHPGain = dwHPGain + wLevel-1-(dwNewEnd-20*(iNbrIT-i)-dwLv1End)/5;
            }

            DWORD dwMaxHP = user->self->GetTrueMaxHP(); 
            DWORD dwHP    = user->self->GetHP();
            user->self->SetMaxHP( dwMaxHP+dwHPGain );
            user->self->SetHP( dwHP+dwHPGain, true );

            csText.Format( "Calc New MAXHP , gain %d Hp. Now ( %d )", dwHPGain, dwMaxHP+dwHPGain );
            csReport += csText;

            TFormat format;
            user->self->SendSystemMessage( format(_STR(15380, user->self->GetLang()),dwHPGain,dwHP+dwHPGain,dwMaxHP+dwHPGain), RGB( 0, 255, 51 ) );
         }
      }
   }

   RQTRAIN_STAT( AGI, "AGI" );
   GET_WORD( wTrain ); // Get the byte but drop the training of willpower

   if(theApp.dwEquilibrageNewCourbeXPEnable == 0) //Stat Train WIDS et INT
   {
      RQTRAIN_STAT( WIS, "WIS" );
      RQTRAIN_STAT( INT, "INT" );
   }
   else
   {
      bool bNeedUpdateMaxMana = false;
      DWORD dwWisGain  = 0;
      DWORD dwOldWis   = 0;
      DWORD dwNewWis   = 0;
      DWORD dwIntGain  = 0;
      DWORD dwOldInt   = 0;
      DWORD dwNewInt   = 0;



      GET_WORD( wTrain );
      if( wTrain != 0 )
      {
         dwStat = user->self->GetTrueWIS();
         if( user->self->UseStatPnts( wTrain ) )
         {
            user->self->SetWIS( (WORD)( dwStat + wTrain ) );
            csText.Format( "Trained %u points in ""WIS"". %u -> %u.", wTrain, dwStat, user->self->GetTrueWIS() );
            csReport += csText;

                  dwOldWis  = dwStat;
                  dwNewWis  = dwStat + wTrain;
            DWORD dwLv1Wis  = 20 + 5*user->self->ViewFlag(__FLAG_NUMBER_OF_REMORTS) + 5*user->self->ViewFlag(__FLAG_NMS_DECHU);
            WORD wLevel     = user->self->GetLevel();

            int iNbrIT = (dwNewWis/60)-(dwOldWis/60);
            for (int i=1;i<=iNbrIT;i++)
            {
               dwWisGain = dwWisGain + wLevel-1-(dwNewWis-60*(iNbrIT-i)-dwLv1Wis)/5;
            }
            bNeedUpdateMaxMana = true;
         }
      }

      GET_WORD( wTrain );
      if( wTrain != 0 )
      {
         dwStat = user->self->GetTrueINT();
         if( user->self->UseStatPnts( wTrain ) )
         {
            user->self->SetINT( (WORD)( dwStat + wTrain ) );
            csText.Format( "Trained %u points in ""INT"". %u -> %u.", wTrain, dwStat, user->self->GetTrueINT() );
            csReport += csText;

            dwOldInt  = dwStat;
            dwNewInt  = dwStat + wTrain;
            DWORD dwLv1Wis  = 20 + 5*user->self->ViewFlag(__FLAG_NUMBER_OF_REMORTS) + 5*user->self->ViewFlag(__FLAG_NMS_DECHU);
            WORD wLevel     = user->self->GetLevel();

            int iNbrIT = (dwNewInt/30)-(dwOldInt/30);
            for (int i=1;i<=iNbrIT;i++)
            {
               dwIntGain = dwIntGain + wLevel-1-(dwNewInt-30*(iNbrIT-i)-dwLv1Wis)/5;
            }

            bNeedUpdateMaxMana = true;
         }
      }

      if(bNeedUpdateMaxMana)
      {
         //calcule new PM with new END...
         DWORD dwMPGain = dwWisGain+dwIntGain;
         DWORD dwMaxMP = user->self->GetTrueMaxMana();          
         DWORD dwMP = user->self->GetMana();
         user->self->SetMaxMana( dwMaxMP+dwMPGain );
         user->self->SetMana( dwMP+dwMPGain );

         csText.Format( "Calc New MAXMana , gain %d Hp. Now ( %d )", dwMPGain, dwMaxMP+dwMPGain );
         csReport += csText;

         TFormat format;
         user->self->SendSystemMessage( format(_STR(15381, user->self->GetLang()),dwMPGain,dwMP+dwMPGain,dwMaxMP+dwMPGain), RGB( 0, 255, 51 ) );
      }

      
   }


   GET_WORD( wTrain ); 

   _LOG_PC
      LOG_MISC_1,
         (char *)(LPCTSTR)csReport
      LOG_

   user->self->Unlock();

   RQ_FOOTER( "RQ_SendStatTrain" );
}

//  Queries the unit's existence.
void TFCMessagesHandler::RQFUNC_QueryUnitExistence( PACKET_FUNC_PROTOTYPE )
/******************************************************************************/
{
   RQ_HEADER;

   if( user->self == NULL )
      return;

   WorldPos wlPos = { 0, 0, user->self->GetWL().world };
   DWORD dwID;

   // Fetch packet information
   GET_LONG( dwID );
   GET_WORD ( wlPos.X );
   GET_WORD ( wlPos.Y );

   // Get the world instance.
   WorldMap *wlWorld = TFCMAIN::GetWorld( wlPos.world );
   if( wlWorld != NULL )
   {
      // Try to find the unit where 
      Unit *lpUnit = wlWorld->FindNearUnit( wlPos, dwID );

      bool boMissingUnit = false;
      // If the unit was not found.
      if( lpUnit == NULL )
      {
         boMissingUnit = true;
      }
      else
      {
         // If this unit is a character
         if( lpUnit->GetType() == U_PC )
         {
            // Get its underlying player structure
            Players *lpPlayer = static_cast< Character * >( lpUnit )->GetPlayer();

            // If packets are not allowed to be send to this unit.
            if( lpPlayer != NULL && !lpPlayer->self->GetInvisibleQuery()->SendPacketTo( user->self ))
            {
               // Declare it non-existent.
               boMissingUnit = true;
            }
         }
      }
      // If the unit was not found.
      if( !boMissingUnit )
      {
         // Packet the unit's popup information (the unit is certain to be non-NULL)
         TFCPacket sending;
         lpUnit->PacketPopup( lpUnit->GetWL(), sending );
         // Send that packet to the requester
         user->self->SendPlayerMessage( sending );
      }
   }
   RQ_FOOTER( "RQ_QueryUnitExistence" );
}

void TFCMessagesHandler::RQFUNC_UseItemByAppearance( PACKET_FUNC_PROTOTYPE )
{
   RQ_HEADER;
   WORD wAppearance = 0;

   GET_WORD( wAppearance );
   if ( !user->in_game )
      return;

   // If character could not use such an item.
   // On vérifie si le pointeur est null (cela corrige certain crash serveur)
   if( user->self == NULL )
   {
      _LOG_DEBUG
         LOG_DEBUG_LVL1,
         "user->self (%s) is NULL in UseItemByAppearance", 
         user->GetAccount()
         LOG_
   }
   else if( !user->self->UseItemByAppearance( wAppearance, user->self ) )
   {
      // Send a message telling the player that its out of this item.
      TFCPacket sending;
      sending << (RQ_SIZE)RQ_CannotFindItemByAppearance;
      sending << (short)wAppearance;

      user->self->SendPlayerMessage( sending );
   }
   RQ_FOOTER( "RQ_UseItemByAppearance" );
}



void TFCMessagesHandler::RQFUNC_UpdateSmile( PACKET_FUNC_PROTOTYPE )
{
   RQ_HEADER;
   WORD wSmileID = 0;

   GET_WORD( wSmileID );
   if ( !user->in_game )
      return;

   if(user->self)
   {
      TFCPacket sending;
      sending << (RQ_SIZE)RQ_UpdateSmile;
      sending << (short)  wSmileID;
      sending << (long)   user->self->GetID();
      Broadcast::BCast( user->self->GetWL(), _DEFAULT_RANGE, sending );
   }
   RQ_FOOTER( "RQFUNC_UpdateSmile" );
}

void TFCMessagesHandler::RQFUNC_TogglePage( PACKET_FUNC_PROTOTYPE )
{
   RQ_HEADER;

   BYTE newState;
   GET_BYTE( newState );

   if( newState == 0 )
      user->TogglePage( false );
   else
      user->TogglePage( true );

   RQ_FOOTER( "RQ_TogglePage" );
}

void TFCMessagesHandler::RQFUNC_Rob ( PACKET_FUNC_PROTOTYPE )
{
   RQ_HEADER;
   DWORD objId = 0;
   GET_LONG( objId );
   user->self->Rob( objId );
   RQ_FOOTER( "RQ_Rob" );
}

void TFCMessagesHandler::RQFUNC_NM_NMSGOLD_AchatOpt1(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   if(!user->self)
      return;

   long lNbrNMSGold = user->GetNMSGold();
   TFCPacket sending;
   sending << (RQ_SIZE)RQ_NM_NMSGOLD_AchatOpt1;
   sending << (long)lNbrNMSGold;
   //the number of items
   unsigned short nbrItems = theApp.m_aAchatOpt1.GetSize();
   sending << (short)nbrItems;

   for(int i=0;i<nbrItems;i++)
   {
      //send name + Desc + prix
      sending << theApp.m_aAchatOpt1[i].strName;

      CString csDesc = _STR( theApp.m_aAchatOpt1[i].iDesc, user->self->GetLang() );
      sending << csDesc;
      sending << (long)theApp.m_aAchatOpt1[i].iCost;
   }
   user->self->SendPlayerMessage(sending);

   RQ_FOOTER( "RQFUNC_NM_NMSGOLD_AchatOpt1" );
}

void TFCMessagesHandler::RQFUNC_NM_NMSGOLD_AchatOpt2(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   if(!user->self)
      return;
  
   long lNbrNMSGold = user->GetNMSGold();
   TFCPacket sending;
   sending << (RQ_SIZE)RQ_NM_NMSGOLD_AchatOpt2;
   sending << (long)lNbrNMSGold;
   //the number of items
   unsigned short nbrItems = theApp.m_aAchatOpt2.GetSize();
   sending << (short)nbrItems;
   for(int i=0;i<nbrItems;i++)
   {
      //send name + Desc + prix
      CString csRealName;
      csRealName.Format("%s [Qté. %d]",theApp.m_aAchatOpt2[i].strName,theApp.m_aAchatOpt2[i].iNbrItem);
      sending << csRealName;
      sending << (long)theApp.m_aAchatOpt2[i].iCost;
      CString csDesc = _STR( theApp.m_aAchatOpt2[i].iDesc, user->self->GetLang() );
      sending << csDesc;
   }
   user->self->SendPlayerMessage(sending);
   
   RQ_FOOTER( "RQFUNC_NM_NMSGOLD_AchatOpt2" );
}

void TFCMessagesHandler::RQFUNC_NM_NMSGOLD_AchatOpt3(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;
   if(!user->self)
      return;

   long lNbrNMSGold = user->GetNMSGold();

   TFCPacket sending;
   sending << (RQ_SIZE)RQ_NM_NMSGOLD_AchatOpt3;
   sending << (long)lNbrNMSGold;

   //the number of items
   unsigned short nbrItems = theApp.m_aAchatOpt3.GetSize();
   sending << (short)nbrItems;

   for(int i=0;i<nbrItems;i++)
   { 
      sending << theApp.m_aAchatOpt3[i].strName;
      sending << (long)theApp.m_aAchatOpt3[i].iCost;
      CString csDesc = _STR( theApp.m_aAchatOpt3[i].iDesc, user->self->GetLang() );
      sending << csDesc;
   }
   user->self->SendPlayerMessage(sending);

   RQ_FOOTER( "RQFUNC_NM_NMSGOLD_AchatOpt3" );
}

void TFCMessagesHandler::RQFUNC_NM_NMSGOLD_AchatOpt4(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   if(!user->self)
      return;

   RQ_FOOTER( "RQFUNC_NM_NMSGOLD_AchatOpt4" );
}

void TFCMessagesHandler::RQFUNC_NM_NMSGOLD_Sanction(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   if(!user->self)
      return;

   //les sanctions
   int iSanctionA = user->GetSanctionA();
   int iSanctionB = user->GetSanctionB();
   unsigned short nbrSanction =  (iSanctionB*4)+iSanctionA;
   //la prison
   DWORD dwPrisonRemain = 0;
   DWORD dwPrisonTS = user->self->ViewFlag(__FLAG_PRISON_TIMESTAMP);
   if(dwPrisonTS >0)
   {
      time_t TimeMsTmp =  time(NULL);
      dwPrisonRemain = (DWORD)dwPrisonTS-TimeMsTmp;
   }

   TFCPacket sending;
   sending << (RQ_SIZE)RQ_NM_NMSGOLD_Sanction;
   sending << (short)nbrSanction;
   sending << (long)dwPrisonRemain;
   user->self->SendPlayerMessage(sending);
   RQ_FOOTER( "RQFUNC_NM_NMSGOLD_Sanction" );
}

void TFCMessagesHandler::RQFUNC_SvrOptions(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   if(!user->self)
      return;

   _LOG_GAMEOP
      LOG_SYSOP,
         "Receive Server Options List %s  [%s]",user->self->GetTrueName(),user->GetAccount()
      LOG_

   if(user->GetGodFlags() & GOD_CAN_GIVE_FLAG_TO_HIM)
   {
      for(int i=0;i<NBR_OPT_GM_OPTIONS;i++)
         m_chOptionsValue[i] = 0;

      if(theApp.dwPVPDropDisabled != 1)
         m_chOptionsValue[0] = 1;
      if(theApp.dwPVMDropDisabled != 1)
         m_chOptionsValue[1] = 1;
      if(theApp.dwUDPFilterEnable == 1)
         m_chOptionsValue[2] = 1;
      if(theApp.dwUDPLogAnalyseEnable == 1)
         m_chOptionsValue[3] = 1;
      if(theApp.dwReloadEnable == 1)
         m_chOptionsValue[4] = 1;
      if(theApp.dwGuildSystemEnable == 1)
         m_chOptionsValue[5] = 1;
      if(theApp.dwAHSystemEnable == 1)
         m_chOptionsValue[6] = 1;
      if(theApp.dwProfessionSystemEnable == 1)
         m_chOptionsValue[7] = 1;
      if(theApp.dwSendDamageHealingSystem == 1)
         m_chOptionsValue[8] = 1;
      if(theApp.m_dwModeRPorHRP == 1)
         m_chOptionsValue[9] = 1;
      if(theApp.m_dwPVPSyetem2Actif == 1)
         m_chOptionsValue[10] = 1;
      if(theApp.m_dwDUELSyetemActif == 1)
         m_chOptionsValue[11] = 1;
      if(theApp.m_dwGMMsgSystem == 1)
         m_chOptionsValue[12] = 1;
      if(theApp.m_dwManagePrisonExit == 1)
         m_chOptionsValue[13] = 1;
      if(theApp.m_dwPseudoname == 1)
         m_chOptionsValue[14] = 1;
      if(theApp.m_dwCCShortcut == 1)
         m_chOptionsValue[15] = 1;
      if(theApp.m_dwXPstat == 1)
         m_chOptionsValue[16] = 1;
      if(theApp.dwManageBankInteret == 1)
         m_chOptionsValue[17] = 1;
      if(theApp.dwManageScrollXP == 1)
         m_chOptionsValue[18] = 1;
      if(theApp.dwAntiplugSystem == 1)
         m_chOptionsValue[19] = 1;


      int iNbrOptions = NBR_OPT_GM_OPTIONS;
      TFCPacket sending;
      sending << (RQ_SIZE)RQ_SvrOptions;
      sending << (long)iNbrOptions;
      for(int i=0;i<NBR_OPT_GM_OPTIONS;i++)
      {
         sending << m_strOptionsTexte[i];
         sending << (char)m_chOptionsValue[i];
      }
      user->self->SendPlayerMessage(sending);
   }

   RQ_FOOTER( "RQFUNC_SvrOptions" );
}

void TFCMessagesHandler::RQFUNC_GetEventsList(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   if(!user->self)
      return;

   _LOG_GAMEOP
      LOG_SYSOP,
      "Receive GetEvents List %s  [%s]",user->self->GetTrueName(),user->GetAccount()
      LOG_

      if(user->GetGodFlags() & GOD_CAN_TELEPORT)
      {
         EventsMaster::SendEventList(user->self);
      }

      RQ_FOOTER( "RQ_GetEventsList" );
}

void TFCMessagesHandler::RQFUNC_AttackMode(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;
   int dwID;
   GET_LONG(dwID);
   user->self->NMCombatMode(dwID);
   RQ_FOOTER( "RQFUNC_AttackMode" );
}

void TFCMessagesHandler::RQFUNC_SendTeachFormuleList(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   if(!user->self)
      return;

   if(theApp.dwProfessionSystemEnable == 0 || (theApp.dwProfessionSystemEnable == 2 && !user->IsGod()))   
   {
      user->self->SendInfoMessage( _STR( 15152 , user->self->GetLang() ),0x0020FF);
      return;
   }

   //////////////////////////////////////////////////////////////////////////////////////////
   // When sent by the client, this request is used to be taught a skill
   WorldPos wlPos = { 0, 0, 0 };
   DWORD dwID;
   short shNbrSkill = 0;
   Unit *lpuTarget;

   WorldMap *world = NULL;


   msg->Get( (short *)&wlPos.X );	// Approximative NPC position
   msg->Get( (short *)&wlPos.Y );			
   msg->Get( (long  *)&dwID );		// ID of NPC who trains	
   msg->Get( (short *)&shNbrSkill ); // Nbr ID to read

   // Fetch all trained skills until the packet is empty.
   list< WORD > taughtFormules;

   for(int i=0;i<shNbrSkill;i++)
   {
      WORD formuleId = 0;
      GET_WORD( formuleId );
      taughtFormules.push_back( formuleId );
   };


   wlPos.world = user->self->GetWL().world;

   world = TFCMAIN::GetWorld( user->self->GetWL().world );

   if( world )
   {			
      // then shoot the talk to the targetted NPC
      lpuTarget = world->FindNearUnit( wlPos, dwID );
      if( lpuTarget )
      {

         MultiLock( user->self, lpuTarget );

         // For all taught skills.
         list< WORD >::iterator a;
         for( a = taughtFormules.begin(); a != taughtFormules.end(); a++ )
         {

            TEACH_DATAF npcTeachData = { *a };
            // Init the NPC data exchange structure
            NPC_DATA npcData = { __TEACH_DATAF, &npcTeachData };


            lpuTarget->SendUnitMessage(MSG_OnNPCDataExchange, lpuTarget, NULL, user->self, &npcData, NULL );

         }

         user->self->Unlock();
         lpuTarget->Unlock();
      }

   }


   RQ_FOOTER( "RQ_NM_SendTeachFormuleList" );
}


void TFCMessagesHandler::RQFUNC_AskCompagnonName(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   if(!user->self)
      return;

   unsigned char temp_length;
   unsigned char temp_byte;
   unsigned char dummy;

   CString name;
   name.Empty();

   msg->Get((char *)(&temp_length));		

   // Retrieves the name
   for(dummy = 0; dummy < temp_length; dummy++)
   {
      msg->Get((char *)(&temp_byte));
      name += (TCHAR)temp_byte;
   }

   user->self->SetCompagnonName(name);



   RQ_FOOTER( "RQFUNC_AskCompagnonName" );
}

void TFCMessagesHandler::RQFUNC_ARENA1_Join(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   if(!user->self)
      return;

   long lAreneID;
   long lAreneType;
   msg->Get( (long  *)&lAreneID );		// ID 
   msg->Get( (long  *)&lAreneType );	// Type

   if(lAreneType == ARENE1_TYPE)
   {
      if(lAreneID < 0 || lAreneID >= Arena1Master::GetNumberOfArene())
         return;

      if(!theApp.m_dwArenaSystem1[lAreneID])
      {
         CString strTmp;
         strTmp.Format(_STR( 15471 , user->self->GetLang() ),Arena1Master::GetAreneName(lAreneID));
         user->self->SendInfoMessage(strTmp ,CL_RED);
         return;
      }
      Arena1Master::AddPlayer(user,lAreneID);
   }
   else if(lAreneType == ARENE2_TYPE)
   {
      if(lAreneID < 0 || lAreneID >= Arena2Master::GetNumberOfArene())
         return;

      if(!theApp.m_dwArenaSystem2[lAreneID])
      {
         CString strTmp;
         strTmp.Format(_STR( 15471 , user->self->GetLang() ),Arena2Master::GetAreneName(lAreneID));
         user->self->SendInfoMessage(strTmp ,CL_RED);
         return;
      }
      Arena2Master::AddPlayer(user,lAreneID);
   }

   
   RQ_FOOTER( "RQFUNC_ARENA1_Join" );
}

void TFCMessagesHandler::RQFUNC_ARENA1_Leave(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;
   
   if(!user->self)
      return;

   long lAreneID;
   long lAreneType;
   msg->Get( (long  *)&lAreneID );		// ID 
   msg->Get( (long  *)&lAreneType );	// Type

   if(lAreneType == ARENE1_TYPE)
   {
      if(lAreneID < 0 || lAreneID >= Arena1Master::GetNumberOfArene())
         return;
      if(!theApp.m_dwArenaSystem1[lAreneID])
      {
         CString strTmp;
         strTmp.Format(_STR( 15471 , user->self->GetLang() ),Arena1Master::GetAreneName(lAreneID));
         user->self->SendInfoMessage(strTmp ,CL_RED);
         return;
      }
      Arena1Master::RemovePlayer(user,true,lAreneID,TRUE);
   }
   else if(lAreneType == ARENE2_TYPE)
   {
      if(lAreneID < 0 || lAreneID >= Arena2Master::GetNumberOfArene())
         return;
      if(!theApp.m_dwArenaSystem2[lAreneID])
      {
         CString strTmp;
         strTmp.Format(_STR( 15471 , user->self->GetLang() ),Arena2Master::GetAreneName(lAreneID));
         user->self->SendInfoMessage(strTmp ,CL_RED);
         return;
      }
      Arena2Master::RemovePlayer(user,true,lAreneID,TRUE);
   }

   RQ_FOOTER( "RQFUNC_ARENA1_Leave" );
}

void TFCMessagesHandler::RQFUNC_ARENA1_GetWaitPlayerList(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;

   if(!user->self)
      return;
    
   long lAreneID;
   long lAreneType;
   msg->Get( (long  *)&lAreneID );		// ID
   msg->Get( (long  *)&lAreneType );	// Type

   
   if(lAreneType == ARENE1_TYPE)
   {
      if(lAreneID < 0 || lAreneID >= Arena1Master::GetNumberOfArene())
         return;
      if(!theApp.m_dwArenaSystem1[lAreneID])
      {
         CString strTmp;
         strTmp.Format(_STR( 15471 , user->self->GetLang() ),Arena1Master::GetAreneName(lAreneID));
         user->self->SendInfoMessage(strTmp ,CL_RED);
         return;
      }
      Arena1Master::SendArenaList(user,lAreneID);
   }
   else if(lAreneType == ARENE2_TYPE)
   {
      if(lAreneID < 0 || lAreneID >= Arena2Master::GetNumberOfArene())
         return;
      if(!theApp.m_dwArenaSystem2[lAreneID])
      {
         CString strTmp;
         strTmp.Format(_STR( 15471 , user->self->GetLang() ),Arena2Master::GetAreneName(lAreneID));
         user->self->SendInfoMessage(strTmp ,CL_RED);
         return;
      }
      Arena2Master::SendArenaList(user,lAreneID);
   }
   

   RQ_FOOTER( "RQFUNC_ARENA1_GetWaitPlayerList" );
}

void TFCMessagesHandler::RQFUNC_ARENA1_UpdatePlayStat(PACKET_FUNC_PROTOTYPE)
{
   RQ_HEADER;
   if(!user->self)
      return;

   long lAreneID;
   long lAreneType;
   msg->Get( (long  *)&lAreneID );		// ID
   msg->Get( (long  *)&lAreneType );	// Type

   if(lAreneType == ARENE1_TYPE)
   {
      if(lAreneID < 0 || lAreneID >= Arena1Master::GetNumberOfArene())
         return;
      if(!theApp.m_dwArenaSystem1[lAreneID])
      {
         CString strTmp;
         strTmp.Format(_STR( 15471 , user->self->GetLang() ),Arena1Master::GetAreneName(lAreneID));
         user->self->SendInfoMessage(strTmp ,CL_RED);
         return;
      }
      Arena1Master::SendArenaPlayStat(user,false,lAreneID);
   }
   else if(lAreneType == ARENE2_TYPE)
   {
      if(lAreneID < 0 || lAreneID >= Arena2Master::GetNumberOfArene())
         return;
      if(!theApp.m_dwArenaSystem2[lAreneID])
      {
         CString strTmp;
         strTmp.Format(_STR( 15471 , user->self->GetLang() ),Arena2Master::GetAreneName(lAreneID));
         user->self->SendInfoMessage(strTmp ,CL_RED);
         return;
      }
      Arena2Master::SendArenaPlayStat(user,false,lAreneID);
   }

   RQ_FOOTER( "RQFUNC_ARENA1_UpdatePlayStat" );
}

void TFCMessagesHandler::ExtractKeyworld(char *pstrMessage,char *pstrKWS, char *pstrKWE, CStringArray &aKeyworld)
{
   char *pCurrent  = NULL;
   char *pRetK     = NULL; 
   char *pRetE     = NULL; 
   char strT[1024];
   char strTK[1024];
   sprintf_s(strT,1024,pstrMessage);

   pCurrent = &strT[0];
   do
   {
      pRetK =  strstr(pCurrent,pstrKWS);
      if(pRetK)
      {
         pRetE =  strstr(pRetK,pstrKWE);
         if(pRetE)
         {
            pCurrent+=((pRetE-pCurrent)+1);
            sprintf_s(strTK,1024,pRetK);
            strTK[(pRetE -pRetK)+1] = 0x00;
            aKeyworld.Add(strTK);
         }
      }
   }while(pRetK);
}