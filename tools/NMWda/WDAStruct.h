// WDAStruct.h: interface for the CWDAUtils class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WDASTRUCT_H__3E00521F_3672_4C26_9EF5_63DC21DC5DF8__INCLUDED_)
#define AFX_WDASTRUCT_H__3E00521F_3672_4C26_9EF5_63DC21DC5DF8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Afxtempl.h"

typedef struct _SpellEffectParam
{
   int dwParam1;
   int dwParam2;
   char *pstrParam2;
}SpellEffectParam;

typedef struct _SpellEffectNM
{
   int dwEffect;
   int dwNbrEffectParam;
   SpellEffectParam *pSpellEffectP;
}SpellEffectNM;

typedef struct _sSpell
{
   int  dwID;
   int  dwElement;
   int  dwAreaRange;
	int  dwVisualEffect;
	int  dwRangeVisualEffect;
	int  dwTargetType;
	int  dwAttackType;
	int  dwMinWis;
	int  dwMinInt;
	int  dwMinLevel;
   int  dwMinStr;
   int  dwMinEnd;
   int  dwMinAgi;
   int  dwMinAttack;
   int  dwMinArchery;
   int  dwMinDodge;
   int  dwMinCS;
   int  dwIcon;
   char chLineOfSight;
   char chSkillExclu;
	char chPVPcheck;
   char chFlag;

   int dwNom;
   int dwMentalExhaust;
   int dwMovementExhaust;
   int dwAttackExhaust;
   int dwDuration;
	int dwTimerFrequency;
	int dwManaCost;
	int dwSuccessPercentage;
	int dwDescText;

   char *pstrNom;
   char *pstrMentalExhaust;
   char *pstrMovementExhaust;
   char *pstrAttackExhaust;
   char *pstrDuration;
	char *pstrTimerFrequency;
	char *pstrManaCost;
	char *pstrSuccessPercentage;
	char *pstrDescText;
   CArray <SpellEffectNM*,SpellEffectNM*> SpellE;
   int dwNbrSpellReq;
   int *pdwSpellReq;
}sSpell;

typedef struct _HivePod
{
   int dwX;
   int dwY;
   int dwZ;
}HivePod;

typedef struct _HiveLarvaGrp
{
   int dwLarveGrp;
   char *pstrlarvaGrp;
}HiveLarvaGrp;


typedef struct _sHives
{
   int dwMinEmergencyTime;
   int dwMaxEmergencyTime;
   int dwMaxChildren;
   int dwEmergencyRange;
   int dwLarva;
   char *pstrLarva;
   int dwSkinID;
   int dwNbrlarvaGrp;
   CArray <HiveLarvaGrp*,HiveLarvaGrp*> aLarvaGrp;
   int dwNbrPod;
   CArray <HivePod*,HivePod*> aHivePod;
}sHives;


typedef struct _ItemContainerI
{
   int dwNbrItemName;
   char *pstrItemName;
}ItemContaineri;


typedef struct _ItemContainerGrp
{
   CArray <ItemContaineri*,ItemContaineri*> aItemItem;
}ItemContainerGrp;

typedef struct _ItemBoost
{
   int dwBoost;
   int dwStat;
   int dwExhaust;
   char *pstrExhaust;
   int dwMinInt;
   int dwMinWis;
}ItemBoost;

typedef struct _ItemSpell
{
   int dwSpell;
   int dwInstilled;
   int dwLevel;
}ItemSpell;

typedef struct _sItems
{
   int dwItemName;
   char *pstrItemName;
   int dwBindedID;
   int dwStructID;
   int dwName;
   char *pstrName;
   int dwAppearance;
   int dwSellType;
   int dwEquipPos;
   int dwBuyFlagID;
   int dwSellPrice;
   int dwStrSellPrice;
   char *pstrSellPrice;
   int dwSize;
	double dArmorAC;
   double dParadePC;
   int dwArmorDodgeMalus;
   int dwArmorMinEnd;
   int dwWeaponDmgRool;
   char *pstrWeaponDmgRool;
   int dwWeaponAttack;
   int dwWeaponStr;
   int dwWeaponAgi;
   int dwLockKeyID;
   char *pstrLockKeyID;
   int dwLockDifficulty;
   int dwBookText;
   char *pstrBookText;
   int dwContainerGold;
   int dwContainerGlobalRespawn;
   int dwContainerUserRespawn;
   int dwExhaust;
   char *pstrExhaust;
   int dwRadiance;
   int dwCharges;
   int dwMinInt;
   int dwMinWis;
   int dwIntlID;
   int dwDropFlags;
   char  chUnique;
   int dwGmItemLocation;
   char *pstrGmItemLocation;
   char  chCanSummon;
   char  chFlag1;
   char  chFlag2;
   CArray <ItemContainerGrp*,ItemContainerGrp*> aItemContainer;
   CArray <ItemBoost*,ItemBoost*> aItemBoost;
   CArray <ItemSpell*,ItemSpell*> aItemSpell;
}sItems;

typedef struct _sItemsPod
{
   int dwItemName;
   char *pstrItemName;
   int dwX;
   int dwY;
   int dwZ;
}sItemsPod;

typedef struct _MapInfo
{
   short shID;
   short shLongeur;
   short shHauteur;
   char strMapName[50];
   BYTE *pMapData;
   BYTE *pMapDataUndo;
   BOOL  bMapUndoPossible;
}MapInfo;


typedef struct _MapBits
{
   unsigned Pixel1:4;
   unsigned Pixel0:4;
}MapBits;

typedef union _uMapBit
{
   BYTE chVal;
   MapBits mapBitval;
}uMapBit;

typedef union _uShortToChar
{
   BYTE chVal[2];
   short  shVal;
}uShortToChar;

typedef union _uIntToChar
{
   BYTE chVal[4];
   int  dwVal;
}uIntToChar;

typedef union _uDoubleToChar
{
   BYTE chVal[8];
   double  dVal;
}uDoubleToChar;



typedef struct _sLinkArea
{
   int dwSrcX;
   int dwSrcY;
   int dwSrcZ;
   int dwDesX;
   int dwDesY;
   int dwDesZ;
}sLinkArea;

typedef struct _sClan
{
   short shClan1;
   short shClan2;
   short shNiveaux;
}sClan;

typedef struct _sClanName
{
   short shClanIndex;
   int dwName;
   char *pstrName;
}sClanName;

typedef struct _sQuestFlag
{
   int dwFlagValue;
   int dwName;
   char *pstrName;
}sQuestFlag;

typedef struct _sItemMonstre
{
   int dwFlagValue;
   int dwFlagValue2;
   int dwName;
   char *pstrName;
}sItemMonstre;

typedef struct _sAttack
{
   int dwDmg;
   char *pstrDmg;
   int dwAttackSkill;
   int dwAttackPercentage;
   int dwAttackSpell;
   int dwAttackMinRange;
   int dwAttackMaxRange;
}sAttack;

typedef struct _sDeadFlagNM
{
   int dwFlag;
   int dwFlagValue;
   char chIncrement;
}sDeadFlagNM;

typedef struct _sDrop
{
   int dwItemID;
   double dDropPercentage;
}sDrop;


typedef struct _sCreature
{
   int dwBindedID;
   int dwNbrCreatureID;
   char*pstrCreatureID;
   int dwNbrName;
   char*pstrName;
   int dwEnd;
   int dwAgi;
   int dwInt;
   int dwVal1;
   int dwVal2;
   int dwVal3;
   int dwVal4;
   int dwAirResist;
   int dwEarthResist;
   int dwWaterResist;
   int dwFireResist;
   int dwDarkResist;
   int dwLightResist;
   int dwAirPower;
   int dwEarthPower;
   int dwWaterPower;
   int dwFirePower;
   int dwDarkPower;
   int dwLightPower;
   int dwLVL;
   int dwHP;
   int dwDodgeSkill;

   double dAc;
   
   int dwAppearance   ;
   int dwDressBody    ;
   int dwDressFeet    ;
   int dwDressGloves  ;
   int dwDressHelm    ;
   int dwDressLegs    ;
   int dwDressWeapon  ;
   int dwDressShield  ;
   int dwDressCape    ;
   int dwAggressivness;
   int dwClanID       ;
   int dwSpeed        ;
   double dXPperHit  ;
   double dXPperDeath;
   int  dwMinGiveGold;
   int  dwMaxGiveGold;
   char chCanAttack  ;
   char chChangeTargetAA  ;
   int  iFriendlyID;



   CArray <sAttack*,sAttack*> aAttack;
   CArray <sDeadFlagNM*,sDeadFlagNM*> aDeadFlag;
   CArray <sDrop*,sDrop*> aDrop;
}sCreature;

typedef struct _AppearanceItem
{
   int dwID;
   int dwNbrName;
   char *pstrName;
}AppearanceItem;

typedef struct _AppearanceMonster
{
   int dwID;
   int dwIDC;
   int dwNbrName;
   char *pstrName;
}AppearanceMonster;

typedef struct _ItemLocation
{
   int dwNbrName;
   char *pstrName;
}ItemLocation;

typedef struct _SpellIcon
{
   int dwID;
   int dwNbrName;
   char *pstrName;
}SpellIcon;

typedef struct _VisualEffect
{
   int dwID;
   int dwNbrName;
   char *pstrName;
}VisualEffect;

#endif // !defined(AFX_WDASTRUCT_H__3E00521F_3672_4C26_9EF5_63DC21DC5DF8__INCLUDED_)