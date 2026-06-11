/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#pragma hdrstop
#include "stdafx.h"
#include "DetectHidden.h"
#include "../TFC_MAIN.h"

/******************************************************************************/
REGISTER_SPELL_EFFECT( DEHIDDEN, DetectHidden::NewFunc, DETECT_HIDDEN_EFFECT, __noop );

/******************************************************************************/
namespace
{
	struct InvisEFFECT
	{
		DWORD popupEffect;
		DWORD effectID;
		DWORD spellID;
	};
};

/******************************************************************************/
DetectHidden::DetectHidden()
/******************************************************************************/
{
   popoffVisualEffect = 0;
}
/******************************************************************************/
DetectHidden::~DetectHidden()
/******************************************************************************/
{
}
/******************************************************************************/
BOOL DetectHidden::InputParameter(CString csParam, WORD wParamID)
/******************************************************************************/
{
    const int Success = 1;
    const int PopoffVisual = 2;
    switch( wParamID )
	{
		case Success:
			if( !successPercentage.SetFormula( csParam ) )
			{
				return FALSE;
			}
			break;
		case PopoffVisual:
			popoffVisualEffect = atoi( csParam );
			break;
		default:
			return FALSE;
	}
	return TRUE;
}
/******************************************************************************/
// Called when timer expires and flag must be removed.
void DetectHidden::DetectHiddenRemoval(EFFECT_FUNC_PROTOTYPE)
/******************************************************************************/
{
	// Timer make the call to the function.
	if( wMessageID == MSG_OnTimer )
	{
        InvisEFFECT *invisEffect = (InvisEFFECT *)lpEffectData;

        // Remove DetectInvisibility
        self->RemoveFlag( __FLAG_DETECT_HIDDEN );

        DispellEffectStatus( self, invisEffect->spellID ,3);

        // Make all invisible units dissappear.
        DetectInvisibility::SendInvisibleList( __FLAG_HIDDEN, self, invisEffect->popupEffect );
    }
	// If the spell got dispelled
   else if( wMessageID == MSG_OnDispell )//now destroy effect directly here
   {
      InvisEFFECT *invisEffect = (InvisEFFECT *)lpEffectData;

      // Remove DetectInvisibility
      self->RemoveFlag( __FLAG_DETECT_HIDDEN );


      DispellEffectStatus( self, invisEffect->spellID ,4); 

      // Make all invisible units dissappear.
      DetectInvisibility::SendInvisibleList( __FLAG_HIDDEN, self, invisEffect->popupEffect );        

      // Remove the effect.
      //self->Remove_Effect( dwEffect );

      if (invisEffect != NULL)
      {
         delete invisEffect;
         invisEffect = NULL;
      }
   }
   // Player saves.
    else if( wMessageID == MSG_OnSavePlayer )
	{
        // Fetch the pointers
        InvisEFFECT *invisEffect = (InvisEFFECT *)lpEffectData;
		LPDATA_SAVE lpDataSave = (LPDATA_SAVE)lpUserData;
		// Prepare buffer to save on player
        lpDataSave->bBufferSize = sizeof( InvisEFFECT );
        lpDataSave->lpbData = new BYTE[ sizeof( InvisEFFECT ) ];        
        memcpy( lpDataSave->lpbData, invisEffect, sizeof( InvisEFFECT ) );        
    }
    // Player loads
    else if( wMessageID == MSG_OnLoadPlayer )
	{
        LPUNIT_EFFECT lpUnitEffect = (LPUNIT_EFFECT)lpEffectData;
        LPDATA_SAVE lpDataSave = (LPDATA_SAVE)lpUserData;

        InvisEFFECT *invisEFFECT = new InvisEFFECT;

        // Fetch the LPFLAG_ADDING;
        if( lpDataSave->bBufferSize == sizeof( InvisEFFECT ) )
		{
            memcpy( invisEFFECT, lpDataSave->lpbData, sizeof( InvisEFFECT ) );
            // Get the spell
            LPSPELL_STRUCT lpSpell = SpellMessageHandler::GetSpell( invisEFFECT->spellID );

            if( lpSpell != NULL )
			{
                // Create an effect status update for the client.
                CreateEffectStatus(
                    self,
                    invisEFFECT->spellID,
                    lpUnitEffect->dwTotalDuration == 0xFFFFFFFF ? 0xFFFFFFFF : ( lpUnitEffect->dwTimer - TFCMAIN::GetRound() ) * 50,
                    lpUnitEffect->dwTotalDuration,
                    lpSpell
                );
            }
        }

        // Attach the flag adding structure to the effect.
        lpUnitEffect->lpData = invisEFFECT;
    }
    // Effect is getting destroyed.
    else if( wMessageID == MSG_OnDestroy )
    {
       // Delete the AddFlag structure associated to the effect.
       InvisEFFECT *invisEffect = (InvisEFFECT *)lpEffectData;
       if (invisEffect != NULL)
       {
          delete invisEffect;
          invisEffect = NULL;
       }
    }
}
/******************************************************************************/
void DetectHidden::CallEffect(SPELL_EFFECT_PROTOTYPE)
/******************************************************************************/
{
   TRACE("***CallEffect\n");
    if( target == NULL )
	{
        return;
    }

    // If the spell fails.
    static Random rnd;
    if( rnd( 1, 100 ) >= successPercentage.GetBoost( self, target, 0, 0, range ) )
	{
        return;
    }
    // Get duration now.
    DWORD dwDuration = lpSpell->bfDuration.GetBoost( self, target );
    if( dwDuration == 0 )
	{
        return;
    }
        
    // Remove the previous effect
    target->RemoveEffect( dwEffectID );
    
    // Add the DetectInvisibility flag
    target->SetFlag( __FLAG_DETECT_HIDDEN, 1 );

    DetectInvisibility::SendNewlyVisibleList( __FLAG_HIDDEN, self, popoffVisualEffect );

    // Setup the add flag
    InvisEFFECT *invisEffect = new InvisEFFECT;
    invisEffect->popupEffect = popoffVisualEffect;
    invisEffect->spellID = lpSpell->wSpellID;
    invisEffect->effectID = dwEffectID;

    // Create an effect status update for the client.
    CreateEffectStatus(
        target,
        lpSpell->wSpellID,
        dwDuration,
        dwDuration,
        lpSpell
    );
    
    CREATE_EFFECT(
        target,
        MSG_OnTimer,
        dwEffectID,
        DetectHiddenRemoval,
        invisEffect,
        dwDuration MILLISECONDS TDELAY,
        dwDuration,
        lpSpell->wSpellID,
        __FLAG_DETECT_HIDDEN
    );
}
/******************************************************************************/
SpellEffect *DetectHidden::NewFunc(LPSPELL_STRUCT lpSpell)
/******************************************************************************/
{
    REGISTER_EFFECT( lpSpell->dwNextEffectID, DetectHiddenRemoval );
   
    CREATE_EFFECT_HANDLE( DetectHidden, 1 )
}
