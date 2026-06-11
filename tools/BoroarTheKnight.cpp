/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "StdAfx.h"
#include "BoroarTheKnight.h"

/******************************************************************************/
BoroarTheKnight::BoroarTheKnight()
/******************************************************************************/
{}
/******************************************************************************/
BoroarTheKnight::~BoroarTheKnight()
/******************************************************************************/
{}

extern NPCstructure::NPC Guard_Two;

/******************************************************************************/
void BoroarTheKnight::Create( void )
/******************************************************************************/
{
    npc = Guard_Two;
	SET_NPC_NAME( "[2996]Boroar the Knight" );
    npc.InitialPos.X = 1799;
    npc.InitialPos.Y = 1252;
	npc.InitialPos.world = 0;
}

/******************************************************************************/
void BoroarTheKnight::OnTalk( UNIT_FUNC_PROTOTYPE )
/******************************************************************************/
{
	CONSTANT POTIONS = 1

	InitTalk

		Begin
			INTL( 2197, "... Arrr... Hel... p... me! Skel... t... ons... A bright light... Arrrgh!")

		Command5(INTL( 518, "BYE"),INTL( 517, "FAREWELL"),INTL( 519, "LEAVE"),INTL( 521, "EXIT"),INTL( 520, "QUIT"))
			INTL( 2198, "he.....l..p!") BREAK

		Default
			INTL( 2199, "Aaa... AAAhh.... Hh...Ah... Hh... Hhh...") BREAK

	EndTalk
}
