/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "VaporizeUnit.h"
#include "../Players.h"
#include "../T4CLog.h"

/******************************************************************************/
REGISTER_SPELL_EFFECT( VAPORIZE_UNIT, VaporizeUnit::NewFunc, VAPORIZE_UNIT_EFFECT, __noop );

/******************************************************************************/
VaporizeUnit::VaporizeUnit()
/******************************************************************************/
{
}
/******************************************************************************/
VaporizeUnit::~VaporizeUnit()
/******************************************************************************/
{
}
/******************************************************************************/
// Input parameter function
BOOL VaporizeUnit::InputParameter(
 CString csParam,   // The parameter's value
 WORD wParamID      // The parameter's ID.
)
/******************************************************************************/
{
    return TRUE;
}
/******************************************************************************/
// This effect annihilates the given unit, if its not a player.
void VaporizeUnit::CallEffect(SPELL_EFFECT_PROTOTYPE)
/******************************************************************************/
{
   TRACE("***VaporizeUnit\n");
    if( self->GetType() == U_PC && target != NULL )
	{
        Character *lpCharacter = static_cast< Character * >( self );
        Players   *lpPlayer = lpCharacter->GetPlayer();
        
        if( lpPlayer != NULL && lpPlayer->GetGodFlags() & GOD_CAN_SLAY_USER )
		{
            _LOG_GAMEOP
                LOG_SYSOP,
                "Gameop %s (%s) cast vaporize on unit named %s.",
                (LPCTSTR)lpCharacter->GetTrueName(),
                (LPCTSTR)lpPlayer->GetFullAccountName(),
                (LPCTSTR)target->GetName( _DEFAULT_LNG )
            LOG_

            target->VaporizeUnit();
        }
    }
}
/******************************************************************************/
// Factory.
SpellEffect *VaporizeUnit::NewFunc(LPSPELL_STRUCT lpSpell) // The spell structure.
/******************************************************************************/
{
    CREATE_EFFECT_HANDLE( VaporizeUnit, 0 )
}