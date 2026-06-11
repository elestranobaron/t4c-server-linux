/******************************************************************************
Modify for vs2008 (06/05/2009)
/******************************************************************************/
#if !defined(AFX_NPCSTRUCTURE_H__8745E2D4_0F4C_11D1_BCDB_00E029058623__INCLUDED_)
#define AFX_NPCSTRUCTURE_H__8745E2D4_0F4C_11D1_BCDB_00E029058623__INCLUDED_

#ifndef WINVER
	#define WINVER 0x0501
#endif
 
#if _MSC_VER >= 1000
	#pragma once
#endif // _MSC_VER >= 1000

#include "StdAfx.h"
#include "Unit.h"
#include "random.h"
#include "MonsterStructure.h"
#include "Broadcast.h"
#include "StatModifierFlagsListing.h"
#include "TFC_MAIN.h"
#include "Objects.h"
#include "Creatures.h"
#include "ObjectListing.h"
#include "MonsterStructure.h"
#include "QuestFlagsListing.h"
#include "NPCmacroScriptLng.h"
#include "GameDefs.h"
#include "SpellListing.h"
#include "TFCTime.h"
#include "SkillListing.h"
#include "_item.h"

/*	Color Code

~<color><text>

ex.
  "Hi, this text is red -> ~rHi hello!!~~ woohoo!"
--------------------------
~~  DEFAULT COLOR
~r	RED
~b  BLUE
~c  CYAN
~y  YELLOW
~g  GREEN
~p  PURPLE
~w  WHITE
~e  GREY
--------------------------
 */


extern Random rnd;

//#define SET_NPC_NAME( __name )	npc.name = _strdup(__name);

//#define SET_NPC_NAME( __name )	if( npc.name ) delete npc.name;\
//                                 npc.name = new char[ strlen( __name ) + 1 ];\
//                                 strcpy_s( npc.name,strlen( __name ) + 1, __name );
#define SET_NPC_NAME( __name )	npc.name = new char[ strlen( __name ) + 1 ];\
                                 strcpy_s( npc.name, strlen( __name ) + 1,__name );


#define SET_RANGE( lox, loy, hix, hiy ) npc.wLoXrange = lox;\
    npc.wLoYrange = loy;\
    npc.wHiXrange = hix;\
    npc.wHiYrange = hiy;

typedef struct _NPCSCHEDULE{
	TFCTIME tTriggerTime;

	int			iBehavior;		// Behavior to instill at the time
	WorldPos	wlDestination;	// Where the NPC should urge :)
	Unit		*Target;		// new target of the unit

} NPCSCHEDULE, *LPNPCSCHEDULE;

/******************************************************************************/
class __declspec(dllexport) NPCstructure : /*public StaticFlags,*/ public BaseReferenceMessages
/******************************************************************************/
{
public:
	NPCstructure();
	virtual ~NPCstructure();
	
	// Here are implementated the default message handlings for the NPC.
	virtual void OnInitialise( UNIT_FUNC_PROTOTYPE );
	virtual void OnAttack( UNIT_FUNC_PROTOTYPE );
	virtual void OnAttacked( UNIT_FUNC_PROTOTYPE );
	virtual void OnHit( UNIT_FUNC_PROTOTYPE );
	virtual void OnServerInitialisation( UNIT_FUNC_PROTOTYPE, WORD wBaseReferenceID );
	virtual void OnNPCDataExchange( UNIT_FUNC_PROTOTYPE );
	virtual void OnGetUnitStructure( UNIT_FUNC_PROTOTYPE );
	virtual void OnQuerySchedule( UNIT_FUNC_PROTOTYPE );
    virtual void OnDeath( UNIT_FUNC_PROTOTYPE );
    virtual void OnAttackHit( UNIT_FUNC_PROTOTYPE );

	class __declspec(dllexport) NPC : public MonsterStructure{
	public:
		WORD BaseReferenceID;
		//WorldPos InitialPos;

		DWORD dwTextColor;		
		MonsterStructure *GetMonsterStructure( void );		
	} npc;

	TemplateList <NPCSCHEDULE> tlSchedule;
};

#endif // !defined(AFX_NPCSTRUCTURE_H__8745E2D4_0F4C_11D1_BCDB_00E029058623__INCLUDED_)
