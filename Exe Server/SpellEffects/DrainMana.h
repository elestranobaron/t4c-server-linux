// DrainMana.h: interface for the DrainMana class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "../SpellEffectManager.h"


class DrainMana : public SpellEffect
{
public:
   DrainMana();
   virtual ~DrainMana();
   BOOL InputParameter( CString csParam, WORD wParamID );
   void CallEffect( SPELL_EFFECT_PROTOTYPE );
   static SpellEffect *NewFunc( LPSPELL_STRUCT lpSpell );
private:
   BoostFormula manaPoints;
   BoostFormula manaRecupPercentage;
   BoostFormula successPercentage;
};


