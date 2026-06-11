// DrainMana.cpp: implementation of the DrainMana class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DrainMana.h"
#include "../TFC_MAIN.h"


#define SAMEDIR( __text )  csParam.CompareNoCase( __text ) == 0
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

REGISTER_SPELL_EFFECT( DRAIN_MANA, DrainMana::NewFunc, DRAIN_MANA_EFFECT, __noop );


DrainMana::DrainMana()
{
}

DrainMana::~DrainMana()
{
}

//////////////////////////////////////////////////////////////////////////////////////////
BOOL DrainMana::InputParameter(CString csParam,WORD wParamID)
{
   const int iMana      = 1;
   const int iManaRecup = 2;
   const int Success    = 3;
   switch( wParamID )
   {
      case iMana:
         if( !manaPoints.SetFormula( csParam ) )
            return FALSE;
      break;
      case iManaRecup:
         if( !manaRecupPercentage.SetFormula( csParam ) )
            return FALSE;
         break;
      case Success:
         if( !successPercentage.SetFormula( csParam ) )
            return FALSE;
      break;

      default:
         return FALSE;
   }
   return TRUE;
}

void DrainMana::CallEffect( SPELL_EFFECT_PROTOTYPE )
{
   if(self != NULL && target != NULL && self->GetType() == U_PC && target->GetType()== U_PC)
   {
      TRACE("***DrainMana\n");
      //on peu voler uniquement entre player ...
      int iNbrManaPoints = manaPoints         .GetBoost( self, target, 0, 0, range );
      int iNbrManaPC     = manaRecupPercentage.GetBoost( self, target, 0, 0, range );
      int iSuccess       = successPercentage  .GetBoost( self, target, 0, 0, range );

      static Random rnd;
      if( rnd( 0, 100 ) <= iSuccess &&  iSuccess > 0)
      {
         int iTM = target->GetMana();
         int iSM = self->GetMana();
         int iNbrManaGive = iNbrManaPoints*iNbrManaPC/100;
         if(iTM < iNbrManaPoints)
            iNbrManaPoints = iTM;

         target->SetMana(iTM-iNbrManaPoints,TRUE);
         self->SetMana(iSM+iNbrManaGive,TRUE);
      }
   }
}

//////////////////////////////////////////////////////////////////////////////////////////
SpellEffect *DrainMana::NewFunc(LPSPELL_STRUCT lpSpell)
{
   CREATE_EFFECT_HANDLE( DrainMana, 0 )
}
