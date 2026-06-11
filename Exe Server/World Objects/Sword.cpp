/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "Sword.h"

/******************************************************************************/
Sword::Sword()
/******************************************************************************/
{}
/******************************************************************************/
Sword::~Sword()
/******************************************************************************/
{}

/******************************************************************************/
// When a sword attacks
void Sword::OnAttack(UNIT_FUNC_PROTOTYPE) // sword prototype 
/******************************************************************************/
{
	ObjectStructure::OnAttack( UNIT_FUNC_PARAM );
	
	LPATTACK_STRUCTURE Blow = (LPATTACK_STRUCTURE)valueIN;
	if(Blow) Blow->Strike += weapon.cDamage.GetBoost( medium, target );
   
    TRACE( "\nmedium=0x%x target=0x%x, attack=%u.", medium, target, (int)weapon.cDamage.GetBoost( medium, target ) );

    medium->DealExhaust( weapon.cDealtExhaust.GetBoost( medium, target ), 0, 0 );
}
/******************************************************************************/
ObjectStructure *Sword::CreateObject( void )
/******************************************************************************/
{
	return new Sword;
}
/******************************************************************************/
// When a swords gets attacked ( uh?! well yes, swords can have AC... )
void Sword::OnAttacked(UNIT_FUNC_PROTOTYPE)
/******************************************************************************/
{
	ObjectStructure::OnAttacked( UNIT_FUNC_PARAM );
	
	LPATTACK_STRUCTURE Blow = (LPATTACK_STRUCTURE)valueIN;
	if(Blow) Blow->Strike -= armor.AC;
}