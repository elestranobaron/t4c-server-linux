
// Resurect.cpp: implementation of the Resurect class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "Immobilization.h"
#include "../GameDefs.h"
#include "../IntlText.h"
#include "../ObjectListing.h"
#include "../T4CLog.h"
#include "../TFC Server.h"
extern CTFCServerApp theApp;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Immobilization::Immobilization()
{
   s_saAttrib.skLevel = 51;
   s_saAttrib.skAGI = 47;
   s_saAttrib.skSTR = 56;
   s_saAttrib.skEND = 0;
   s_saAttrib.skINT = 0;
   s_saAttrib.skWIS = 0;
   s_saAttrib.skWIL = 0;
   s_saAttrib.skLCK = 0;

   UnitEffectManager::RegisterEffect( EFFECT_IMMOBILIZATION, ImmobilizationCallback );
}

//////////////////////////////////////////////////////////////////////////////////////////
void Immobilization::Destroy( void )
//////////////////////////////////////////////////////////////////////////////////////////
//  Destroys peek.
// 
//////////////////////////////////////////////////////////////////////////////////////////
{
}
LPSKILLPNTFUNC Immobilization::lpOnAddPnts = NULL;

#define IMMOBILIZATION_SKILL   lpusUserSkill->GetSkillPnts( self )


//////////////////////////////////////////////////////////////////////////////////////////
void Immobilization::ImmobilizationCallback
//////////////////////////////////////////////////////////////////////////////////////////
// Timer callback when stunblow should be removed.
// 
(
 EFFECT_FUNC_PROTOTYPE 
 )
 //////////////////////////////////////////////////////////////////////////////////////////
{	

   if( wMessageID == MSG_OnTimer )
   {
      if ( self->GetImmobilizationCycle() != 0) 
      {
         // On diminue le cycle immob
         self->SetImmobilizationCycle(self->GetImmobilizationCycle() - 1);

         CREATE_EFFECT( self, 
                        MSG_OnTimer, 
                        EFFECT_IMMOBILIZATION, 
                        Immobilization::ImmobilizationCallback, 
                        NULL, 
						SKILL_IMMOB_CYCLE_TIME MILLISECONDS TDELAY, // Timer frequency
                        SKILL_IMMOB_CYCLE_TIME MILLISECONDS TDELAY, // Duration
                        __SKILL_IMMOBILIZATION,
                        0 );
         // Exhaust de mouvement sur la cible
         self->DealExhaust(0,0,SKILL_IMMOB_TARGET_EXHAUST);
      }
   }
}


//////////////////////////////////////////////////////////////////////////////////////////
void Immobilization::ImmobilizationBoustRemovalCallback(EFFECT_FUNC_PROTOTYPE )

{	
   self->RemoveBoost( BOOST_SKILL_IMMOBILIZATION_DODGE);
}


//////////////////////////////////////////////////////////////////////////////////////////
void Immobilization::ExhaustRemovallCallback
//////////////////////////////////////////////////////////////////////////////////////////
// Timer callback when immobilisation can be re-used
// 
(
 EFFECT_FUNC_PROTOTYPE 
 )
 //////////////////////////////////////////////////////////////////////////////////////////
{		
   self->RemoveFlag( __FLAG_IMMOBILIZATION_EXHAUST );
}

//////////////////////////////////////////////////////////////////////////////////////////
int Immobilization::Func
//////////////////////////////////////////////////////////////////////////////////////////
// Critical strike main function
// 
(
 DWORD dwReason,			// Hook which was used to call the skill.
 Unit *self,				// Unit using the skill.
 Unit *medium,				// Unused.
 Unit *target,				// Target of attack.
 void *valueIN,				// Unused.
 void *valueOUT,			// Unused.
 LPUSER_SKILL lpusUserSkill // Current skill strength of the user.
 )
 // Return: int, SKILL_* return parameter.
 //////////////////////////////////////////////////////////////////////////////////////////
{    

   const int ImmobilizationRange = 32; //1 cran

   if(theApp.dwEquilibrageNewSkillEnable == 0)
   {
      return SKILL_NO_FEEDBACK;
   }

   // this skill is used on locked objects only.
   if( !( dwReason & HOOK_USE_TARGET_UNIT ) ) 
   {
      return SKILL_NO_FEEDBACK;
   }

   // Skill only work on alive people
   if(target->ViewFlag(__FLAG_NMS_PLAYER_DEATH) > 0)
   {
      return SKILL_NO_FEEDBACK;
   }

   // Skill only works on OTHER characters.
   if( self == target )
   {
      self->SendSystemMessage( _STR( 15383, self->GetLang() ) );        
      return SKILL_NO_FEEDBACK;
   }

   // Ne marche pas sur les mobs ayant 65000 endu
   // - TODO -

   Character *lpCharS = static_cast< Character * >( self );
   Character *lpCharT = static_cast< Character * >( target );

   if( !GAME_RULES::InPVP( self, target ) )
   {
      // Sends a 'cannot cast spell because of pvp blabla' msg to caster.
      self->SendSystemMessage( _STR( 463, self->GetLang() ) );
      return SKILL_NO_FEEDBACK;
   }

   // Return if any of the two opponents are in a safe area.
   if( GAME_RULES::InSafeHaven( self, target ) )
   {
      // Sends a 'cannot cast spell because of pvp blabla' msg to caster.
      self->SendSystemMessage( _STR( 463, self->GetLang() ) );
      return SKILL_NO_FEEDBACK;
   }


   if(lpCharS)
   {
      if(self->GetType() == U_PC && target->GetType() == U_PC && !lpCharS->GetNMCombatMode() && !theApp.IsTargetOnList(self->GetID(),target->GetID()))
      {
         // cannot send spell PVP if combat mode not activated...
         if(self == target)
            self->SendSystemMessage( _STR( 15047, self->GetLang() ) );
         else
            self->SendSystemMessage( _STR( 15036, self->GetLang() ) );
          return SKILL_NO_FEEDBACK;
      }
   }

   UINT uiRet = GAME_RULES::NMPVPCanAttack( self, target );
   if( uiRet > 0)
   {
      if(uiRet == 1)
         lpCharS->SendInfoMessage(_STR( 15037 , lpCharS->GetLang() ),0x0570D5);
      else if(uiRet == 2)
         lpCharS->SendInfoMessage(_STR( 15038 , lpCharS->GetLang() ),0x0570D5);
      else if(uiRet == 3)
         lpCharS->SendInfoMessage(_STR( 15039 , lpCharS->GetLang() ),0x0570D5);
      else if(uiRet == 98)
         lpCharS->SendInfoMessage(_STR( 15040 , lpCharS->GetLang() ),0x0570D5);
      else if(uiRet == 99)
         lpCharS->SendInfoMessage(_STR( 15345 , lpCharS->GetLang() ),0x0570D5);
      else if(uiRet == 1000)
         lpCharS->SendInfoMessage(_STR( 15511 , lpCharS->GetLang() ),0x0570D5);
      else 
         lpCharS->SendInfoMessage(_STR( 15041 , lpCharS->GetLang() ),0x0570D5);

      return SKILL_NO_FEEDBACK;
   }
  

  

   // If the player can use immobilization.
   if( self->ViewFlag( __FLAG_IMMOBILIZATION_EXHAUST ) == 0 ) 
   {

      target->Lock();
      self->Lock();

      // Calculate the range between the players and it's target.
      int nXdiff = abs( self->GetWL().X - target->GetWL().X );
      int nYdiff = abs( self->GetWL().Y - target->GetWL().Y );
      int nRange = ::sqrt( double(nXdiff * nXdiff + nYdiff * nYdiff) );

      double dCompSuccess = 0;
      double dBonusSuccess = 0;
      double dFinalSuccess = 0;
      double dPow = 0;
      double dResist = 0;

      int iSuccess = 0;
      int iTargetExaust = 0;
      int iExaust = 0;
      int iManaCost = 0;

      if( nRange <= ImmobilizationRange  )
      {
         //Moen--------------- // success comp
         
         /*
         dCompSuccess = (double)IMMOBILIZATION_SKILL / 5.0;
         // success bonus : targetEnd - selfEnd * 1/50
         dBonusSuccess = ((double)target->GetEND() - (double)self->GetEND()) * 1/50.0 ;
         // succes final (%russite)
         dFinalSuccess = dCompSuccess + dBonusSuccess;
         */


         //Kiwi
         
         dCompSuccess = (double)IMMOBILIZATION_SKILL / 5.0; 
         dPow         = (((double)self->GetSTR()) + ((double)self->GetAGI()));
         dResist      = (((double)target->GetSTR()) + ((double)target->GetEND()) + ((double)target->GetAGI()) * 1/2.0 + ((double)target->GetINT()) * 1/3.0 + ((double)target->GetWIS()) * 1/3.0);

         dBonusSuccess = dPow/dResist;

         //calcul final de la reussite
         dFinalSuccess = dCompSuccess*log(1.0+dBonusSuccess);

             
         // Coup en mana (fixe)		
         iManaCost = 10;

         if(lpCharS->GetMana() < iManaCost) 
         {
            // not enought mana
            self->SendInfoMessage( _STR( 15387 , self->GetLang() ) ,0x000080);
            // unlock
            self->Unlock();
            target->Unlock();

            // return 
            return SKILL_NO_FEEDBACK;
         }

         WorldPos tempPos;
         WorldMap *wlWorld = TFCMAIN::GetWorld( self->GetWL().world );
         Unit *lpCollisionUnit = NULL;
         if (wlWorld->GetCollisionPos( self->GetWL(), target->GetWL(), &tempPos, &lpCollisionUnit, false, false ))
         {
            self->SendSystemMessage( _STR( 15388, self->GetLang() ) );
            self->Unlock();
            target->Unlock();
            return SKILL_NO_FEEDBACK;
         }


			// Show an effect (entrangle + boule noire)
         // lpCharS->_CastSpellDirect(10531,target);
         Broadcast::BCSpellEffect( lpCharS->GetWL(), _DEFAULT_RANGE, 30379, lpCharS->GetID(), lpCharT->GetID(), lpCharS->GetWL(),lpCharT->GetWL(),GetNextGlobalEffectID(),0);
         lpCharS->SetMana(lpCharS->GetMana() - iManaCost, TRUE); // on consomme la mana du caster...
         
         //Et houpla, on inflige de la fatigue au joueur utilisant IM
         self->DealExhaust(750,750,750);

         // Test de la competence
         iSuccess = rnd( 0, 100 );

         if(  iSuccess <= dFinalSuccess && dFinalSuccess >0)
         {
            // On immobilise la cible : Calcul du nombre de cycles TargetExaust / 2000
            if((int)target->GetSTR() > 500) 
            {
               // 5000
               iTargetExaust =  5000;
            }
            else 
            {
               //5000 + 1d(12501-(self.str*25)))
               double dblRightValue = 12501.0 - ((double)target->GetSTR() * 25.0);
               iTargetExaust = 5000 + rnd( dice( static_cast< int >( 1 ), static_cast< int >( dblRightValue ) ) );
            }

            // On met  jour le nombre de cycles
            DWORD dwNbrCycle = iTargetExaust / 2000;
            target->SetImmobilizationCycle(dwNbrCycle);

            CString strDesc;
            strDesc.Format("%s",_STR( 15384 , lpCharS->GetLang() ));

            TFCPacket sendingSkillEffectS;
            sendingSkillEffectS << (RQ_SIZE)RQ_CreateEffectStatus;
            sendingSkillEffectS << (long)100022; //100000 +SkillID soit 22
            sendingSkillEffectS << (long)(dwNbrCycle*2000);
            sendingSkillEffectS << (long)(dwNbrCycle*2000);
            sendingSkillEffectS << (long)62;
            sendingSkillEffectS << strDesc;
            target->SendPlayerMessage( sendingSkillEffectS );



            int nDodgeBoost = 0;
            // Add dodge penalty 1/3.
            nDodgeBoost -= (target->GetDODGE()*3/4);
            target->SetBoost( BOOST_SKILL_IMMOBILIZATION_DODGE, STAT_DODGE, nDodgeBoost );

            CREATE_EFFECT( target, 
                           MSG_OnTimer, 
                           EFFECT_IMMOBILIZATION_BR, 
                           ImmobilizationBoustRemovalCallback, 
                           NULL, 
                           iTargetExaust MILLISECONDS TDELAY, // Timer frequency
                           iTargetExaust MILLISECONDS TDELAY, // Duration
                           __SKILL_IMMOBILIZATION,
                           0  );

            // EFFET D'ENCHEVETREMENT 
            // On lance l'effet pour 2000ms avec un bloquage de 1745 ms
            CREATE_EFFECT( target, 
                           MSG_OnTimer, 
                           EFFECT_IMMOBILIZATION, 
                           ImmobilizationCallback, 
                           NULL, 
                           SKILL_IMMOB_CYCLE_TIME MILLISECONDS TDELAY, // Timer frequency
                           SKILL_IMMOB_CYCLE_TIME MILLISECONDS TDELAY, // Duration
                           __SKILL_IMMOBILIZATION,
                           0  );

            // Exhaust de mouvement sur la cible
            target->DealExhaust(0,0,SKILL_IMMOB_TARGET_EXHAUST);
         }
         else 
         {
         }

         if( ((Character*)self)->GetGodFlags() & GOD_DEVELOPPER )
         {
            CString csMessage;
            csMessage.Format( "[\"Immobilization\" ] immobilization block the player for %d. (iSuccess: {%d}, ({%10.3lf} dBonusSuccess + {%10.3lf} compSuccess) = {%10.3lf} dFinalSuccess ) // %d",
                                 iTargetExaust ,
                                 iSuccess ,
                                 dBonusSuccess ,
                                 dCompSuccess ,
                                 dFinalSuccess,
                                 self->GetImmobilizationCycle());
            self->SendSystemMessage( csMessage );
         }
      }
      else 
      {
         self->SendSystemMessage( _STR(15357, self->GetLang()) );
      }

      // EXHAUST DU LANCEUR Calling RemoveEffect triggers the effect (in this case), so, we must call RemoveEffect *before* calling the SetFlag =)
      self->RemoveEffect( EFFECT_IMMOBILIZATION_EXHAUST );
      self->SetFlag(__FLAG_IMMOBILIZATION_EXHAUST, (UINT)Immobilization::ExhaustRemovallCallback );

	  // Calcul de l'exhaust mental du skill: 1000+if((1200-(self.level-70)*20)>=0?(1200-(self.level-70)*20):0)ms
	  int casterExhaustTime = 1000;
	  int exhaustMod = 1200-(self->GetLevel()-70 ) * 20;
	  if (exhaustMod > 0) casterExhaustTime += exhaustMod;

      CREATE_EFFECT( self, 
                     MSG_OnTimer, 
                     EFFECT_IMMOBILIZATION_EXHAUST, 
                     ExhaustRemovallCallback, 
                     NULL, 
					      casterExhaustTime MILLISECONDS TDELAY, // Timer frequency
                     casterExhaustTime MILLISECONDS TDELAY, // Duration
                     __SKILL_IMMOBILIZATION,
                     0 );

      // On libre les ressources
      target->Unlock();
      self->Unlock();
   }
   else 
   {
      self->SendSystemMessage( _STR(15358, self->GetLang()) );
   }

   

   return SKILL_NO_FEEDBACK;
}