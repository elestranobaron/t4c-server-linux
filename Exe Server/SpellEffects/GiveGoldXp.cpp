/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "GiveGoldXp.h"

/******************************************************************************/
// Register the spell effect
REGISTER_SPELL_EFFECT( GIVEGOLDXP, GiveGoldXp::NewFunc, GIVEGOLDXP_EFFECT, __noop );

/******************************************************************************/
GiveGoldXp::GiveGoldXp()
/******************************************************************************/
{
	bError = NOT_LOADED;	
   wGive = 0;
}
/******************************************************************************/
GiveGoldXp::~GiveGoldXp()
/******************************************************************************/
{
}
/******************************************************************************/
// Setup the parameters
BOOL GiveGoldXp::InputParameter( CString csParam, WORD wParamID )
/******************************************************************************/
{
	// The param ID
	const int GoldXp = 1;	// What has to be given?
	const int Qty = 2; // The quantity (boost formula)

	BOOL boReturn = TRUE;

	switch( wParamID )
	{
		case GoldXp:
		{
			// Gold
			if( csParam.CompareNoCase( "gold" ) == 0 )
			{
				wGive = GOLD;				
			}
			// XP
			else if( csParam.CompareNoCase( "xp" ) == 0 )
			{
				wGive = XP;
			}
			// Unknown
			else
			{
				boReturn = FALSE;
			}

			break;
		}
		case Qty:
		{
			// Set the formula
			if( Quantity.SetFormula( csParam ) == FALSE )
			{
				// if it fails
				boReturn = FALSE;
			}
			else

			break;
		}
	}

	return boReturn;
}
/******************************************************************************/
// Set the player gold or xp
void GiveGoldXp::CallEffect( SPELL_EFFECT_PROTOTYPE )
/******************************************************************************/
{
   TRACE("***GiveGoldXp\n");
	// Continue only if the spell has been correctly loaded
	if( bError = LOADED )
	{
		// And if the target exists (avoid any crash)
		if( target != NULL )
		{
			switch( wGive )
			{
				// Give the gold
				case GOLD:
				{
					GiveGoldFunc( Quantity.GetBoost( self, target ), target, true );
					break;
				}
				// Give the XP
				case XP:
				{
					int amount = Quantity.GetBoost( self, target );
					target->SetXP( target->GetXP() + amount );

					break;
				}
				// Give Nothing
				default:
				{
					break;
				}
			}
		}
	}
}
/******************************************************************************/
// Create a SpellEffect object.
SpellEffect *GiveGoldXp::NewFunc( LPSPELL_STRUCT lpSpell )
{
	CREATE_EFFECT_HANDLE( GiveGoldXp, 0 )
}