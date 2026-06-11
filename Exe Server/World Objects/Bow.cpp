/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "Bow.h"

/******************************************************************************/
Bow::Bow()
/******************************************************************************/
{
	//AddFlag(__FLAG_EQUIP_POSITION, equip_where);
}
/******************************************************************************/
Bow::~Bow()
/******************************************************************************/
{}
/******************************************************************************/
// A bow attacking only deals exhaust, the damage is dealt in the RangedAttack function.
void Bow::OnAttack(UNIT_FUNC_PROTOTYPE)
/******************************************************************************/
{
    ObjectStructure::OnAttack( UNIT_FUNC_PARAM );
    medium->DealExhaust( weapon.cDealtExhaust.GetBoost( medium, target ), 0, 0 );
}
/******************************************************************************/
// When a bow gets attacked ( uh?! well yes, bows can have AC... )
void Bow::OnAttacked(UNIT_FUNC_PROTOTYPE)
/******************************************************************************/
{
	ObjectStructure::OnAttacked( UNIT_FUNC_PARAM );
	
	LPATTACK_STRUCTURE Blow = (LPATTACK_STRUCTURE)valueIN;
	if(Blow) Blow->Strike -= armor.AC;
}
/******************************************************************************/
ObjectStructure *Bow::CreateObject( void )
/******************************************************************************/
{
	return new Bow;
}
