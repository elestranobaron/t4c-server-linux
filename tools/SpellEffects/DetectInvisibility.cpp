/******************************************************************************
Modify for vs2008 (26/04/2009)
Add Invisibility 2 by Nightmare 928/06/2009)
/******************************************************************************/
#include "stdafx.h"
#include "DetectInvisibility.h"
#include "../TFC_MAIN.h"

/******************************************************************************/
REGISTER_SPELL_EFFECT( DEINVIS, DetectInvisibility::NewFunc, DETECT_INVISIBILITY_EFFECT, __noop );

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
DetectInvisibility::DetectInvisibility()
/******************************************************************************/
{
   popoffVisualEffect = 0;
}
/******************************************************************************/
DetectInvisibility::~DetectInvisibility()
/******************************************************************************/
{
}
/******************************************************************************/
BOOL DetectInvisibility::InputParameter(CString csParam, WORD wParamID)
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
//  Sends a remove unit for all invisible units on the screen
void DetectInvisibility::SendInvisibleList(DWORD flagId, Unit *self, DWORD popup)
/******************************************************************************/
{
    WorldMap *wl = TFCMAIN::GetWorld( self->GetWL().world );

    if( wl == NULL )
	{
        return;
    }

    // Get all units within the default range.
    TemplateList< Unit > *unitList = wl->GetLocalUnits( self->GetWL(), 30, FALSE );

    if( unitList == NULL )
	{
        return;
    }
    
    unitList->ToHead();
    while( unitList->QueryNext() )
	{
        Unit *un = unitList->Object();

        // If the unit is invisible.
        if( un->ViewFlag( flagId ) != 0 )
		{
            if( popup != 0 )
			{
                WorldPos wlPos = self->GetWL();

                TFCPacket sending;
                // Display the dissappearing effect.
                sending << (RQ_SIZE)__EVENT_SPELL_EFFECT;
                sending << (short)popup;
                sending << (long) un->GetID();
                sending << (long) un->GetID();
                sending << (short)wlPos.X;
                sending << (short)wlPos.Y;
                sending << (short)wlPos.X;
                sending << (short)wlPos.Y;
                sending << (long)GetNextGlobalEffectID();
                sending << (long)0;

                self->SendPlayerMessage( sending );
            }

            // Make it dissappear.
            TFCPacket sending;
            sending << (RQ_SIZE)__EVENT_OBJECT_REMOVED;
            sending << (char)0;
            sending << (long)un->GetID();

            self->SendPlayerMessage( sending );
        }
    }

	if (unitList != NULL)
	{
		delete unitList;
		unitList = NULL;
	}
}
/******************************************************************************/
//  Sends the list of all invisible players, now visible
void DetectInvisibility::SendNewlyVisibleList(DWORD flagId, Unit *self, DWORD popup)
/******************************************************************************/
{
   WorldMap *wl = TFCMAIN::GetWorld( self->GetWL().world );

   if( wl == NULL )
   {
      return;
   }

   // Get all units within the default range.
    TemplateList< Unit > *unitList = wl->GetLocalUnits( self->GetWL(), 30, FALSE );

    if( unitList == NULL )
    {
       return;
    }

    unitList->ToHead();
    while( unitList->QueryNext() )
    {
       Unit *un = unitList->Object();

       // If the unit is invisible.
       if( un->ViewFlag( flagId ) != 0 )
       {
          // Make it dissappear.
          TFCPacket sending;
          un->PacketPopup( un->GetWL(), sending );
          self->SendPlayerMessage( sending );

          if( popup != 0 )
          {
             WorldPos wlPos = self->GetWL();

             // Display the dissappearing effect.
             sending << (RQ_SIZE)__EVENT_SPELL_EFFECT;
             sending << (short)popup;
             sending << (long) un->GetID();
             sending << (long) un->GetID();
             sending << (short)wlPos.X;
             sending << (short)wlPos.Y;
             sending << (short)wlPos.X;
             sending << (short)wlPos.Y;
             sending << (long)GetNextGlobalEffectID();
             sending << (long)0;

             self->SendPlayerMessage( sending );
          }
       }
    }

	if (unitList != NULL)
	{
		delete unitList;
		unitList = NULL;
	}
}

//  Sends the list of all invisible players, now visible
void DetectInvisibility::SendNewlyVisible2List(DWORD flagId,Unit *self)
{
   WorldMap *wl = TFCMAIN::GetWorld( self->GetWL().world );
   
   if( wl == NULL )
   {
      return;
   }
   
   // Get all units within the default range.
   TemplateList< Unit > *unitList = wl->GetLocalUnits( self->GetWL(), 30, FALSE );
   
   if( unitList == NULL )
   {
      return;
   }
   
   unitList->ToHead();
   while( unitList->QueryNext() )
   {
      Unit *un = unitList->Object();
      
      // If the unit is invisible.
      if( un->ViewFlag( flagId ) != 0 )
      {
         // Make it dissappear.
         TFCPacket sending;
         un->PacketPopup( un->GetWL(), sending );
         self->SendPlayerMessage( sending );
      }
   }
   
   if (unitList != NULL)
   {
		delete unitList;
		unitList = NULL;
   }
}

/******************************************************************************/
// Called when timer expires and flag must be removed.
void DetectInvisibility::DetectInvisibilityRemoval(EFFECT_FUNC_PROTOTYPE)
/******************************************************************************/
{
	// Timer make the call to the function.
	if( wMessageID == MSG_OnTimer )
	{
        InvisEFFECT *invisEffect = (InvisEFFECT *)lpEffectData;

        // Remove DetectInvisibility
        self->RemoveFlag( __FLAG_DETECT_INVISIBILITY );

        DispellEffectStatus( self, invisEffect->spellID ,5);

        // Make all invisible units dissappear.
        SendInvisibleList( __FLAG_INVISIBILITY, self, invisEffect->popupEffect );
        SendInvisibleList( __FLAG_INVISIBILITY2, self, invisEffect->popupEffect );
    }
    // If the spell got dispelled
    else if( wMessageID == MSG_OnDispell )//now destroy effect directly here
	{
        InvisEFFECT *invisEffect = (InvisEFFECT *)lpEffectData;

        // Remove DetectInvisibility
        self->RemoveFlag( __FLAG_DETECT_INVISIBILITY );

        DispellEffectStatus( self, invisEffect->spellID ,6);

        // Make all invisible units dissappear.
        SendInvisibleList( __FLAG_INVISIBILITY, self, invisEffect->popupEffect );        
        SendInvisibleList( __FLAG_INVISIBILITY2, self, invisEffect->popupEffect );

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
    else if( wMessageID == MSG_OnDestroy ){
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
void DetectInvisibility::CallEffect(SPELL_EFFECT_PROTOTYPE)
/******************************************************************************/
{
   TRACE("***DetectInvisibility\n");
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
    target->SetFlag( __FLAG_DETECT_INVISIBILITY, 1 );

    SendNewlyVisibleList( __FLAG_INVISIBILITY, self, popoffVisualEffect );
    SendNewlyVisible2List( __FLAG_INVISIBILITY2, self);

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
        DetectInvisibilityRemoval,
        invisEffect,
        dwDuration MILLISECONDS TDELAY,
        dwDuration,
        lpSpell->wSpellID,
        __FLAG_DETECT_INVISIBILITY
    );
}
/******************************************************************************/
SpellEffect *DetectInvisibility::NewFunc(LPSPELL_STRUCT lpSpell)
/******************************************************************************/
{
    REGISTER_EFFECT( lpSpell->dwNextEffectID, DetectInvisibilityRemoval );
    
    CREATE_EFFECT_HANDLE( DetectInvisibility, 1 )
}
