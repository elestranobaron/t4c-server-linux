// Parry.cpp: implementation of the Parry class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "Parry.h"
#include "../TFC Server.h"

extern CTFCServerApp theApp;

//static BoostFormula bfSuccess;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
Parry::Parry()
{
   //  #/Name            Class     Cost Skills Fr  Level Stats Requirements Class Pts
   //  --------          -----     ---- ------ --  ----- ------------------ ---------
   // 4/Meditate:        Mages.     26  1- 500 WU    12  INT: 40.           25M.

   // Setup the formula for the success rate :)
   //bfSuccess.SetFormula("( (1 - (0.5 * self.weight/self.maxweight)) - (1 - if(self.hp < self.maxhp? (self.hp/self.maxhp)/4 : 1))) * (0.5+if(global.hour>=7? if(global.hour<=19?0.5:0):0))");

   s_saAttrib.skLevel = 10;
   s_saAttrib.skAGI = 30;
   s_saAttrib.skSTR = 0;
   s_saAttrib.skEND = 0;
   s_saAttrib.skINT = 20;
   s_saAttrib.skWIS = 0;
   s_saAttrib.skWIL = 0;
   s_saAttrib.skLCK = 0;
}

void Parry::Destroy( void )
{

}

//////////////////////////////////////////////////////////////////////////////////////////
void Parry::OnAddPoints
//////////////////////////////////////////////////////////////////////////////////////////
// Function which will add parry points
// 
(
 Unit *lpuTrained,			// Unit that trained.
 LPUSER_SKILL lpUserSkill,	// Skill's current points.
 DWORD dwNbPoints			// New skill points.
 )
 //////////////////////////////////////////////////////////////////////////////////////////
{

}

LPSKILLPNTFUNC Parry::lpOnAddPnts = OnAddPoints;

//////////////////////////////////////////////////////////////////////////////////////////
int Parry::Func
//////////////////////////////////////////////////////////////////////////////////////////
// Parry's function, does nothing
// -- Hook_None -- 
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
   // Added an option to enable servers to disable it in case it turns out to be buggy or unbalanced.
   // This option *will* be removed on next releases
   if (theApp.dwDebugSkillParryDisabled != 0) 
      return SKILL_NO_FEEDBACK;

   //The player must have 1point a least:
   if (lpusUserSkill->GetSkillPnts( self ) <= 0) {
      return SKILL_FAILED;
   }
   // Player got attacked?
   if( dwReason == HOOK_HIT )
   {
      if(theApp.dwEquilibrageSkillNewFormulaEnable == 0)
      {
         LPATTACK_STRUCTURE s_asBlow = (LPATTACK_STRUCTURE)valueIN;

         // NOTE: When attack is a spell, lDodgeSkill will be 0. In this case, no parry :)
         if (s_asBlow->lDodgeSkill == 0)
            return SKILL_NO_FEEDBACK;

         double nSuccess = ((double)lpusUserSkill->GetSkillPnts( self)/(lpusUserSkill->GetSkillPnts( self )+50))*10+((double)lpusUserSkill->GetSkillPnts( self )*self->GetSTR()/3000);		

         // If the % is negative, set it to 0 to avoid any problems
         if( nSuccess < 0 )
            nSuccess = 0;

         else if( nSuccess > 25 )
            nSuccess = 25;

         // Roll the dice! And... Should we parry?
         if( rnd.roll( dice( 1, 100 ) ) < nSuccess )
         {						
            // You can only deflect an attack if you have something equiped on hands!
            Unit **equipment = ((Character*)self)->GetEquipment();
            if (equipment[Character::weapon_right] != NULL)
            {
               // Nullifie damage.
               s_asBlow->Strike = 0;

               //Show a tiny effect :)
               Broadcast::BCSpellEffect( self->GetWL(), _DEFAULT_RANGE,30009, self->GetID(), 
                                         0, self->GetWL(),self->GetWL(),GetNextGlobalEffectID(),0);

               self->SetFlag(__FLAG_NMS_PARADE_SKILL,self->ViewFlag(__FLAG_NMS_PARADE_SKILL)+1);

               if( ((Character*)self)->GetGodFlags() & GOD_DEVELOPPER )
               {
                  self->SendSystemMessage("Attack deflected by your parry skill.");
               }
            }

         }
      }
      else
      {

         /**********************************************
         * Moen_OK (09/04/2010) -+-+-+-+ SKILL V2 +-+-+-+-
         ***********************************************/

         /****
         * Formule :
         * Pour calculer la parade on prend en compte deux valeurs, les skills points de l'utilisateur et les points de parade des items
         * 
         * Disons qu'à 100% + 200 skill points tu pares un coup sur 4. (1/4)
         * Un joueur ayant 200 skillpoints (100 pur + 100 boost) et qui porte une dague (10%) + focus(0%) ne paradera que 10% de la valeur qu'il devrait faire
         * Ce joueur paradera alors 1 coup sur 40 (1/40)
         * 
         * SkillSuccess = 5/40 * skillpts (5/40 = 1/8, mais on augmente pour les erreurs d'arrondie)
         * Success = SkillSuccess * ItemPTS /100
         */

         double nSuccess = 0;
         double nSkillSuccess = 0;

         LPATTACK_STRUCTURE s_asBlow = (LPATTACK_STRUCTURE)valueIN;

         // NOTE: When attack is a spell, lDodgeSkill will be 0. In this case, no parry :)
         if (s_asBlow->lDodgeSkill == 0)
            return SKILL_NO_FEEDBACK;

         //int nSuccess = lpusUserSkill->GetSkillPnts( self ) / 4 * bfSuccess.GetBoost(self);				
         nSkillSuccess = ((double)(lpusUserSkill->GetSkillPnts( self )* 5 / 40));

         // If the % is negative, set it to 0 to avoid any problems
         if( nSkillSuccess < 0 )
            nSkillSuccess = 0;

         // We get the user's equipment
         Unit **equipment = ((Character*)self)->GetEquipment();

         // You can only deflect an attack if you have something equiped on hands (Left and/or right), the Parry's point must be > 0
         double dblParryPC = 0;

         if( equipment[ Character::weapon_right ] )
            dblParryPC += equipment[ Character::weapon_right ]->GetParryPC();
         if( equipment[ Character::weapon_right ] != equipment[ Character::weapon_left ] && equipment[ Character::weapon_left ] != NULL ) 
            dblParryPC += equipment[ Character::weapon_left ]->GetParryPC();

         // You can only deflect an attack if you have something equiped on hands (Left and/or right) with a parry's item points greather than 0
         if( dblParryPC > 0 ) 
         {
            //We compute the succes with the Skill success and the items points " SkillSucces * ItemPts % "
            nSuccess = nSkillSuccess * dblParryPC  / 100;

            // Roll the dice! And... Should we parry?
            if( rnd.roll( dice( 1, 100 ) ) < nSuccess ) 
            {
               // Nullifie damage.
               s_asBlow->Strike = 0;

               //Show a tiny effect :)
               Broadcast::BCSpellEffect( self->GetWL(), 30, 30009, self->GetID(), 0, 
                  self->GetWL(),self->GetWL(),GetNextGlobalEffectID(),0);

               self->SetFlag(__FLAG_NMS_PARADE_SKILL,self->ViewFlag(__FLAG_NMS_PARADE_SKILL)+1);

               if( ((Character*)self)->GetGodFlags() & GOD_DEVELOPPER )
               {
                  CString csMessage;
                  csMessage.Format( "[\"Parry\" \"v2\"] Attack deflected by your parry skill. (nSkillSuccess: {%lf}, nSuccess: {%lf} ))", nSkillSuccess, nSuccess);
                  self->SendSystemMessage( csMessage );
               }
            } // end : rnd.roll( dice( 1, 100 )
         } // end : dblParryPC > 0

      } // end : skill V2 formula
   }

   return SKILL_NO_FEEDBACK;
}