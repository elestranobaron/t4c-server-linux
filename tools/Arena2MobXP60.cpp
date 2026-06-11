/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "StdAfx.h"
#include "Arena2MobXP60.H"

/******************************************************************************/
Arena2MobXP60::Arena2MobXP60()
/******************************************************************************/
{}
/******************************************************************************/
Arena2MobXP60::~Arena2MobXP60()
/******************************************************************************/
{}

extern NPCstructure::NPC ArenaMobXP60NPC;

/******************************************************************************/
void Arena2MobXP60::Create( void )
/******************************************************************************/
{
	npc = ( ArenaMobXP60NPC );
	SET_NPC_NAME( "[10965]Battleworm" );
	npc.InitialPos.X = 0;
	npc.InitialPos.Y = 0;
	npc.InitialPos.world = 0;
}
/******************************************************************************/
void Arena2MobXP60::OnDeath( UNIT_FUNC_PROTOTYPE )
/******************************************************************************/
{
    INIT_HANDLER
	if( target != NULL )
	{
		IF(rnd.roll(dice(1, 100)) <= (90 - (USER_LEVEL)))
			GiveItem(__OBJ_COLOSSEUM_TOKEN)
			PRIVATE_SYSTEM_MESSAGE(INTL( 10682, "You receive a battle token for your efforts."))
		ENDIF
	}
    CLOSE_HANDLER

	CastSpellSelf(__SPELL_MOB_ARENA_LEVEL_SPELL)

	SimpleMonster::OnDeath( UNIT_FUNC_PARAM );
}
/******************************************************************************/
void Arena2MobXP60::OnDestroy( UNIT_FUNC_PROTOTYPE )
/******************************************************************************/
{
	IF(CheckGlobalFlag(__GLOBAL_FLAG_NUMBER_MONSTERS_IN_ARENA_2) > 0)
		GiveGlobalFlag(__GLOBAL_FLAG_NUMBER_MONSTERS_IN_ARENA_2, CheckGlobalFlag(__GLOBAL_FLAG_NUMBER_MONSTERS_IN_ARENA_2) - 1)
	ENDIF

	SimpleMonster::OnDestroy( UNIT_FUNC_PARAM );
}