// TakeGold.cpp: implementation of the TakeGold class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TakeGold.h"
#include "../TFC_MAIN.h"


#define SAMEDIR( __text )  csParam.CompareNoCase( __text ) == 0
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

REGISTER_SPELL_EFFECT( TAKEGOLD, TakeGold::NewFunc, TAKEGOLD_EFFECT, __noop );


TakeGold::TakeGold()
{
}

TakeGold::~TakeGold()
{
}

//////////////////////////////////////////////////////////////////////////////////////////
BOOL TakeGold::InputParameter(CString csParam,WORD wParamID)
{
   const int Montant   = 1;
   const int Success   = 2;
   switch( wParamID )
   {
      case Montant:
         if( !montantPercentage.SetFormula( csParam ) )
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

void TakeGold::CallEffect( SPELL_EFFECT_PROTOTYPE )
{
   if(self != NULL && target != NULL && self->GetType() == U_PC && target->GetType()== U_PC)
   {
      TRACE("***TakeGold\n");
      //on peu voler uniquement entre player ...
      int iMontant = montantPercentage.GetBoost( self, target, 0, 0, range );
      int iSuccess = successPercentage.GetBoost( self, target, 0, 0, range );

      static Random rnd;
      if( rnd( 0, 100 ) <= iSuccess &&  iSuccess > 0)
      {
         int iTG = target->GetGold();
         int iSG = self->GetGold();
         if(iTG < iMontant)
            iMontant = iTG;

         target->SetGold(iTG-iMontant,FALSE);
         self->SetGold(iSG+iMontant,TRUE);

      }
   }
}

//////////////////////////////////////////////////////////////////////////////////////////
SpellEffect *TakeGold::NewFunc(LPSPELL_STRUCT lpSpell)
{
    CREATE_EFFECT_HANDLE( TakeGold, 0 )
}
