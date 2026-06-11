// Sneak.cpp: implementation of the Sneak class.
//
//////////////////////////////////////////////////////////////////////

#pragma hdrstop
#include "stdafx.h"
#include "Sneak.h"
#include "../TFC_MAIN.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Sneak::Sneak()
{
	s_saAttrib.skLevel = 24;
	s_saAttrib.skAGI = 75;
	s_saAttrib.skSTR = 0;
	s_saAttrib.skEND = 0;
	s_saAttrib.skINT = 0;
	s_saAttrib.skWIS = 0;
	s_saAttrib.skWIL = 0;
	s_saAttrib.skLCK = 0;
}

void Sneak::Destroy( void )
{
	s_saAttrib.tlskSkillRequired.AnnihilateList();
}

LPSKILLPNTFUNC Sneak::lpOnAddPnts = NULL;

//////////////////////////////////////////////////////////////////////////////////////////
int Sneak::Func
//////////////////////////////////////////////////////////////////////////////////////////
// Powerfull blow function
// 
(
 DWORD dwReason,			// Hook which made the skill function call.
 Unit *self,				// Unit attacking
 Unit *medium,				// Unused.
 Unit *target,				// Unit attacked.
 void *valueIN,				// LPATTACK_STRUCTURE, blow/strike
 void *valueOUT,			// Unused.
 LPUSER_SKILL lpusUserSkill // Skill strength.
)
// Return: int, SKILL_SUCCESSFULL or SKILL_FAILED
//////////////////////////////////////////////////////////////////////////////////////////
{	
	// This skill is only used when moving and hidden.
	if( !( dwReason & HOOK_MOVE ) || self->ViewFlag( __FLAG_HIDDEN ) == 0 )	
	{
        return SKILL_NO_FEEDBACK;
	}
	
    WorldMap *wl = TFCMAIN::GetWorld( self->GetWL().world );

    if( wl == NULL )
	{
        return SKILL_NO_FEEDBACK;
    }
	
    // Get all units within the default range.
    TemplateList< Unit > *unitList = wl->GetLocalUnits( self->GetWL(), 30, FALSE );

    if( unitList == NULL )
	{
        return SKILL_NO_FEEDBACK;
    }
	
    DWORD sneakSkill = lpusUserSkill->GetSkillPnts( self );
    int userHeuristic;
    if( unitList->NbObjects() == 0 )
	{
        userHeuristic = 0;
    }else
	{
        userHeuristic = ( unitList->NbObjects() - 1 ) * 10;   
    }	

    // If sneak succeeds
	int nRand = rnd.roll( dice( 1, 100 ) );
	int nSuccess = sneakSkill + self->GetAGI() / 6 - userHeuristic;		

	if( nRand < nSuccess )
	{
        // Nothing happens.		
		if (unitList != NULL)
		{
			delete unitList;
			unitList = NULL;
		}
        return SKILL_NO_FEEDBACK;
    }

    // Unhide the unit.
    self->Unhide();

	if (unitList != NULL)
	{
		delete unitList;
		unitList = NULL;
	}

	return SKILL_NO_FEEDBACK;
}