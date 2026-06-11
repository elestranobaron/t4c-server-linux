// MoveOut.h: interface for the MoveOut class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "../SpellEffectManager.h"


class MoveOut : public SpellEffect
{
public:
   MoveOut();
   virtual ~MoveOut();
   BOOL InputParameter( CString csParam, WORD wParamID );
   void CallEffect( SPELL_EFFECT_PROTOTYPE );
   static SpellEffect *NewFunc( LPSPELL_STRUCT lpSpell );
private:
   DWORD        dwDirection;
   BoostFormula successPercentage;
};


