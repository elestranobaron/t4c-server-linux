// ArenaClobal.h: interface for the Professions class.
//
//////////////////////////////////////////////////////////////////////
#pragma once

#include "TFC Server.h"

typedef struct _sCombatArenaLocation
{
   CString strZOneName;
   WorldPos wlTopLeft;
   WorldPos wlBottomRight;
   WorldPos wlRecallBlueStart;
   WorldPos wlRecallRedStart;
   WorldPos wlRecallBlueDead;
   WorldPos wlRecallRedDead;
   WorldPos wlItemPod1;
   WorldPos wlItemPod2;
   WorldPos wlItemPod3;
   WorldPos wlItemPod4;
   WorldPos wlItemPod5;

   int      iDescMsgID;
   int      iItemID1;
   int      iItemID2;
   int      iSettingsMinPlayer;
   int      iSettingsMaxPlayer;
   int      iSettingsMaxPoint;
   int      iSettingsMinuteMax;
   int      iSettingsWaitStartSec;
   int      iSettingsWaitDeathSec;
   int      iSettingsTakeItemSec;
}sCombatArenaLocation;


typedef struct _sArenaPlayerList
{
   Players *pPlayer;
   WorldPos wlOldLoc;
}sArenaPlayerList;

typedef struct _sArenaTakeItemList
{
   Unit    *pItemUnit;
   Players *pPlayer;
   UINT  uiDecompte;
}sArenaTakeItemList;


typedef struct _sArenaFlagCfg
{
   int iNbrMinPlayer;
   int iNbrMaxPlayer;
   int iNbrMaxPoint;
   int iNbrMaxMinute;
   int iNbrStartWaitTimeSec;
   int iNbrPlayWaitTimeSec;
   int iNbrPlayWaitDeathSec;
   int iNbrPlayWaitDeathMSec;
   int iNbrPlayTakeItemSec;
}sArenaFlagCfg;

typedef struct _sArenaFlagGame
{
   CPtrArray c_aTakeFlagList1;
   CPtrArray c_aTakeFlagList2;
   CPtrArray c_aPlayerList;
   int       c_iLastNbrSec;
   int       c_iLastNbrMin;
   int       c_iLevelMin;
   int       c_iLevelMax;
   int       c_AreneStatus;
   int       c_PointBTeam;
   int       c_PointRTeam;
   int       c_FlagBInHome;
   int       c_FlagRInHome;
   DWORD     c_dwStartTime;
   list< sCombatArenaLocation >::iterator c_itArene;
   sArenaFlagCfg c_ArenaCfg;
}sArenaFlagGame;

#define DEFAULT_TYPE -1
#define ARENE1_TYPE  0
#define ARENE2_TYPE  1