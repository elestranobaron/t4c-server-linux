// Search.cpp: implementation of the Search class.
//
//////////////////////////////////////////////////////////////////////
#pragma hdrstop

#include "stdafx.h"
#include "Search.h"
#include "../TFC_MAIN.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Search::Search()
{
	s_saAttrib.skLevel = 20;
	s_saAttrib.skAGI = 0;
	s_saAttrib.skSTR = 0;
	s_saAttrib.skEND = 0;
	s_saAttrib.skINT = 25;
	s_saAttrib.skWIS = 0;
	s_saAttrib.skWIL = 0;
	s_saAttrib.skLCK = 0;
}


void Search::Destroy( void )
{
	s_saAttrib.tlskSkillRequired.AnnihilateList();
}

LPSKILLPNTFUNC Search::lpOnAddPnts = NULL;

//////////////////////////////////////////////////////////////////////////////////////////
int Search::Func
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
	// this skill is used on locked objects only.
	if( !( dwReason & HOOK_USE ) ){
        self->SendSystemMessage( _STR( 7259, self->GetLang() ) );
        return SKILL_NO_FEEDBACK;
	}
   
    WorldMap *wl = TFCMAIN::GetWorld( self->GetWL().world );

    if( wl == NULL ){
        self->SendSystemMessage( _STR( 7259, self->GetLang() ) );
        return SKILL_NO_FEEDBACK;
    }

    EXHAUST sExhaust = self->GetExhaust();

    // If player is exhausted.
    if( sExhaust.move > TFCMAIN::GetRound() || sExhaust.attack > TFCMAIN::GetRound() ){
        self->SendSystemMessage( _STR( 2776, self->GetLang() ) );
        return SKILL_NO_FEEDBACK;
    }


    // Get all units within the default range.
    TemplateList< Unit > *unitList = wl->GetLocalUnits( self->GetWL(), 30, FALSE );

    if( unitList == NULL ){
        self->SendSystemMessage( _STR( 7259, self->GetLang() ) );
        return SKILL_NO_FEEDBACK;
    }
    

    DWORD searchSkill = lpusUserSkill->GetSkillPnts( self );

    bool found = false;
    // Search for hidden units within the surrounding area.
    unitList->ToHead();
    while( unitList->QueryNext() ){
        Unit *un = unitList->Object();

        // Do not search for ourselves.
        if( un != self && un->PickLock() ){
            // If the unit is hidden.
            if( un->ViewFlag( __FLAG_HIDDEN ) != 0 ){

                // If search succeeds on this unit.
                static Random rnd;
                if( rnd( 1, 100 ) < ( searchSkill / 3 + self->GetINT() / 10 ) ){
                    // Unhide it.
                    un->Unhide();

                    found = true;
                }
            }
            un->Unlock();
        }
    }

    self->DealExhaust( 3000, 3000, 3000 );

    if( found ){
        self->SendSystemMessage( _STR( 7258, self->GetLang() ) );
    }else{
        self->SendSystemMessage( _STR( 7259, self->GetLang() ) );
    }

	if (unitList != NULL)
	{
		delete unitList;
		unitList = NULL;
	}

	return SKILL_NO_FEEDBACK;
}