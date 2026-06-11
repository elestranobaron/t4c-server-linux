/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "IcySaber.h"

/******************************************************************************/
IcySaber::IcySaber()
/******************************************************************************/
{}
/******************************************************************************/
IcySaber::~IcySaber()
/******************************************************************************/
{}
/******************************************************************************/
// When a sword attacks
void IcySaber::OnAttack(UNIT_FUNC_PROTOTYPE) // sword prototype 
/******************************************************************************/
{
	LPATTACK_STRUCTURE Blow = (LPATTACK_STRUCTURE)valueIN;
	if(Blow)
	{		
		int nSabreStr = self->ViewFlag( __FLAG_ICY_SABER );
		Blow->Strike += ( rnd.roll( dice( ( nSabreStr / 5 ), 5 ) ) + ( nSabreStr / 4 ) );
	}
}
/******************************************************************************/
// If IcySaber is disturbed.
void IcySaber::OnDisturbed(UNIT_FUNC_PROTOTYPE)
/******************************************************************************/
{
	LPDWORD lpdwReason = ( LPDWORD )( valueIN );

	if( *lpdwReason == OBJECT_DISTURB_DROP || *lpdwReason == OBJECT_DISTURB_ROB  || *lpdwReason == OBJECT_DISTURB_GET )
	{
		self->SetMark( MARK_DELETION );

	}
}
/******************************************************************************/
ObjectStructure *IcySaber::CreateObject( void )
/******************************************************************************/
{
	return new IcySaber;
}