// TakeGold.h: interface for the TakeGold class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "../SpellEffectManager.h"


class TakeGold : public SpellEffect
{
public:
   TakeGold();
   virtual ~TakeGold();
   BOOL InputParameter( CString csParam, WORD wParamID );
   void CallEffect( SPELL_EFFECT_PROTOTYPE );
   static SpellEffect *NewFunc( LPSPELL_STRUCT lpSpell );
private:
   BoostFormula montantPercentage;
   BoostFormula successPercentage;
};


