/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "DispellEffect.h"
#include "../WorldMap.h"
#include "../tfc_main.h"
#include "../Broadcast.h"
#include "../Game_Rules.h"
#include "../Character.h"
#include "../T4CLog.h"

/******************************************************************************/
REGISTER_SPELL_EFFECT( DISPELLEFFECT, DispellEffect::NewFunc, DISPELL_EFFECT, __noop );


/******************************************************************************/
DispellEffect::DispellEffect()
/******************************************************************************/
{
   targetSpellID = 0;
}
/******************************************************************************/
DispellEffect::~DispellEffect()
/******************************************************************************/
{
}

/******************************************************************************/
//  Parameters are inputed to the dispell effect.
BOOL DispellEffect::InputParameter(
 CString csParam,   // The parameter value.
 WORD wParamID      // The parameter ID.
)
/******************************************************************************/
{
    const int TargetSpellIDParam = 1;
    const int SuccessPercentageParam = 2;
    
    switch( wParamID )
	{
		case TargetSpellIDParam:
		{
			targetSpellID = atoi( csParam );
			if( targetSpellID == 0 )
			{
				return FALSE;
			}
			break;
		}
		case SuccessPercentageParam:
			if( !successPercentage.SetFormula( csParam ) )
			{
				return FALSE;
			}
			break;
		default:
			return FALSE;
	}
	return TRUE;
}
/******************************************************************************/
//  Dispell's the given unit
void DispellEffect::Dispell(Unit *caster, Unit *target) // The dispell target.
/******************************************************************************/
{
   // Get the target's effects
   TemplateList<UNIT_EFFECT>*effectList = target->GetAllEffects();

   if( effectList == NULL )
   {
      return;
   }

   effectList->Lock();
   effectList->ToHead();
   while( effectList->QueryNext() )
   {
      UNIT_EFFECT *effect = effectList->Object();

      // If the spell ID was found
      if( effect->bindedSpellID == targetSpellID )
      {
         // Send a dispell message to the effect.
         effect->lpFunc( MSG_OnDispell, effect->dwEffect, target, NULL, caster, effect->lpData, NULL );
         effectList->DeleteAbsolute();
      }        
   }
   effectList->Unlock();

#if 0
   list< UNIT_EFFECT * > dispellList;

   effectList->Lock();

   effectList->ToHead();
   while( effectList->QueryNext() )
   {
      UNIT_EFFECT *effect = effectList->Object();

      // If the spell ID was found
      if( effect->bindedSpellID == targetSpellID )
      {
         // Send a dispell message to the effect
         dispellList.push_back( effect );
      }        
   }

   effectList->Unlock();

   list< UNIT_EFFECT * >::iterator i;
   for( i = dispellList.begin(); i != dispellList.end(); i++ )
   {
      // Send a dispell message to the effect.
      (*i)->lpFunc( MSG_OnDispell, (*i)->dwEffect, target, NULL, caster, (*i)->lpData, NULL );
   }
   #endif

}
/******************************************************************************/
//  Called when the effect is triggered by the binded spell.
void DispellEffect::CallEffect(SPELL_EFFECT_PROTOTYPE)
/******************************************************************************/
{
   TRACE("***DispellEffect\n");
    // Target required.
    if( self == NULL )
	{
        return;
    }

    static Random rnd;
    
    // If there is a unit in center.
    if( target == NULL )
	{
        // Require a target.
        return;
    }

    // If the spell succesfully caught the target.
	int iSuccess = successPercentage.GetBoost( self, target, 0, 0, range );
    if( rnd( 0, 100 ) <= iSuccess &&  iSuccess > 0)
	{
        Dispell( self, target );    
    }
}
/******************************************************************************/
// Returns an instance of a spell effect.
SpellEffect *DispellEffect::NewFunc(LPSPELL_STRUCT lpSpell)
/******************************************************************************/
{
    CREATE_EFFECT_HANDLE( DispellEffect, 0 );
}
