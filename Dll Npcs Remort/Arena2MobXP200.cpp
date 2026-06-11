/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "StdAfx.h"
#include "Arena2MobXP200.H"

/******************************************************************************/
Arena2MobXP200::Arena2MobXP200()
/******************************************************************************/
{}
/******************************************************************************/
Arena2MobXP200::~Arena2MobXP200()
/******************************************************************************/
{}

extern NPCstructure::NPC ArenaMobXP200NPC;

/******************************************************************************/
void Arena2MobXP200::Create( void )
/******************************************************************************/
{
	npc = ( ArenaMobXP200NPC );
	SET_NPC_NAME( "[10950]Shadowgoblin" );
	npc.InitialPos.X = 0;
	npc.InitialPos.Y = 0;
	npc.InitialPos.world = 0;
}
/******************************************************************************/
void Arena2MobXP200::OnPopup( UNIT_FUNC_PROTOTYPE )
/******************************************************************************/
{																	 
  	CastSpellSelf(__SPELL_MOB_ARENA_MINOR_REGENERATION_SPELL) 	
	SimpleMonster::OnPopup( UNIT_FUNC_PARAM );
}
/******************************************************************************/
void Arena2MobXP200::OnDeath( UNIT_FUNC_PROTOTYPE )
/******************************************************************************/
{
    INIT_HANDLER
	if( target != NULL )
	{
		IF(rnd.roll(dice(1, 100)) <= (230 - (USER_LEVEL)))
			GiveItem(__OBJ_COLOSSEUM_TOKEN)
			PRIVATE_SYSTEM_MESSAGE(INTL( 10682, "You receive a battle token for your efforts."))
		ENDIF
	}
    CLOSE_HANDLER

	CastSpellSelf(__SPELL_MOB_ARENA_LEVEL_SPELL)

	SimpleMonster::OnDeath( UNIT_FUNC_PARAM );
}
/******************************************************************************/
void Arena2MobXP200::OnDestroy( UNIT_FUNC_PROTOTYPE )
/******************************************************************************/
{
	IF(CheckGlobalFlag(__GLOBAL_FLAG_NUMBER_MONSTERS_IN_ARENA_2) > 0)
		GiveGlobalFlag(__GLOBAL_FLAG_NUMBER_MONSTERS_IN_ARENA_2, CheckGlobalFlag(__GLOBAL_FLAG_NUMBER_MONSTERS_IN_ARENA_2) - 1)
	ENDIF

	SimpleMonster::OnDestroy( UNIT_FUNC_PARAM );
}