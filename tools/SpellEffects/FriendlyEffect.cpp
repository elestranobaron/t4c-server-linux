/******************************************************************************
Modify for vs2008 (26/04/2009)
Remove in CallEffect the dwDuration affect, is already read in InputParameter by Nightmare (28/06/2009)
/******************************************************************************/
#include "stdafx.h"
#include "..\NPCmacroScriptLng.h"
#include "FriendlyEffect.h"
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


REGISTER_SPELL_EFFECT( FLAG_ADDING, FriendlyEffect::NewFunc, FRIENDLY_EFFECT, FriendlyEffect::Init );

FriendlyEffect::FriendlyEffect()
{
    dwMonsterFID  = 0;
    dwValue       = 0;
}

FriendlyEffect::~FriendlyEffect()
{
}

/******************************************************************************/
// Called by REGISTER_SPELL_EFFECT to complete spell initialisation
void FriendlyEffect::Init( void )
{
}

/******************************************************************************/
// Enters the different effect parameters
BOOL FriendlyEffect::InputParameter(CString csParam,WORD wParamID)
{       
   const int MonsterID = 1;
   const int ValueID   = 2;

   BOOL boOK = TRUE;

   switch( wParamID )
   {
      // FlagID
      case MonsterID:
         dwMonsterFID = atoi( (LPCTSTR)csParam );
         if( dwMonsterFID == 0 )
         {
            boOK = FALSE;
         }     
      break;
      // FlagValue
      case ValueID:
         dwValue = atoi( (LPCTSTR)csParam );
         if(dwValue != 0 && dwValue != 1)
            dwValue = 0;
      break;
      // Duration
      default:         
         boOK = FALSE; 
      break;
   }

   return boOK;
}

/******************************************************************************/
// Does the flag adding effect
void FriendlyEffect::CallEffect(SPELL_EFFECT_PROTOTYPE)
{
   TRACE("***FriendlyEffect\n");

   if( target != NULL )
   {
      int iFactionID = target->ViewFlag(__FLAG_PJ_VS_MONSTER_FRIENDLY);
      if(iFactionID >0 && iFactionID <32 && dwMonsterFID > 0)
      {
         DWORD dwFlagID      = 3000000+dwMonsterFID;
         DWORD dwFactionMask = PLFactionMask(iFactionID);
         if(dwValue == 0) //reset le friendly
         {
            GiveGlobalFlag( dwFlagID,CheckGlobalFlag(dwFlagID)& ~dwFactionMask ); //set le bit de friendly a 0
         }
         else //Set le friendly
         {
            GiveGlobalFlag( dwFlagID,CheckGlobalFlag(dwFlagID)| dwFactionMask ); //set le bit de friendly a 1
         }
         BroadcastFriendlyUpdate(target->GetWL(),dwMonsterFID);
      }
   }
}
/******************************************************************************/
// Returns an instance of FriendlyEffect
SpellEffect *FriendlyEffect::NewFunc(LPSPELL_STRUCT lpSpell)
/******************************************************************************/
{
	CREATE_EFFECT_HANDLE( FriendlyEffect, 1 )
}