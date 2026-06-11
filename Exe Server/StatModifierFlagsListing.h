/******************************************************************************
Modify for vs2008 (06/05/2009)
Add un packet de flag et comment ceux pas utiliser... by NightMare (29/06/2009)
/******************************************************************************/
#ifndef __STATMODIFIERFLAGSLISTING_H
#define __STATMODIFIERFLAGSLISTING_H

const BYTE _LAWFUL		= 0; // not aggressive at all
const BYTE _NEUTRAL		= 50; // 1/2 agressive
const BYTE _CHAOTIC		= 100; // always agressive



//#define __FLAG_NMS_SCROLL_XP_BONNUS	   	  20000 //obsolete
//#define __FLAG_NMS_DEATH_HLL      	   	  20001 //obsolete
//#define __FLAG_NMS_DEATH_HLL_CNT  	   	  20003 //obsolete
//#define __FLAG_PVP_CRIME_TIMES               20103 //obsolete
//#define __FLAG_XP_SCROLL_LO                  20105 //obsolete
//#define __FLAG_XP_SCROLL_HI                  20106 //obsolete
//#define __FLAG_IF_PLAYER_IS_GR               20111 //obsolete


#define __FLAG_NMS_BOUST_XP      	   	   20002
#define __FLAG_NMS_COLOR_HAIR    	   	   20004
#define __FLAG_NMS_PLAYER_CAN_PLUNDER        20007
#define __FLAG_NMS_CANT_ATTACK_OTHER_PLAYER  20008
#define __FLAG_NMS_PLAYER_DEATH_SEX          20009
#define __FLAG_NMS_PLAYER_DEATH	            20010 
#define __FLAG_NMS_PLAYER_DEATH_TIMER	      20011 
#define __FLAG_UNIT_COLOR			            20015
#define __FLAG_TEST_WORLD_SPELL	            20016 

#define __FLAG_STUN					20017
#define __FLAG_MEDITATING			20018
#define __FLAG_PRAYING				20019
#define __FLAG_DEATH_LOCATION		20020
//#define __FLAG_WARCRY_NOT_AFFECTED	20021
//#define __FLAG_TRAP_DETECTED		20022
#define __FLAG_EVASIVNESS			20023
#define __FLAG_BERSERK				20024
#define __FLAG_ARM_EXTENT			20025
//#define __FLAG_FIST_OF_ROCK			20026
#define __FLAG_FLY					20027
#define __FLAG_WALK_ON_WATER		20028
//#define __FLAG_NPC_TALKTO           20029
#define __FLAG_AIR_RESIST           20030
#define __FLAG_WATER_RESIST         20031
#define __FLAG_FIRE_RESIST          20032
#define __FLAG_EARTH_RESIST         20033
#define __FLAG_ROBBING              20034
//#define __FLAG_SEPARATING_GOLD      20035
#define __FLAG_SHOUT                20036
#define __FLAG_LIGHT_RESIST         20037
#define __FLAG_DARK_RESIST          20038
#define __FLAG_LIGHT_POWER          20039
#define __FLAG_DARK_POWER           20040

#define __FLAG_AIR_POWER            20041
#define __FLAG_WATER_POWER          20042
#define __FLAG_FIRE_POWER           20043
#define __FLAG_EARTH_POWER          20044

#define __FLAG_FIRST_AID_EXHAUST         20045
#define __FLAG_IMMOBILIZATION_EXHAUST    20046
#define __FLAG_PRIMALSCREAM_EXHAUST      20047

#define __FLAG_INVISIBILITY         20050
#define __FLAG_HIDDEN               20051
#define __FLAG_DETECT_INVISIBILITY  20052
#define __FLAG_DETECT_HIDDEN        20053
#define __FLAG_PEEKING			   	20054


#define __FLAG_DEATH_NUMBER		   20055
#define __FLAG_KILL_NUMBER			   20056
#define __FLAG_ORACLE_TOKEN		   20057 


#define __FLAG_MAJ01_REROLL_FREE    20080   //MAJ NMS 2011 05 23  
#define __FLAG_MAJ01_CREDIT_SPELL   20081



#define __FLAG_NMS_SPEED            20097
#define __FLAG_NMS_RUNESTONE_TIME   20098
#define __FLAG_NMS_EN_PRISON        20099
#define __FLAG_RPHRP_STATUS         20100
#define __FLAG_RPHRP_TIME           20101
#define __FLAG_RPHRP_BLOCK          20102

#define __FLAG_MODE_COMBAT_TIMES    20104
#define __FLAG_UNIT_COLOR_OLD			20107
#define __FLAG_INVISIBILITY2        20108
#define __FLAG_DUEL_WIN             20109
#define __FLAG_DUEL_LOSE            20110
#define __FLAG_NMS_DECHU            20112
#define __FLAG_NMS_SECACHER_SKILL   20113
#define __FLAG_NMS_PARADE_SKILL     20114
#define __FLAG_NMS_PILLER_SKILL     20115
#define __FLAG_NMS_RESURECT_SKILL   20116
#define __FLAG_NMS_VOL_SKILL        20117
#define __FLAG_NMS_COUPOEIL_SKILL   20118

//Syeteme banque avec interet.....
#define __FLAG_NMSBANK_TYPE_CDOMPTE         20119
#define __FLAG_NMSBANK_OR_EN_BANK           20120
#define __FLAG_NMSBANK_OR_MINIMUM_WEEKS     20121
#define __FLAG_NMSBANK_MAX_OR               20122
#define __FLAG_NMSBANK_MIN_OR               20123
#define __FLAG_NMSBANK_MAX_EMPRUNT          20124
#define __FLAG_NMSBANK_OR_EMPRUNT_NBR_WEEKS 20126
#define __FLAG_NMSBANK_NEXT_INTERET_TIME    20127

//Flag Arena GAME
#define __FLAG_ARENAGAME_TEAM                  20140

//Systeme scroll Xp Bonnus
#define __FLAG_SCROLL_XP_MANAGEMENT            20150 //0 = OFF, !0 = need to activate for this time delay
#define __FLAG_SCROLL_XP_TIMESTAMP             20151 //0 = OFF, !0 = Timestamp to stop using... 
#define __FLAG_XP_SCROLL_VALUE_LO              20152 //contain low 32 bits value
#define __FLAG_XP_SCROLL_VALUE_HI              20153 //contain hi 32 bits value
#define __FLAG_FORCE_LEVELUP_REROLL            20154 //contain hi 32 bits value
#define __FLAG_REROLL_BLOCK_LEVELUP            20155
#define __FLAG_NMS_TAG_DISPLAY_OVER_HEAD       20156
#define __FLAG_NMS_BOUST_GOLD	   	           20157
#define __FLAG_SCROLL_XP_MULTIPLICATEUR        20158 //100 == 100% 
#define __FLAG_SCROLL_OR_MANAGEMENT            20159 //0 = OFF, !0 = need to activate for this time delay
#define __FLAG_SCROLL_OR_TIMESTAMP             20160 //0 = OFF, !0 = Timestamp to stop using... 
#define __FLAG_OR_SCROLL_VALUE_LO              20161 //contain low 32 bits value
#define __FLAG_OR_SCROLL_VALUE_HI              20162 //contain hi 32 bits value
#define __FLAG_SCROLL_OR_MULTIPLICATEUR        20163 //100 == 100% 
#define __FLAG_FREE_PAID_SERVER_FLAGS          20164
#define __FLAG_PRISON_TIMESTAMP                20165 
#define __FLAG_DEATH_LOST_XP			           20170 // On place le nombre de pts d'XP perdus à la mort
#define __FLAG_DEATH_LOST_GOLD			        20171 // on place le nombre de pièces d'or perdues à la mort
#define __FLAG_REMOVE_TARGETTING               20172
#define __FLAG_QUETE_DE_VILLIS                 20173
#define __FLAG_MINIONS_DISPLAYED               20174
#define __FLAG_MINIONS_MONSTERID               20175
#define __FLAG_MINIONS_CALL_FLOOD_CNT          20176
#define __FLAG_GET_RETURN_LAST_POS_X           20177
#define __FLAG_GET_RETURN_LAST_POS_Y           20178
#define __FLAG_GET_RETURN_LAST_POS_W           20179
#define __FLAG_CONVERTED_TO_NEW_XP_CHART       20180
#define __FLAG_RESET_BOUST_EQUIP_POS           20181
#define __FLAG_RESET_BOUST_EQUIP_POS_USER_ONLY 20182
#define __FLAG_INTERACTION_RP                  20183
#define __FLAG_NMS_LAST_HONOR_TIME             20184
#define __FLAG_NMS_LAST_CRIME_TIME             20185
#define __FLAG_NMS_PVP_ROB_PLUNDER_CNT         20186
#define __FLAG_PLAYER_USE_NEW_CHEST            20187
#define __FLAG_PLAYER_ARENE_HAVE_FLAG          20188
#define __FLAG_PLAYER_ARENE_BLOCK_VALUE_FLAG   20189
#define __FLAG_PLAYER_ARENE_BLOCK_TIME_FLAG    20190
#define __FLAG_POISSON_AVRIL_ITEMS             20191
#define __FLAG_POINTS_RP_XP_EVENTS             20192
#define __FLAG_POINTS_RP_XP_EVENTS_TOTAL       20193
#define __FLAG_PJ_VS_MONSTER_FRIENDLY          20194




#endif
