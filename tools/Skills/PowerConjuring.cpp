// PowerConjuring.cpp: implementation of the PowerConjuring class.
//
// Permet de creer un système d'aleatoirisation des coups du mage.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "PowerConjuring.h"
#include "../TFC Server.h"
extern CTFCServerApp theApp;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PowerConjuring::PowerConjuring()
{
//  #/Name            Class     Cost Skills Fr  Level Stats Requirements Class Pts
//  --------          -----     ---- ------ --  ----- ------------------ ---------
// x/PowerConjuring:   MAGE.  10  5-1000 AU    35  int: 150           25W.
	
	s_saAttrib.skLevel = 35;
	s_saAttrib.skAGI = 0;
	s_saAttrib.skSTR = 0;
	s_saAttrib.skEND = 0;
	s_saAttrib.skINT = 150;
	s_saAttrib.skWIS = 75;
	s_saAttrib.skWIL = 0;
	s_saAttrib.skLCK = 0;
}

void PowerConjuring::Destroy( void )
{}

LPSKILLPNTFUNC PowerConjuring::lpOnAddPnts = NULL;

//////////////////////////////////////////////////////////////////////////////////////////
int PowerConjuring::Func
//////////////////////////////////////////////////////////////////////////////////////////
// Power conjuring function
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
   if(theApp.dwEquilibrageNewSkillEnable == 0)
   {
      return SKILL_NO_FEEDBACK;
   }

	//The player must have 1point a least:
	if (lpusUserSkill->GetSkillPnts( self ) <= 0)
   {
		return SKILL_FAILED;
	}

   // If function was called by an attack hook
   if(dwReason & HOOK_SPELL_ATTACK)
   {
      LPATTACK_STRUCTURE s_asBlow = (LPATTACK_STRUCTURE) valueIN;

      /*** v2 Kiwi*/
      int dPow = lpusUserSkill->GetSkillPnts( self );
      int nSuccess = dPow / 2;

      if( rnd.roll( dice( 1, 100 ) ) < nSuccess )
      {
         double oldDmg = s_asBlow->Strike;

         double rand   = rnd.roll( dice( 1, dPow/3 )) - (dPow/6);
		   double aleadmg = (s_asBlow->Strike * (rand/100));

		   s_asBlow->Strike += aleadmg;

         if( ((Character*)self)->GetGodFlags() & GOD_DEVELOPPER )
         {
            CString csMessage;
            csMessage.Format( "[\"Power conjuring\"] randomise damages. (Old: %lf, New: %lf (with a success of %d)) - %lf rand / %lf aleadmg", oldDmg, s_asBlow->Strike, nSuccess, rand ,aleadmg);
            self->SendSystemMessage( csMessage );
         }
      }
      else
      {
         return SKILL_FAILED;
      }
      return SKILL_SUCCESSFULL;
   }

   return SKILL_FAILED;
}