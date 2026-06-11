/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "StdAfx.h"
#include "Arena3MobXP250.H"

/******************************************************************************/
Arena3MobXP250::Arena3MobXP250()
/******************************************************************************/
{}
/******************************************************************************/
Arena3MobXP250::~Arena3MobXP250()
/******************************************************************************/
{}

extern NPCstructure::NPC ArenaMobXP250NPC;

/******************************************************************************/
void Arena3MobXP250::Create( void )
/******************************************************************************/
{
	npc = ( ArenaMobXP250NPC );
	SET_NPC_NAME( "[10952]Darkflyer" );
	npc.InitialPos.X = 0;
	npc.InitialPos.Y = 0;
	npc.InitialPos.world = 0;
}
/******************************************************************************/
void Arena3MobXP250::OnPopup( UNIT_FUNC_PROTOTYPE )
/******************************************************************************/
{																	 
  	CastSpellSelf(__SPELL_MOB_ARENA_MINOR_REGENERATION_SPELL) 	
	SimpleMonster::OnPopup( UNIT_FUNC_PARAM );
}
/******************************************************************************/
void Arena3MobXP250::OnDeath( UNIT_FUNC_PROTOTYPE )
/******************************************************************************/
{
    INIT_HANDLER
	if( target != NULL )
	{
		IF(rnd.roll(dice(1, 100)) <= (270 - (USER_LEVEL)))
			GiveItem(__OBJ_COLOSSEUM_TOKEN)
			PRIVATE_SYSTEM_MESSAGE(INTL( 10682, "You receive a battle token for your efforts."))
		ENDIF
	}
    CLOSE_HANDLER

	CastSpellSelf(__SPELL_MOB_ARENA_LEVEL_SPELL)

	SimpleMonster::OnDeath( UNIT_FUNC_PARAM );
}
/******************************************************************************/
void Arena3MobXP250::OnDestroy( UNIT_FUNC_PROTOTYPE )
/******************************************************************************/
{
	IF(CheckGlobalFlag(__GLOBAL_FLAG_NUMBER_MONSTERS_IN_ARENA_3) > 0)
		GiveGlobalFlag(__GLOBAL_FLAG_NUMBER_MONSTERS_IN_ARENA_3, CheckGlobalFlag(__GLOBAL_FLAG_NUMBER_MONSTERS_IN_ARENA_3) - 1)
	ENDIF

	SimpleMonster::OnDestroy( UNIT_FUNC_PARAM );
}