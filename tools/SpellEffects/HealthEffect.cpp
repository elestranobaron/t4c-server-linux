/******************************************************************************
Modify for vs2008 (26/04/2009)
ADD NM PVP Send healing and damage by Nightmare (28/06/2009)
/******************************************************************************/
#include "stdafx.h"
#include "HealthEffect.h"
#include "../GameDefs.h"
#include "../tfc_main.h"
#include "../Broadcast.h"
#include "../Game_Rules.h"
#include "../format.h"
#include "../Character.h"
#include "../T4CLog.h"
#include "../TFC Server.h"

extern CTFCServerApp theApp;

/******************************************************************************/
REGISTER_SPELL_EFFECT( HEALTH, HealthEffect::NewFunc, HEALTH_EFFECT, __noop );

/******************************************************************************/
HealthEffect::HealthEffect()
/******************************************************************************/
{
}
/******************************************************************************/
HealthEffect::~HealthEffect()
/******************************************************************************/
{
}
/******************************************************************************/
// Enters a parameters needed for this effect to work
BOOL HealthEffect::InputParameter(
 CString csParam,    // The parameter value.
 WORD wParamID      // The parameter ID.
)
/******************************************************************************/
{
    const int HealthBoost = 1;
    const int RadialHealthBoost = 2;
    const int Success = 3;
    
    BOOL boParamOK = TRUE;

    switch( wParamID )
	{
		// The standard spell health modifier for the target unit
		case HealthBoost:
			// If the given formula isn't valid
			if( !cHealth.SetFormula( csParam ) )
			{
				// Invalidate parameter.
				boParamOK = FALSE;
			}
			break;
		// Peripheral damage, this parameter can be null.
		case RadialHealthBoost:
			csParam.TrimRight();
			csParam.TrimLeft();

			// If the parameter doesn't exist, leave formula untouched (it will return 0).
			if( !csParam.IsEmpty() )
			{
				// If the given formula isn't valid
				if( !cRadialHealth.SetFormula( csParam ) )
				{
					// Invalidate parameter.
					boParamOK = FALSE;
				}
			}
			break;
		case Success:
			if( !successPercent.SetFormula( csParam ) )
			{
				return FALSE;
			}
			break;
		default:
			boParamOK = FALSE;
			break;
	}

	return boParamOK;
}
/******************************************************************************/
// Deals damage to a unit
int HealthEffect::DoUnitDamage(
 Unit *self,                // Unit dealing damage.
 Unit *target,              // Unit target of damage.
 LPSPELL_STRUCT lpSpell,    // Spell effect dealing damage.
 double dblDamage           // Damage to deal.
)
/******************************************************************************/
{
   TRACE( "\r\nself=%u, target=%u",self->GetUnderBlock(),target->GetUnderBlock());
   // Return if any of the two opponents are in a safe area.
   if( GAME_RULES::InSafeHaven( self, target ))
      return 0;

   UINT uiRet = GAME_RULES::NMPVPCanAttack( self, target );
   if( uiRet > 0 )
      return 0;

   // Quantity of damage dealt.
   int nDamage = 0;

   // If the two units are in PVP
   if( GAME_RULES::InPVP( self, target ) )
   {
      double dblHealth = dblDamage;
      // Avoid unnecessary damage calculation if no damage was dealt.
      if( dblHealth != 0 )
      {
         // Organise an attack.
         ATTACK_STRUCTURE Blow = {0,0,0,0,0};

         Blow.Strike = -dblHealth;            

         TRACE( "Blow->Strike=%f. Traget = %X", Blow.Strike,target );
         target->Lock();

         double dblPreACstrike = Blow.Strike;

         // If this is a 'physical' spell.
         nDamage = Blow.Strike;
         target->attacked( &Blow, NULL );
         if( lpSpell->bDamageType == ATTACK_PHYSICAL )
         {
            Blow.Strike = Blow.Strike > 0 ? Blow.Strike : 0;
            nDamage = Blow.Strike;
         }
         else
         {
            Blow.Strike = nDamage;
         }

         if(theApp.dwEquilibrageNewSkillEnable == 1)
         {
			   // Moen_OK : Application des skills mages (Nigh: A Tester)
			   // Then process all attacked-intrinsic skills
            if( self->GetType() == U_PC) // The unit is a player
            {
               // Spell ID not in the black list
               // Moen_OK : Night flusher la fonction car le spell exclusion ets rendu un parametre des wda...
               if (!lpSpell->boSkillExclusion) 
               {
                  TFormat format;
                  TemplateList <USER_SKILL> * tlusSkills ;
                  tlusSkills = static_cast< Character *>( self )->GetSkillsList();
                  // Il y a des skills..
                  if (tlusSkills != NULL) 
                  {
                     // .. on prends ceux qui sont à activer au moment de l'attaque
                     tlusSkills[Hook_OnSpellAttack].ToHead();
                     while(tlusSkills[Hook_OnSpellAttack].QueryNext())
                     {
                        LPUSER_SKILL lpusUserSkill = tlusSkills[Hook_OnSpellAttack].Object();
                        Skills::ExecuteSkill(lpusUserSkill->GetSkillID(), HOOK_SPELL_ATTACK,self, NULL, target, &Blow, NULL, lpusUserSkill);

                        _LOG_DEBUG 
                           LOG_DEBUG_LVL3, 
                           "Application du skill mage (Hook_OnSpellAttack) : %d ", 
                           lpusUserSkill->GetSkillID()
                        LOG_
                     }
                  }
                  else 
                  {
                     // self->SendSystemMessage(  format(  "No skill"   )      );
                  }
               }
            }
         }

         // Then process 'attack' with a NULL unit.            

         bool bDamageSent = false;
         if( self->GetType() == U_PC && target != self)
         {
            Players *lpPlayer = static_cast< Character *>( self )->GetPlayer();
            if( lpPlayer != NULL && lpPlayer->GetGodFlags() & GOD_DEVELOPPER )
            {
               TFormat format;
               self->SendSystemMessage(
                  format(
                  "Spell %u hits %s for %.2f damages (%.2f post-AC damages).",
                  lpSpell->wSpellID,
                  (LPCTSTR)target->GetName(_DEFAULT_LNG),
                  dblPreACstrike,
                  Blow.Strike
                  )
                  );
            } 


            if( lpPlayer != NULL)
            {
               lpPlayer->m_DPSCounter +=(int)Blow.Strike;                  
               if((int)Blow.Strike >0 && theApp.dwSendDamageHealingSystem == 1)
               {
                  DWORD color = CL_HEAL_DAMAGE_1;
                  CString strDamage;
                  strDamage.Format("-%d",(int)Blow.Strike);
                  TFCPacket sending;
                  sending << (RQ_SIZE)RQ_DamageUnit;
                  sending << (long)target->GetID();
                  sending << strDamage;
                  sending << (long)color;
                  lpPlayer->self->SendPlayerMessage( sending );
               }
            }
         }

         if( target->GetType() == U_PC )
         {
            Players *lpPlayer = static_cast< Character *>( target )->GetPlayer();
            if( lpPlayer != NULL && lpPlayer->GetGodFlags() & GOD_DEVELOPPER )
            {
               TFormat format;
               target->SendSystemMessage(
                  format(
                  "%s casts spell %u on you for %.2f damages (%.2f post-AC damages).",                            
                  (LPCTSTR)self->GetName(_DEFAULT_LNG),
                  lpSpell->wSpellID,
                  dblPreACstrike,
                  Blow.Strike
                  )
                  );
            }
            if( lpPlayer != NULL)
            {
               if((int)Blow.Strike >0 && theApp.dwSendDamageHealingSystem == 1)
               {
                  DWORD color = CL_HEAL_DAMAGE_2;
                  CString strDamage;
                  strDamage.Format("-%d",(int)Blow.Strike);
                  TFCPacket sending;
                  sending << (RQ_SIZE)RQ_DamageUnit;
                  sending << (long)target->GetID();
                  sending << strDamage;
                  sending << (long)color;
                  lpPlayer->self->SendPlayerMessage( sending );
               }
            }
         } 

         // Hit target with blast.
         if( target->hit( &Blow, self ) == -1 )
         {
            // Tell everyone that the "thing" died
            Broadcast::BCObjectChanged( target->GetWL(), _DEFAULT_RANGE_CHANGE,
               target->GetCorpse(),
               target->GetID(),
               1
               );
         }
         //NM_COMM_ ::Add en else pkoi chnager la vie et le struc du mob si ye mort sa donne plus rien...
         else if( Blow.Strike != 0 )// If the blow did damage to the unit.
         {
            // Broadcast HP change.
            TFCPacket sending;
            sending << (RQ_SIZE)RQ_UnitUpdate;
            target->PacketUnitInformation( sending );

            Broadcast::BCast( target->GetWL(), _DEFAULT_RANGE, sending );
         }

         target->Unlock();
      }
   }

   // Return the quantity of damage delt. Damage is NEGATIVE
   return( -nDamage );
}
/******************************************************************************/
// Heals the current target.
int HealthEffect::DoUnitHealing(
 Unit *self,                // Caster.
 Unit *target,              // Target of healing.
 LPSPELL_STRUCT lpSpell,    // Spell.
 double dblHealth           // Health to heal.
)
/******************************************************************************/
{
    target->Lock();

    if(self && target && theApp.m_dwPVPSyetem2Actif == 1)
    {
         //on sassure que c des joueur, on sassure que c pas le meme joueur
         if( self->GetType() == U_PC && target->GetType() == U_PC && self != target)
         {
            bool bSafeS = false;
            bool bSafeT = false;
            bool bCombatS = false;
            bool bCombatT = false;
            if(self->GetUnderBlockMap()  == __SAFE_HAVEN   || self->GetUnderBlockMap()  == __INDOOR_SAFE_HAVEN)
               bSafeS = true;
            if(target->GetUnderBlockMap()  == __SAFE_HAVEN   || target->GetUnderBlockMap()  == __INDOOR_SAFE_HAVEN)
               bSafeT = true;
            if(self->GetNMCombatMode())
               bCombatS = true;
            if(target->GetNMCombatMode())
               bCombatT = true;

            if(bSafeS != bSafeT || bCombatS != bCombatT)
            {
               
               self->SendInfoMessage(_STR( 15468 , self->GetLang() ),0x0570D5);

               target->Unlock();
               return 0.00;
            }
         }
    }
    
    
    int nHP = target->GetHP();
    int nOldHP = nHP;

    TRACE( "\r\nHealing target for %d damage", (int)dblHealth );

    TRACE( ", HP Before=%d. ", nHP );
    nHP += (int)dblHealth;
    TRACE( ", HP After=%d. ", nHP );

    if( self->GetType() == U_PC )
    {
       Players *lpPlayer = static_cast< Character *>( self )->GetPlayer();
       if( lpPlayer != NULL && lpPlayer->GetGodFlags() & GOD_DEVELOPPER )
       {
          TFormat format;
          self->SendSystemMessage(format("Spell %u healed %s for %f hp.",lpSpell->wSpellID,(LPCTSTR)target->GetName(_DEFAULT_LNG),dblHealth));
       }

       if( lpPlayer != NULL)
       {
          if((int)dblHealth >0  && theApp.dwSendDamageHealingSystem == 1)
          {
             if(self == target)
             {
               lpPlayer->self->AddYoursHealingValue((int)dblHealth);
             }
             else
             {
                DWORD color = CL_HEAL_DAMAGE_1;
                CString strDamage;
                strDamage.Format("+%d",(int)dblHealth);
                TFCPacket sending;
                sending << (RQ_SIZE)RQ_HealingUnit;
                sending << (long)target->GetID();
                sending << strDamage;
                sending << (long)color;
                lpPlayer->self->SendPlayerMessage( sending );
             }
          }
       }
    }
    if( target->GetType() == U_PC )
    {
       Players *lpPlayer = static_cast< Character *>( target )->GetPlayer();
       if( lpPlayer != NULL && lpPlayer->GetGodFlags() & GOD_DEVELOPPER )
       {
          TFormat format;
          target->SendSystemMessage(format("%s casts spell %u on you, which healed %f hp.",(LPCTSTR)self->GetName(_DEFAULT_LNG),lpSpell->wSpellID,dblHealth));
       }
       if( lpPlayer != NULL)
       {
          if((int)dblHealth >0 && self != target && theApp.dwSendDamageHealingSystem == 1)
          {
             DWORD color = CL_HEAL_DAMAGE_3;
             CString strDamage;
             strDamage.Format("+%d",(int)dblHealth);
             TFCPacket sending;
             sending << (RQ_SIZE)RQ_HealingUnit;
             sending << (long)target->GetID();
             sending << strDamage;
             sending << (long)color;
             lpPlayer->self->SendPlayerMessage( sending );
          }
       }
    }

    // If health would mean giving too much HP
    if( nHP >= (int)target->GetMaxHP() )
    {
       // Set unit's HP to max.
       target->SetHP( target->GetMaxHP(), true );
    }
    else
    {
       // Set the new unit's HP.
       target->SetHP( nHP, true );
    }

    target->Unlock();

    TFCPacket sending;
    sending << (RQ_SIZE)RQ_HPchanged;
    sending << (long)target->GetHP();
    sending << (long)target->GetMaxHP();
    target->SendPlayerMessage( sending );

    sending.Destroy();
    sending << (RQ_SIZE)RQ_UnitUpdate;
    target->PacketUnitInformation( sending );

    Broadcast::BCast( target->GetWL(), _DEFAULT_RANGE, sending );

    // Returns the quantity of healt given.
    return (int)dblHealth;
}
/******************************************************************************/
//  Deals the health effect. Also usefull for derivative functions.
int HealthEffect::DealHealthEffect(SPELL_EFFECT_PROTOTYPE) // The spell effect prototype.
/******************************************************************************/
{
   // Default center is the spell's target position
   double dblHealth;

   int nTotalDamage = 0;

   // If there is a unit in center.
   if( target == NULL )
   {
      return 0;
   }

   // If spell failed.
   static Random rnd;
   int iSuccess = successPercent.GetBoost( self, target, 0, 0, range );
   if( rnd( 0, 100 ) <=  iSuccess && iSuccess > 0)
   {
	   if( range == 0 )
	   {
		  dblHealth = cHealth.GetBoost( self, target );
	   }
	   else
	   {
		  dblHealth = cRadialHealth.GetBoost( self, target, 0, 0, range );
	   }
      if(dblHealth < 0 && target->GetAC() >= 65000)
      {
         dblHealth = 0; //CV: A VALIDER :assure que les mob avec 65000k de ca ou plus sont invincible spell aussi
      }

	   // If this does damage.
	   if( dblHealth < 0 )
	   {
		  // Deal damage.
		  nTotalDamage += DoUnitDamage( self, target, lpSpell, dblHealth );
	   }
	   else if( dblHealth > 0 )
	   {
		  // Heal target.
		  nTotalDamage += DoUnitHealing( self, target, lpSpell, dblHealth );
	   }
	   return nTotalDamage;
   }

   return 0;
}
/******************************************************************************/
// Does the spell effect.
void HealthEffect::CallEffect(SPELL_EFFECT_PROTOTYPE) // The spell effect variables.
/******************************************************************************/
{    
   TRACE("***HealthEffect\n");
    // Simply deal health effect.
    DealHealthEffect( SPELL_EFFECT_PARAMS );
}
/******************************************************************************/
// Returns a pointer to a HealthEffect object.
SpellEffect *HealthEffect::NewFunc( LPSPELL_STRUCT lpSpell)
/******************************************************************************/
{    
    CREATE_EFFECT_HANDLE( HealthEffect, 0 );
}

