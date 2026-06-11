// CriticalStrike.cpp: implementation of the CriticalStrike class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "CriticalStrike.h"
#include "../TFC Server.h"
extern CTFCServerApp theApp;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CriticalStrike::CriticalStrike()
{
   //  #/Name            Class     Cost Skills Fr  Level Stats Requirements Class Pts
   //  --------          -----     ---- ------ --  ----- ------------------ ---------
   //12/Critical Strike: Warriors.  51  1- 200 AU    35  STR: 70; INT: 35.  200W.

   s_saAttrib.skLevel = 35;
   s_saAttrib.skAGI = 0;
   s_saAttrib.skSTR = 70;
   s_saAttrib.skEND = 0;
   s_saAttrib.skINT = 35;
   s_saAttrib.skWIS = 0;
   s_saAttrib.skWIL = 0;
   s_saAttrib.skLCK = 0;
   //ADD_REQUIRED_SKILL( __SKILL_MONSTER_LORE, 72 )
}

void CriticalStrike::Destroy( void )
{
   s_saAttrib.tlskSkillRequired.AnnihilateList();
}

LPSKILLPNTFUNC CriticalStrike::lpOnAddPnts = NULL;

//////////////////////////////////////////////////////////////////////////////////////////
int CriticalStrike::Func
//////////////////////////////////////////////////////////////////////////////////////////
// Critical strike main function
// 
(
 DWORD dwReason,			// Hook which was used to call the skill.
 Unit *self,				// Unit using the skill.
 Unit *medium,				// Unused.
 Unit *target,				// Target of attack.
 void *valueIN,				// LPATTACK_STRUCTURE, current blow.
 void *valueOUT,			// Unused.
 LPUSER_SKILL lpusUserSkill // Current skill strength of the user.
 )
 // Return: int, SKILL_* return parameter.
 //////////////////////////////////////////////////////////////////////////////////////////
{	  
   if(theApp.dwEquilibrageNewSkillEnable == 0)
   {
      return SKILL_NO_FEEDBACK;
   }

   //The player must have 1point a least:
   if (lpusUserSkill->GetSkillPnts( self ) <= 0) {
      return SKILL_FAILED;
   }



   // If skill was called for an attack
   if(dwReason & HOOK_ATTACK || dwReason & HOOK_SPELL_ATTACK)
   {
      double critzValue;
      int nSuccess;
      int iMaxDice;

      LPATTACK_STRUCTURE s_asBlow = (LPATTACK_STRUCTURE)valueIN;
      //calcule un % entre 0 a X  soit 0 == 0% et si X == 1000 == 10%
      nSuccess = (lpusUserSkill->GetSkillPnts( self )*100) / 150;

      if( rnd.roll( dice( 1, 10000 ) ) < nSuccess ) 
      {
         //calcule un maxmumum du dee soit baser sur 2000 skill == 100 % de crit
         // 1000 de skill == 0.5 * maximum de crit   50
         // 2000 de skill == 1.0 * maximum de crit  100
         // 2500 de skill == 1.25* maximum de crit  125 
         // 5000 de skill == 2.50* maximum de crit  250
         iMaxDice = (int)(lpusUserSkill->GetTrueSkillPnts()*100/2000); 
         //on calcule un % de critique de min 20% du maximum jusqua maximum...


         double dCritMin = ((iMaxDice/100.00)*20.00)/100.00;
         int iRndDice = rnd.roll(dice( 1, iMaxDice ));
         critzValue = (double)(iRndDice)/100.00;
         if(critzValue < dCritMin)
            critzValue = dCritMin;
         if (critzValue > 2.50)	
            critzValue = 2.50;

         double dOldStrike = s_asBlow->Strike;
         s_asBlow->Strike  = s_asBlow->Strike + (s_asBlow->Strike*critzValue);

         

         //iMaxDice = (int)(lpusUserSkill->GetTrueSkillPnts()/625);
         //critzValue = (double)(rnd.roll( dice( 1, 2*(iMaxDice-1) ) ) / 2.0);
         //critzValue += 1.5;0.

         // Correctif
         //if (critzValue > 3)	
         //   critzValue = 3;
         //s_asBlow->Strike *= critzValue;

         if( ((Character*)self)->GetGodFlags() & GOD_DEVELOPPER ) 
         {  
            CString csMessage;
            csMessage.Format( "[\"Critical\" \"Strike\"] increased damages : critzValue : {%f} Old: {%f}, New: {%lf}  Bonus :{%lf})", critzValue, dOldStrike, s_asBlow->Strike,dOldStrike*critzValue );
            self->SendSystemMessage( csMessage );
         }
      } 
      else 
         return SKILL_FAILED;
      
      return SKILL_SUCCESSFULL;
   }

   return SKILL_FAILED;
}