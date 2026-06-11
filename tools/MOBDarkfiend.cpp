#include "StdAfx.h"
#include "MOBDarkfiend.h"

MOBDarkfiend::MOBDarkfiend()
{}

MOBDarkfiend::~MOBDarkfiend()
{}

void MOBDarkfiend::OnPopup( UNIT_FUNC_PROTOTYPE )
{
	CastSpellSelf( __SPELL_EFFECT_FLAK2_WITH_PURPLE_BALL )
	SimpleMonster::OnPopup( UNIT_FUNC_PARAM );
}

extern NPCstructure::NPC MOBDarkfiendNPC;

void MOBDarkfiend::Create()
{
	npc = MOBDarkfiendNPC;
	SET_NPC_NAME( "[10954]Darkfiend" );
	npc.InitialPos.X = 0;
	npc.InitialPos.Y = 0;
	npc.InitialPos.world = 0;
}

void MOBDarkfiend::OnDeath( UNIT_FUNC_PROTOTYPE )
{	
	SimpleMonster::OnDeath( UNIT_FUNC_PARAM );
}
