// ArmorPenetration.cpp: implementation of the ArmorPenetration class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "ArmorPenetration.h"
#include "../TFC Server.h"
extern CTFCServerApp theApp;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ArmorPenetration::ArmorPenetration()
{
   s_saAttrib.skLevel = 25;
   s_saAttrib.skAGI = 40;
   s_saAttrib.skSTR = 75;
   s_saAttrib.skEND = 0;
   s_saAttrib.skINT = 30;
   s_saAttrib.skWIS = 0;
   s_saAttrib.skWIL = 0;
   s_saAttrib.skLCK = 0;
}

LPSKILLPNTFUNC ArmorPenetration::lpOnAddPnts = NULL;

//////////////////////////////////////////////////////////////////////////////////////////
int ArmorPenetration::Func
//////////////////////////////////////////////////////////////////////////////////////////
// ArmorPenetration's function
// -- Hook_Attack -- 
(
 DWORD dwReason,			// Hook which called this strike.
 Unit *self,				// Unused.
 Unit *medium,				// Unused.
 Unit *target,				// Unused.
 void *valueIN,				// Unused.
 void *valueOUT,			// Unused.
 LPUSER_SKILL lpusUserSkill // Skill strength.
 )
 // Return: int, SKILL_NO_FEEDBACK
 //////////////////////////////////////////////////////////////////////////////////////////
{

   //The player must have 1point a least:
   if (lpusUserSkill->GetSkillPnts( self ) <= 0) 
      return SKILL_FAILED;

   if( dwReason == HOOK_ATTACK )
   {

      if(theApp.dwEquilibrageSkillNewFormulaEnable == 0)
      {
         if (target != NULL && target->GetAC() >= 5000) 
            return SKILL_NO_FEEDBACK;

         LPATTACK_STRUCTURE s_asBlow = (LPATTACK_STRUCTURE)valueIN;

         int nSuccess = lpusUserSkill->GetSkillPnts( self ) / 2;

         if( rnd.roll( dice( 1, 100 ) ) < nSuccess )
         {     
            ///NMS Methode...
            double dAC	   = target->GetAC();
            double dTA     = lpusUserSkill->GetSkillPnts( self );
            double dNewAC  = lpusUserSkill->GetSkillPnts( self ) > 125 ?
               (double)lpusUserSkill->GetSkillPnts( self )/(lpusUserSkill->GetSkillPnts( self )+5) * dAC :
            (double)lpusUserSkill->GetSkillPnts( self )/((double)lpusUserSkill->GetSkillPnts( self )+13.2) * dAC;

            dNewAC = dNewAC > 0 ? dNewAC : 0;

            double dBoust = dTA<200?(dTA/200) * (s_asBlow->TrueStrike/3.05):(s_asBlow->TrueStrike/3.05);
            double dDam   = s_asBlow->TrueStrike + dNewAC;


            s_asBlow->Strike = dDam + dBoust;

            if( ((Character*)self)->GetGodFlags() & GOD_DEVELOPPER )
            {
               CString csMessage;
               csMessage.Format( "dNewAC = %f", dNewAC );
               self->SendSystemMessage( csMessage );
               csMessage.Format( "dBoust = %f", dBoust );
               self->SendSystemMessage( csMessage );
               csMessage.Format( "dDam = %f", dDam );
               self->SendSystemMessage( csMessage );
               csMessage.Format( "s_asBlow->Strike = %f", s_asBlow->Strike );
               self->SendSystemMessage( csMessage );
            }
         }
      }
      else
      {
         if (target != NULL && target->GetAC() >= 25000)
            return SKILL_NO_FEEDBACK;

         LPATTACK_STRUCTURE s_asBlow = (LPATTACK_STRUCTURE)valueIN;


         // Palier de 10, 5 par 5
         //Moen
         /*
         int nSuccess = (int)(lpusUserSkill->GetSkillPnts( self ) / 15) * 5;
         if( rnd.roll( dice( 1, 100 ) ) < nSuccess )
         {
            ///NMS Methode...
            double dAC	   = target->GetAC();
            double oldDmg = s_asBlow->Strike;
            int nPourcentAC = (int)(lpusUserSkill->GetSkillPnts( self ) / 10) * 5;
            double dACPenetrated = dAC * nPourcentAC / 100;
            double NewStrike = dACPenetrated + s_asBlow->Strike;
            s_asBlow->Strike = NewStrike;
            if( ((Character*)self)->GetGodFlags() & GOD_DEVELOPPER )
            {
               CString csMessage;
               csMessage.Format( "[\"Armor\" \"Penetration\" \"v2\"] increased damages. (Old: %lf, New: %lf ))", oldDmg, s_asBlow->Strike );
               self->SendSystemMessage( csMessage );
            }
         }
         */

         //Kiwi
         //int nSuccess = (int)(lpusUserSkill->GetSkillPnts( self ) / 2) ;
		 int nSuccess = lpusUserSkill->GetSkillPnts( self ) / 2;
		 
         int iBonuscomp =0;

         if (nSuccess > 75 )
         {
             iBonuscomp = nSuccess - 75;
             nSuccess = 75;
         }

         if( rnd.roll( dice( 1, 100 ) ) < nSuccess )
         {
             double oldDmg = s_asBlow->TrueStrike;
             double dAC	   = target->GetAC();
             double dTA    = lpusUserSkill->GetSkillPnts( self );
             double dNewAC  = lpusUserSkill->GetSkillPnts( self ) > 125 ?
                (double)lpusUserSkill->GetSkillPnts( self )/(lpusUserSkill->GetSkillPnts( self )+5) * dAC :
             (double)lpusUserSkill->GetSkillPnts( self )/((double)lpusUserSkill->GetSkillPnts( self )+13.2) * dAC;

             dNewAC = dNewAC > 0 ? dNewAC : 0;

             double dBoust = ((dTA+2*iBonuscomp)/200) * (s_asBlow->TrueStrike/3.05);
             double dDam   = s_asBlow->TrueStrike + dNewAC;


             //s_asBlow->Strike = dDam + dBoust;
			
			if(  (dDam + dBoust) >  s_asBlow->Strike)
				s_asBlow->Strike = dDam + dBoust;



            if( ((Character*)self)->GetGodFlags() & GOD_DEVELOPPER )
            {
               CString csMessage;
               csMessage.Format( "[\"Armor\" \"Penetration\" \"v2\"] increased damages. (Old: %lf, New: %lf ))", oldDmg, s_asBlow->Strike );
               self->SendSystemMessage( csMessage );
            }
         }
      }
   }
   return SKILL_NO_FEEDBACK;
}
