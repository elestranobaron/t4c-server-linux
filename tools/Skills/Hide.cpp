// Hide.cpp: implementation of the Hide class.
//
//////////////////////////////////////////////////////////////////////
#pragma hdrstop
#include "stdafx.h"
#include "Hide.h"
#include "..\ObjectListing.h"
#include "..\_item.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Hide::Hide()
{
	s_saAttrib.skLevel = 13;
	s_saAttrib.skAGI = 34;
	s_saAttrib.skSTR = 0;
	s_saAttrib.skEND = 0;
	s_saAttrib.skINT = 20;
	s_saAttrib.skWIS = 0;
	s_saAttrib.skWIL = 0;
	s_saAttrib.skLCK = 0;
}

void Hide::Destroy( void )
{
	s_saAttrib.tlskSkillRequired.AnnihilateList();
}

LPSKILLPNTFUNC Hide::lpOnAddPnts = NULL;

//////////////////////////////////////////////////////////////////////////////////////////
int Hide::Func
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
	if( !( dwReason & HOOK_USE_TARGET_UNIT ) ){
        self->SendSystemMessage( _STR( 7254, self->GetLang() ) );

        return SKILL_NO_FEEDBACK;
	}
    // If hide is not used on an object or on the player (self).
	if( !( target->GetType() == U_OBJECT || target == self ) ){
        self->SendSystemMessage( _STR( 7256, self->GetLang() ) );
        return SKILL_NO_FEEDBACK;
    }

    bool object = false;
    // If the target is an item
    if( target->GetType() == U_OBJECT ){
        object = true;
        _item *lpItem = NULL;

        // Fetch the target's item structure.
        target->SendUnitMessage( MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &lpItem );
        if( lpItem == NULL ){
            self->SendSystemMessage( _STR( 7256, self->GetLang() ) );
            return SKILL_NO_FEEDBACK;
        }
        // If the item cannot be hidden
        if( lpItem->dwDropFlags & CANNOT_HIDE ){
            self->SendSystemMessage( _STR( 7256, self->GetLang() ) );
            return SKILL_NO_FEEDBACK;
        }
    }
   
    WorldMap *wl = TFCMAIN::GetWorld( self->GetWL().world );

    if( wl == NULL ){
        if( object ){
            self->SendSystemMessage( _STR( 7254, self->GetLang() ) );            
        }else{
            self->SendSystemMessage( _STR( 7904, self->GetLang() ) );
        }

        return SKILL_NO_FEEDBACK;
    }

    // Get all units within the default range.
    TemplateList< Unit > *unitList = wl->GetLocalUnits( self->GetWL(), 30, FALSE );

    if( unitList == NULL ){
        if( object ){
            self->SendSystemMessage( _STR( 7254, self->GetLang() ) );            
        }else{
            self->SendSystemMessage( _STR( 7904, self->GetLang() ) );
        }
        return SKILL_NO_FEEDBACK;
    }
    
    DWORD hideSkill = lpusUserSkill->GetSkillPnts( target );

    int userHeuristic;
    if( unitList->NbObjects() == 0 ){
        userHeuristic = 0;
    }else{
        userHeuristic = ( unitList->NbObjects() - 1 ) / 2 + 1;
    }

    static Random rnd;
    if( userHeuristic == 0 || rnd( 1, 100 ) >= 
        (( 25 + hideSkill * 3 / 4 - target->GetRadiance() / 4 ) /   
        userHeuristic )
    ){
        if( object ){
            self->SendSystemMessage( _STR( 7254, self->GetLang() ) );            
        }else{
            self->SendSystemMessage( _STR( 7904, self->GetLang() ) );
        }

		if (unitList != NULL)
		{
			delete unitList;
			unitList = NULL;
		}
        return SKILL_NO_FEEDBACK;
    }


    target->SetFlag( __FLAG_HIDDEN, 1 );
  
    // Broadcast the hidden state to all in view users.
    unitList->ToHead();
    while( unitList->QueryNext() ){
        Unit *un = unitList->Object();

        // If the unit does not have detect invisible.
        if( un->ViewFlag( __FLAG_DETECT_HIDDEN ) == 0 ){
            // Make it dissappear.
            TFCPacket sending;

            sending << (RQ_SIZE)__EVENT_OBJECT_REMOVED;
            // Make the player dissappear
            sending << (char)0;
            sending << (long)target->GetID();

            un->SendPlayerMessage( sending );
        }
    }

    self->DealExhaust( 2000, 0, 2000 );

    if( target->GetType() == U_OBJECT ){
        self->SendSystemMessage( _STR( 7257, self->GetLang() ) );
    }else{
        self->SendSystemMessage( _STR( 7255, self->GetLang() ) );
        self->SetFlag(__FLAG_NMS_SECACHER_SKILL,self->ViewFlag(__FLAG_NMS_SECACHER_SKILL)+1);
    }

	if (unitList != NULL)
	{
		delete unitList;
		unitList = NULL;
	}

	return SKILL_NO_FEEDBACK;
}