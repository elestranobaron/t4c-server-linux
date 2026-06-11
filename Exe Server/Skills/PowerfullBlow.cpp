// PowerfullBlow.cpp: implementation of the PowerfullBlow class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "PowerfullBlow.h"
#include "../TFC Server.h"

extern CTFCServerApp theApp;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PowerfullBlow::PowerfullBlow()
{
   //  #/Name            Class     Cost Skills Fr  Level Stats Requirements Class Pts
   //  --------          -----     ---- ------ --  ----- ------------------ ---------
   // 8/Powerful Blow:   Warriors.  10  5-1000 AU    09  STR: 35.           25W.

   s_saAttrib.skLevel = 15;
   s_saAttrib.skAGI = 30;
   s_saAttrib.skSTR = 50;
   s_saAttrib.skEND = 0;
   s_saAttrib.skINT = 0;
   s_saAttrib.skWIS = 0;
   s_saAttrib.skWIL = 0;
   s_saAttrib.skLCK = 0;
}

void PowerfullBlow::Destroy( void )
{}

LPSKILLPNTFUNC PowerfullBlow::lpOnAddPnts = NULL;

//////////////////////////////////////////////////////////////////////////////////////////
int PowerfullBlow::Func
//////////////////////////////////////////////////////////////////////////////////////////
// Powerfull blow function
// 
(
 DWORD dwReason,			// Hook which made the skill function call.
 Unit *self,					// Unit attacking
 Unit *medium,				// Unused.
 Unit *target,				// Unit attacked.
 void *valueIN,				// LPATTACK_STRUCTURE, blow/strike
 void *valueOUT,			// Unused.
 LPUSER_SKILL lpusUserSkill // Skill strength.
 )
 // Return: int, SKILL_SUCCESSFULL or SKILL_FAILED
 //////////////////////////////////////////////////////////////////////////////////////////
{
   //The player must have 1point a least:
   if (lpusUserSkill->GetSkillPnts( self ) <= 0) {
      return SKILL_FAILED;
   }
   // If function was called by an attack hook
   if(dwReason & HOOK_ATTACK)
   {
      if(theApp.dwEquilibrageSkillNewFormulaEnable == 0)
      {
         LPATTACK_STRUCTURE s_asBlow = (LPATTACK_STRUCTURE)valueIN;

         int nSuccess = lpusUserSkill->GetSkillPnts( self ) / 2;

         if( rnd.roll( dice( 1, 100 ) ) < nSuccess )
         {
            double oldDmg = s_asBlow->Strike;
            double rand   = rnd.roll( dice( 1, 33 ) );
            s_asBlow->Strike *= 133;
            s_asBlow->Strike /= 100;
            if( ((Character*)self)->GetGodFlags() & GOD_DEVELOPPER )
            {
               CString csMessage;
               csMessage.Format( "[\"Powerfull\" \"Blow\"] increased damages. (Old: %lf, New: %lf (+%lf%%))", oldDmg, s_asBlow->Strike, rand );
               self->SendSystemMessage( csMessage );
            }
         }
         else 
         {
            return SKILL_FAILED;
         }

         return SKILL_SUCCESSFULL;
      }
      else
      {
         /**********************************************
         * Kiwi v2
         ***********************************************/
         LPATTACK_STRUCTURE s_asBlow = (LPATTACK_STRUCTURE)valueIN;


         int nSuccess = (int)(lpusUserSkill->GetSkillPnts( self ) / 2) ;
         int iBonuscomp =0;

         if (nSuccess > 75 )
         {
             iBonuscomp = nSuccess - 75;
             nSuccess = 75;
         }

         if( rnd.roll( dice( 1, 100 ) ) < nSuccess )
         {
            double oldDmg = s_asBlow->Strike;
            int iDegatsBonusPC = 32+rnd.roll( dice( 1, 1+iBonuscomp ) );
            int iDegats   = 100+iDegatsBonusPC;
            s_asBlow->Strike *= iDegats;
            s_asBlow->Strike /= 100;


            /* Moen// On crée le cooldown -  timer = temps de cooldown de l'arme * 1.33
            long round = TFCMAIN::GetRound(); // Compteur de tic.
            EXHAUST sExhaustBASE = self->GetExhaust(); // Exhaust de l'arme

            int iExaust = (sExhaustBASE.attack-round)*50;
            iExaust *= 133;
            iExaust /= 100;

            self->DealExhaust(iExaust,iExaust,iExaust);
            EXHAUST sExhaust = self->GetExhaust(); */

            if( ((Character*)self)->GetGodFlags() & GOD_DEVELOPPER )
            {
               CString csMessage;
               csMessage.Format( "[\"Powerfull\" \"Blow\"] increased damages. (Old: %lf, New: %lf (+%d %%))", oldDmg, s_asBlow->Strike, iDegatsBonusPC );
               self->SendSystemMessage( csMessage );


               /*
               CString csMessage;
               csMessage.Format( "[\"Powerfull\" \"Blow\" \"v2\"] increased damages. (Old: %lf, New: %lf )) succes : %d " , oldDmg, s_asBlow->Strike, nSuccess);
               self->SendSystemMessage( csMessage );
               csMessage.Format( "    cooldown arme : \"%d\"(%d ms) -- cooldown competence : \"%d\"(%d ms)",
                                 sExhaustBASE.attack-round, (sExhaustBASE.attack-round)*50,
                                 sExhaust.attack-round, (sExhaust.attack-round)*50);
               self->SendSystemMessage( csMessage );
               */
            }
         }
         else
         {
            return SKILL_FAILED;
         }

         return SKILL_SUCCESSFULL;
      }
   }

   return SKILL_FAILED;
}