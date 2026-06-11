// PickLock.cpp: implementation of the CPickLock class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "PickLock.h"
#include "../ObjectListing.h"
#include "../_item.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPickLock::CPickLock()
{
	s_saAttrib.skLevel = 12;
	s_saAttrib.skAGI = 40;
	s_saAttrib.skSTR = 0;
	s_saAttrib.skEND = 0;
	s_saAttrib.skINT = 0;
	s_saAttrib.skWIS = 0;
	s_saAttrib.skWIL = 0;
	s_saAttrib.skLCK = 0;
}

void CPickLock::Destroy( void )
{
	s_saAttrib.tlskSkillRequired.AnnihilateList();
}

LPSKILLPNTFUNC CPickLock::lpOnAddPnts = NULL;

//////////////////////////////////////////////////////////////////////////////////////////
int CPickLock::Func
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
	int nReturn = SKILL_PERSONNAL_FEEDBACK_FAILED;

	// this skill is used on locked objects only.
	if( dwReason & HOOK_USE_TARGET_UNIT ){
		if( target->GetType() == U_OBJECT ){
            if( self->GetType() == U_PC ){
                Character *ch = static_cast< Character * >( self );
                ch->StopAutoCombat();
            }

			// Get the item structure of the locked item.
			_item *sItem;
			target->SendUnitMessage( MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &sItem );

			// If item has a lock
			if( sItem->lock.key != 0 ){
                int nSuccess = lpusUserSkill->GetSkillPnts( self ) - sItem->lock.wDifficulty;

                if( rnd.roll( dice( 1, 100 ) ) < nSuccess ){
                    // Setup a BOOL variable to pass to the use_item function.
                    BOOL boLockPick = TRUE;

                    DWORD itemUsed = 0;
					target->SendUnitMessage(MSG_OnUse, target, self, self, &boLockPick, &itemUsed );

                }else{
                    self->SendSystemMessage( _STR( 11645, self->GetLang() ) );
                }
            // normally use the item. (without lock).
            }else{
                self->SendSystemMessage( _STR( 11532, self->GetLang() ) );
            }
		}
	}



	return nReturn;
}