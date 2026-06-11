/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "StdAfx.h"
#include "Arena3MobXP500.H"

/******************************************************************************/
Arena3MobXP500::Arena3MobXP500()
/******************************************************************************/
{}
/******************************************************************************/
Arena3MobXP500::~Arena3MobXP500()
/******************************************************************************/
{}

extern NPCstructure::NPC ArenaMobXP500NPC;

/******************************************************************************/
void Arena3MobXP500::Create( void )
/******************************************************************************/
{
	npc = ( ArenaMobXP500NPC );
	SET_NPC_NAME( "[10964]Drake" );
	npc.InitialPos.X = 0;
	npc.InitialPos.Y = 0;
	npc.InitialPos.world = 0;
}
/******************************************************************************/
void Arena3MobXP500::OnPopup( UNIT_FUNC_PROTOTYPE )
/******************************************************************************/
{																	 
  	CastSpellSelf(__SPELL_MOB_ARENA_MAJOR_REGENERATION_SPELL) 	
	SimpleMonster::OnPopup( UNIT_FUNC_PARAM );
}
/******************************************************************************/
void Arena3MobXP500::OnDeath( UNIT_FUNC_PROTOTYPE )
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
void Arena3MobXP500::OnDestroy( UNIT_FUNC_PROTOTYPE )
/******************************************************************************/
{
	IF(CheckGlobalFlag(__GLOBAL_FLAG_NUMBER_MONSTERS_IN_ARENA_3) > 0)
		GiveGlobalFlag(__GLOBAL_FLAG_NUMBER_MONSTERS_IN_ARENA_3, CheckGlobalFlag(__GLOBAL_FLAG_NUMBER_MONSTERS_IN_ARENA_3) - 1)
	ENDIF

	SimpleMonster::OnDestroy( UNIT_FUNC_PARAM );
}