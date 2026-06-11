// StunBlow.cpp: implementation of the StunBlow class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "StunBlow.h"
#include "..\TFC Server.h"

extern CTFCServerApp theApp;

StunBlow::StunBlow(){
   //  #/Name            Class     Cost Skills Fr  Level Stats Requirements Class Pts
   //  --------          -----     ---- ------ --  ----- ------------------ ---------
   // 7/Stun Blow:       Warriors.  05  5- 500 AU    03  AGI: 20; STR: 25.  25W.

   s_saAttrib.skLevel = 3;
   s_saAttrib.skAGI = 20;
   s_saAttrib.skSTR = 25;
   s_saAttrib.skEND = 0;
   s_saAttrib.skINT = 0;
   s_saAttrib.skWIS = 0;
   s_saAttrib.skWIL = 0;
   s_saAttrib.skLCK = 0;
   UnitEffectManager::RegisterEffect( EFFECT_STUN_BLOW, TimerStunRemovallCallback );

}

void StunBlow::Destroy( void ){
   s_saAttrib.tlskSkillRequired.AnnihilateList();
}

LPSKILLPNTFUNC StunBlow::lpOnAddPnts = NULL;

//////////////////////////////////////////////////////////////////////////////////////////
void StunBlow::TimerStunRemovallCallback
//////////////////////////////////////////////////////////////////////////////////////////
// Timer callback when stunblow should be removed.
// 
(
 EFFECT_FUNC_PROTOTYPE 
 )
 //////////////////////////////////////////////////////////////////////////////////////////
{		
   self->RemoveFlag(__FLAG_STUN);		
   TRACE("\r\n\r\nRemove stun!!\r\n\r\n");	
}

//////////////////////////////////////////////////////////////////////////////////////////
int StunBlow::Func
//////////////////////////////////////////////////////////////////////////////////////////
// Stunblow function.
// 
(
 DWORD dwReason,			// Hook which called this skill.
 Unit *self,				// Unit attacking
 Unit *medium,				// Unused.
 Unit *target,				// Unit attacked.
 void *valueIN,				// LPATTACK_STRUCTURE, the blow..
 void *valueOUT,			// Unused.
 LPUSER_SKILL lpusUserSkill // Skill strength
 )
 // Return: int, SKILL_SUCCESSFULL or SKILL_FAILED
 //////////////////////////////////////////////////////////////////////////////////////////
{
   // If skill got called by an attack hook
   if(dwReason == HOOK_ATTACK){

      // This skill is an OnAttack skill, so it receives an ATTACK_STRUCTURE as valueIN
      LPATTACK_STRUCTURE s_asBlow	= (LPATTACK_STRUCTURE)valueIN;

      int nStunSuccess = 0;

      if(theApp.dwEquilibrageSkillNewFormulaEnable == 0)
      {
         if( self->GetLevel() >= target->GetLevel() * 3 / 4 )
            nStunSuccess = lpusUserSkill->GetSkillPnts( self ) / 2;
         else
            nStunSuccess = lpusUserSkill->GetSkillPnts( self ) / 4;
      }
      else
      {
         // Skill / 2, par pallier de 20
         if( self->GetLevel() >= target->GetLevel() * 3 / 4 )
            nStunSuccess = (int)(lpusUserSkill->GetSkillPnts( self ) / 20) * 10;
         // Skill / 4, par pallier de 20
         else
            nStunSuccess = (int)(lpusUserSkill->GetSkillPnts( self ) / 20) * 5;
      }

      // If stun blow is successfull.
      if( rnd.roll( dice( 1, 100 ) ) < nStunSuccess )
      {
         // If strength vs endurance roll succeeds.
         if( rnd.roll( dice( 1, self->GetSTR() ) ) > rnd.roll( dice( 1, target->GetEND()*3 ) ) )
         //if( rnd.roll( dice( 1, self->GetSTR() ) ) > rnd.roll( dice( 1, target->GetEND() ) ) )
         {
            TRACE("\r\n\r\nStunned!\r\n\r\n");
            target->RemoveEffect( EFFECT_STUN_BLOW );
            target->SetFlag(__FLAG_STUN, (UINT)StunBlow::TimerStunRemovallCallback);

            DWORD timer = rnd.roll( dice( 1, 1000, 1000 ) );
            if( ((Character*)self)->GetGodFlags() & GOD_DEVELOPPER )
            {
               CString csMessage;
               csMessage.Format( "[\"Stunt\" \"Blow\"] Stunned for %dms (succes : %d)", timer, nStunSuccess);
               self->SendSystemMessage( csMessage );
            }

            CREATE_EFFECT(target, MSG_OnTimer, EFFECT_STUN_BLOW, TimerStunRemovallCallback, 
                           NULL, timer MILLISECONDS TDELAY,timer,__SKILL_STUN_BLOW,0);
            return SKILL_SUCCESSFULL;
         }            
      }
      return SKILL_FAILED;
   }
   return SKILL_FAILED;
}