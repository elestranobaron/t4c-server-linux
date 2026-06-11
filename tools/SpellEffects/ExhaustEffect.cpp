/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#pragma hdrstop
#include "stdafx.h"
#include "ExhaustEffect.h"
#include "../TFC_MAIN.h"

typedef struct _EXHAUST_ADDING
{
   DWORD spellID;
   DWORD AttackExhaust;
   DWORD MentalExhaust;
   DWORD MoveExhaust;
} EXHAUST_ADDING, *LPEXHAUST_ADDING;

/******************************************************************************/
REGISTER_SPELL_EFFECT( EXHAUST_EFFECT_INSTANCE, ExhaustEffect::NewFunc, EXHAUST_EFFECT, __noop );

/******************************************************************************/
ExhaustEffect::ExhaustEffect()
/******************************************************************************/
{
}
/******************************************************************************/
ExhaustEffect::~ExhaustEffect()
/******************************************************************************/
{
}
/******************************************************************************/
//  Adds parameters to the exhaust effect.
BOOL ExhaustEffect::InputParameter(
 CString csParam,   // The parameter value.
 WORD wParamID
)
/******************************************************************************/
{
    const int Attack = 1;
    const int Mental = 2;
    const int Move   = 3;
    const int Success = 4;

    switch( wParamID )
	{
		case Attack:
			if( !attackExhaust.SetFormula( csParam ) )
			{
				return FALSE;
			}
			break;
		case Mental:
			if( !mentalExhaust.SetFormula( csParam ) )
			{
				return FALSE;
			}
			break;
		case Move:
			if( !moveExhaust.SetFormula( csParam ) )
			{
				return FALSE;
			}
			break;
		case Success:
			if( !successPercent.SetFormula( csParam ) )
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
//  Deals the exhaust to the target unit.
void ExhaustEffect::CallEffect(SPELL_EFFECT_PROTOTYPE)
/******************************************************************************/
{
   TRACE("***ExhaustEffect\n");
    if( target == NULL )
	{
        return;
    }

    static Random rnd;
	int iSuccess = successPercent.GetBoost( self, target, 0, 0, range ) ;
    if( rnd( 0, 100 ) <= iSuccess && iSuccess > 0 )
	{
    	DWORD theAttackExhaust = attackExhaust.GetBoost( self, target, 0, 0, range );
		DWORD theMentalExhaust = mentalExhaust.GetBoost( self, target, 0, 0, range );
		DWORD theMoveExhaust   = moveExhaust.GetBoost( self, target, 0, 0, range );
	    
		target->DealExhaust(theAttackExhaust,theMentalExhaust,theMoveExhaust);
	    
		DWORD maxExhaust = theAttackExhaust;
		if( theMentalExhaust > maxExhaust )
		{
			maxExhaust = theMentalExhaust;
		}
		if( theMoveExhaust > maxExhaust )
		{
			maxExhaust = theMoveExhaust;
		}

      // Setup the add flag
      LPEXHAUST_ADDING lpAddExhaust = new EXHAUST_ADDING;
      lpAddExhaust->spellID  = lpSpell->wSpellID;
      lpAddExhaust->AttackExhaust = theAttackExhaust;
      lpAddExhaust->MentalExhaust = theMentalExhaust;
      lpAddExhaust->MoveExhaust   = theMoveExhaust;

		// Create an effect status update for the client.
		CreateEffectStatus(target,lpSpell->wSpellID,maxExhaust,maxExhaust,lpSpell);

      DWORD dwDuration = 0;
      if(theAttackExhaust > theMentalExhaust)
         dwDuration = theAttackExhaust;
      else
         dwDuration = theMentalExhaust;
      if(theMoveExhaust > dwDuration)
         dwDuration = theMoveExhaust;
         

      CREATE_EFFECT(
         target,
         MSG_OnTimer,
         dwEffectID,
         ExhaustRemoval,
         lpAddExhaust,
         dwDuration MILLISECONDS TDELAY,
         dwDuration,
         lpSpell->wSpellID,
         0
         );
	}
}

/******************************************************************************/
// Called when timer expires and flag must be removed.
void ExhaustEffect::ExhaustRemoval(EFFECT_FUNC_PROTOTYPE)
/******************************************************************************/
{
   // Timer make the call to the function.
   if( wMessageID == MSG_OnTimer )
   {
      LPEXHAUST_ADDING lpExhaustFlag = (LPEXHAUST_ADDING)lpEffectData;
      if (lpExhaustFlag != NULL)
         DispellEffectStatus( self, lpExhaustFlag->spellID ,661);
   }
   // If the spell got dispelled
   else if( wMessageID == MSG_OnDispell )//now destroy effect directly here
   {
      LPEXHAUST_ADDING lpExhaustFlag = (LPEXHAUST_ADDING)lpEffectData;
      if (lpExhaustFlag != NULL)
      {
         DispellEffectStatus( self, lpExhaustFlag->spellID ,662);

         //reset le deal exhaust ici...
         EXHAUST newExhaust = self->GetExhaust();
         if(lpExhaustFlag->AttackExhaust >0 )
            newExhaust.attack = 0;
         if(lpExhaustFlag->MentalExhaust >0 )
            newExhaust.mental = 0;
         if(lpExhaustFlag->MoveExhaust >0 )
         {
            newExhaust.move      = 0;
            newExhaust.boWalking = TRUE;
         }
         self->SetExhaust(newExhaust);
      }

      // Remove the effect.
      //self->Remove_Effect( dwEffect ); 

      if (lpExhaustFlag != NULL)
      {
         delete lpExhaustFlag;
         lpExhaustFlag = NULL;
      }
   }
   // Player exited
   else if( wMessageID == MSG_OnSavePlayer )
   {
      // Fetch the pointers
      LPEXHAUST_ADDING lpAddExhaust = (LPEXHAUST_ADDING)lpEffectData;
      LPDATA_SAVE lpDataSave = (LPDATA_SAVE)lpUserData;
      // Prepare buffer to save on player
      lpDataSave->bBufferSize = sizeof( EXHAUST_ADDING );
      lpDataSave->lpbData = new BYTE[ sizeof( EXHAUST_ADDING ) ];        
      memcpy( lpDataSave->lpbData, lpAddExhaust, sizeof( EXHAUST_ADDING ) );             
   }
   // Player loads
   else if( wMessageID == MSG_OnLoadPlayer )
   {
      LPUNIT_EFFECT lpUnitEffect = (LPUNIT_EFFECT)lpEffectData;
      LPDATA_SAVE lpDataSave = (LPDATA_SAVE)lpUserData;

      LPEXHAUST_ADDING lpAddExhaust = new EXHAUST_ADDING;

      // Fetch the LPFLAG_ADDING;
      if( lpDataSave->bBufferSize == sizeof( EXHAUST_ADDING ) )
      {
         memcpy( lpAddExhaust, lpDataSave->lpbData, sizeof( EXHAUST_ADDING ) );
         // Get the spell
         LPSPELL_STRUCT lpSpell = SpellMessageHandler::GetSpell( lpAddExhaust->spellID );

         if( lpSpell != NULL )
         {
            // Create an effect status update for the client.
            CreateEffectStatus(
               self,
               lpAddExhaust->spellID,
               ( lpUnitEffect->dwTimer - TFCMAIN::GetRound() ) * 50,
               lpUnitEffect->dwTotalDuration,
               lpSpell
               );
         }
      }
      // Attach the flag adding structure to the effect.
      lpUnitEffect->lpData = lpAddExhaust;
   }
   // Effect is getting destroyed.
   else if( wMessageID == MSG_OnDestroy )
   {
      // Delete the AddFlag structure associated to the effect.
      LPEXHAUST_ADDING lpExhaustFlag = (LPEXHAUST_ADDING)lpEffectData;
      if (lpExhaustFlag != NULL)
      {
         delete lpExhaustFlag;
         lpExhaustFlag = NULL;
      }
   }
}


/******************************************************************************/
//  Creates an instance of the exhaust effect.
SpellEffect *ExhaustEffect::NewFunc(LPSPELL_STRUCT lpSpell)
/******************************************************************************/
{
   // Register this spell's effect function.
   REGISTER_EFFECT( lpSpell->dwNextEffectID, ExhaustRemoval );
   CREATE_EFFECT_HANDLE( ExhaustEffect, 0 );
}
