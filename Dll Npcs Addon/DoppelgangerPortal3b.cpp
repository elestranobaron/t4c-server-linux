/******************************************************************************
Modify for vs2008 (23/04/2009)
******************************************************************************/
#include "StdAfx.h"
#include "DoppelgangerPortal3b.h"

/******************************************************************************/
DoppelgangerPortal3b::DoppelgangerPortal3b()
/******************************************************************************/
{}
/******************************************************************************/
DoppelgangerPortal3b::~DoppelgangerPortal3b()
/******************************************************************************/
{}

extern NPCstructure::NPC PortalNPC;

/******************************************************************************/
void DoppelgangerPortal3b::Create( )
/******************************************************************************/
{
    npc = PortalNPC;
    SET_NPC_NAME(  "[12731]A radiating portal" );
    npc.InitialPos.X = 3043;
    npc.InitialPos.Y = 1316;
    npc.InitialPos.world = 1;
}
/******************************************************************************/
void DoppelgangerPortal3b::OnAttacked( UNIT_FUNC_PROTOTYPE )
/******************************************************************************/
{
}
/******************************************************************************/
void DoppelgangerPortal3b::OnInitialise( UNIT_FUNC_PROTOTYPE )
/******************************************************************************/
{
	NPCstructure::OnInitialise( UNIT_FUNC_PARAM );
	WorldPos wlPos = { 0,0,0 };
	self->SetDestination( wlPos );
	self->Do( nothing );
	self->SetCanMove( FALSE );
}
/******************************************************************************/
void DoppelgangerPortal3b::OnTalk( UNIT_FUNC_PROTOTYPE )
/******************************************************************************/
{
	InitTalk

		Begin
		""
		IF(IsInRange(4))
			PRIVATE_SYSTEM_MESSAGE(INTL( 12018, "You step through the portal and appear somewhere else."))
			CastSpellTarget(__SPELL_DOPPELGANGER_RETURN_TELEPORT_DISPEL)
			TELEPORT(2850, 177, 0)
		ELSE
			PRIVATE_SYSTEM_MESSAGE(INTL( 12014, "You must step closer to the portal to inspect it."))
		ENDIF
		BREAK

		Default
			INTL( 7353, "This is a bug, please report it.")

	EndTalk
}
