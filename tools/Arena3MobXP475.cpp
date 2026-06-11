/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "StdAfx.h"
#include "Arena3MobXP475.H"

/******************************************************************************/
Arena3MobXP475::Arena3MobXP475()
/******************************************************************************/
{}
/******************************************************************************/
Arena3MobXP475::~Arena3MobXP475()
/******************************************************************************/
{}

extern NPCstructure::NPC ArenaMobXP475NPC;

/******************************************************************************/
void Arena3MobXP475::Create( void )
/******************************************************************************/
{
	npc = ( ArenaMobXP475NPC );
	SET_NPC_NAME( "[10962]Darkimpaler" );
	npc.InitialPos.X = 0;
	npc.InitialPos.Y = 0;
	npc.InitialPos.world = 0;
}
/******************************************************************************/
void Arena3MobXP475::OnPopup( UNIT_FUNC_PROTOTYPE )
/******************************************************************************/
{																	 
  	CastSpellSelf(__SPELL_MOB_ARENA_MAJOR_REGENERATION_SPELL) 	
	SimpleMonster::OnPopup( UNIT_FUNC_PARAM );
}
/******************************************************************************/
void Arena3MobXP475::OnDeath( UNIT_FUNC_PROTOTYPE )
/******************************************************************************/
{
    INIT_HANDLER
	if( target != NULL )
	{
		GiveItem(__OBJ_COLOSSEUM_TOKEN)
		PRIVATE_SYSTEM_MESSAGE(INTL( 10682, "You receive a battle token for your efforts."))
	}
    CLOSE_HANDLER

	CastSpellSelf(__SPELL_MOB_ARENA_LEVEL_SPELL)

	SimpleMonster::OnDeath( UNIT_FUNC_PARAM );
}
/******************************************************************************/
void Arena3MobXP475::OnDestroy( UNIT_FUNC_PROTOTYPE )
/******************************************************************************/
{
	IF(CheckGlobalFlag(__GLOBAL_FLAG_NUMBER_MONSTERS_IN_ARENA_3) > 0)
		GiveGlobalFlag(__GLOBAL_FLAG_NUMBER_MONSTERS_IN_ARENA_3, CheckGlobalFlag(__GLOBAL_FLAG_NUMBER_MONSTERS_IN_ARENA_3) - 1)
	ENDIF

	SimpleMonster::OnDestroy( UNIT_FUNC_PARAM );
}