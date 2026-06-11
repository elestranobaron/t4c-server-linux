/******************************************************************************
Modify for vs2008 (26/04/2009)
Remove in CallEffect the dwDuration affect, is already read in InputParameter by Nightmare (28/06/2009)
/******************************************************************************/
#include "stdafx.h"
#include "../NPCmacroScriptLng.h"
#include "SpellPositionEffect.h"
#include "../TFC_MAIN.h"

/******************************************************************************/

// Saved Structure 
typedef struct _FLAG_ADDINGv1
{
    DWORD dwFlagID;
} FLAG_ADDINGv1, *LPFLAG_ADDINGv1;

typedef struct _FLAG_ADDING
{
    DWORD dwFlagID;
    DWORD effectID;
    DWORD spellID;
} FLAG_ADDING, *LPFLAG_ADDING;


REGISTER_SPELL_EFFECT( FLAG_ADDING, SpellPositionEffect::NewFunc, SPELL_POSITION_EFFECT, SpellPositionEffect::Init );

SpellPositionEffect::SpellPositionEffect()
{
    dwSpellID   = 0;
    dwFreq      = 0;
    dwNbrRepeat = 0;
}

SpellPositionEffect::~SpellPositionEffect()
{
}

/******************************************************************************/
// Called by REGISTER_SPELL_EFFECT to complete spell initialisation
void SpellPositionEffect::Init( void )
{
}

/******************************************************************************/
// Enters the different effect parameters
BOOL SpellPositionEffect::InputParameter(CString csParam,WORD wParamID)
{       
   const int cSpellID = 1;
   const int cFreq    = 2;
   const int cRepeat  = 3;

   BOOL boOK = TRUE;

   switch( wParamID )
   {
      case cSpellID: // Spell ID
         dwSpellID = atoi( (LPCTSTR)csParam );
         if( dwSpellID == 0 )
            boOK = FALSE;
      break;
      case cFreq: // Frequence
         dwFreq = atoi( (LPCTSTR)csParam );
         if( dwFreq == 0 )
            boOK = FALSE;
      break;
      case cRepeat: // Nbr Repeat Time
         dwNbrRepeat = atoi( (LPCTSTR)csParam );
         if( dwNbrRepeat == 0 )
            boOK = FALSE;
      break;

      default:         
         boOK = FALSE; 
      break;
   }

   return boOK;
}

/******************************************************************************/
// Does the flag adding effect
void SpellPositionEffect::CallEffect(SPELL_EFFECT_PROTOTYPE)
{
   TRACE("***SpellPositionEffect\n");

   AddCastSpellPosition( self,wlPos,dwSpellID,dwFreq,dwNbrRepeat);
}
/******************************************************************************/
// Returns an instance of SpellPositionEffect
SpellEffect *SpellPositionEffect::NewFunc(LPSPELL_STRUCT lpSpell)
/******************************************************************************/
{
	CREATE_EFFECT_HANDLE( SpellPositionEffect, 1 )
}