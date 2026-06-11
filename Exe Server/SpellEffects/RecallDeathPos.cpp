/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "RecallDeathPos.h"

/******************************************************************************/
REGISTER_SPELL_EFFECT( RECALLDEATHPOS_EFFECT, RecallDeathPos::NewFunc, RECALLDEATHPOS_EFFECT, __noop );
const WorldPos wlDeathPos = { 2948, 1041, 0 }; // Temple de Lighthaven

/******************************************************************************/
RecallDeathPos::RecallDeathPos()
/******************************************************************************/
{
}
/******************************************************************************/
RecallDeathPos::~RecallDeathPos()
/******************************************************************************/
{
}
/******************************************************************************/
// Input parameter
BOOL RecallDeathPos::InputParameter(
 CString csParam,   // Parameter
 WORD wParamID      // Paremeter ID.
)
/******************************************************************************/
{
    return true;
}
/******************************************************************************/
// Does the recall effect
void RecallDeathPos::CallEffect(SPELL_EFFECT_PROTOTYPE) // The spell data.
/******************************************************************************/
{
   TRACE("***RecallDeathPos\n");
   if( target != NULL )
   {

      WorldPos wlTeleportPos;

      DWORD dwPosValue = target->ViewFlag( __FLAG_DEATH_LOCATION );

      if( dwPosValue != 0 )
      {
         wlTeleportPos.X = ( (WORD)( dwPosValue >> 20 ) ) & 0x0FFF;	// Strip the first 4 bits of the word.
         wlTeleportPos.Y = ( (WORD)( dwPosValue >> 8 )  ) & 0x0FFF;
         wlTeleportPos.world = ( (BYTE)( dwPosValue ) & 0xFF );
      }
      else
      {
         wlTeleportPos = wlDeathPos;
      }

      target->Teleport( wlTeleportPos, 0 );
   }
}
/******************************************************************************/
// Creates an instance of Recall effect.
SpellEffect *RecallDeathPos::NewFunc(LPSPELL_STRUCT lpSpell) // The spell structure
/******************************************************************************/
{
   CREATE_EFFECT_HANDLE( RecallDeathPos, 0 );    
}