/******************************************************************************
Modify for vs2008 (06/05/2009)
Add Profession by Nightmare (29/06/2009)
/******************************************************************************/
#ifdef _WIN32
#include <afx.h>
#else
#include <cstdlib>
typedef unsigned short WORD;
typedef unsigned char BYTE;
typedef unsigned int DWORD;
typedef void* LPVOID;
#define __declspec(x)
#endif

#ifndef __SHAREDSTRUCTURES_H
#define __SHAREDSTRUCTURES_H

#define __PLAYER		1
#define __CREATURE		2
#define __OBJECT		3

#define __SHOP_DATA		0x1
#define __TEACH_DATA	   0x2
#define __TRAIN_DATA	   0x3
#define __TEACH_DATAF	0x4
#define __POINTS_DATA	0x5


#define __BUY			0x1
// Returns a price but doesn't sell the item
#define __SELL			0x2

#define SAME_POS( wl_1, wl_2 ) (wl_1.X == wl_2.X && wl_1.Y == wl_2.Y && wl_1.world == wl_2.world)

/******************************************************************************/
struct WorldPos
{
	int X, Y;
	int world;

	bool operator== (const WorldPos &otherPos) const {
		return (X == otherPos.X && Y == otherPos.Y && world == otherPos.world);
	}
	bool AreInRange(const WorldPos &otherPos, int range) const {
		return ( world == otherPos.world && std::abs(X - otherPos.X) <= range && std::abs(Y - otherPos.Y) <= range );
	}
};
/******************************************************************************/
struct MonsterEncounter
{
	WORD wMinX, wMaxX;
	WORD wGroupID;
};
/******************************************************************************/
typedef struct _NPC_DATA
{
	BYTE DataID;
	LPVOID Data;
} NPC_DATA, *LPNPC_DATA;
/******************************************************************************/
typedef struct _TEACH_DATA
{
	WORD wSkillID;			// ID of the skill.
} TEACH_DATA, *LPTEACH_DATA;
/******************************************************************************/
typedef struct _TRAIN_DATA
{
	WORD wSkillID;			// ID of the skill.
	WORD wSkillPnts;		// Number of skill points to spend on skill.
} TRAIN_DATA, *LPTRAIN_DATA;
/******************************************************************************/
typedef struct _SHOP_DATA
{
	BYTE  Action;	// Either BUY or SELL
	WORD  Item;		// Item ID of the item (if buy)
	WORD  wQuantity;	// Quantity of items to buy.
	DWORD ID;		// ID of the item to sell (if sell)
} SHOP_DATA, *LPSHOP_DATA;
/******************************************************************************/
typedef struct _TEACH_DATAF{
	WORD wFormuleID;			// ID of the Formule.
} TEACH_DATAF, *LPTEACH_DATAF;
/******************************************************************************/


#define CANCEL_DEATH				0x01	// Cancels the death. Unit dying won't die.
#define DESTROY_UNIT_ON_DEATH		0x02	// Destroy this object even if death was cancelled.
#define SAVE_UNIT_ON_DEATH			0x04	// Don't remove this object from backpack or equipped.
#define DESTROY_BACKPACK			0x08	// Destroy backpack and equipped items.

/******************************************************************************/
typedef struct _DEATH_DATA
{
	BYTE bDeathType;
} DEATH_DATA, *LPDEATH_DATA;

#endif
