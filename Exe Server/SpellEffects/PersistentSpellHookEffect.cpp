/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
// PersistentSpellHookEffect.cpp: implementation of the PersistentSpellHookEffect class.
// The difference between this and the SpellHookEffect is that this one wont wear off 
// when Characters die.
/******************************************************************************/
#include "stdafx.h"
#include "PersistentSpellHookEffect.h"
#include "../SpellMessageHandler.h"
#include "../T4CLog.h"
#include "../tfc_main.h"
#include "../random.h"

/******************************************************************************/
static Random rnd;

/******************************************************************************/
typedef struct _SPELLHOOKv1
{
    WORD   wHook;
    WORD   wSpellID;
    DWORD  dwFrequency;
    DWORD  dwEffectID;
    DWORD  dwTotalDuration;
} SPELLHOOKv1, *LPSPELLHOOKv1;

typedef struct _SPELLHOOK
{
    WORD   wHook;
    WORD   wSpellID;
    WORD   wOriginalSpellID;
    DWORD  dwFrequency;
    DWORD  dwEffectID;
    DWORD  dwTotalDuration;
} SPELLHOOK, *LPSPELLHOOK;

/******************************************************************************/
REGISTER_SPELL_EFFECT( PERSISTENTSPELL, PersistentSpellHookEffect::NewFunc, PERSISTENT_SPELL_HOOK_EFFECT, __noop );

/******************************************************************************/
PersistentSpellHookEffect::PersistentSpellHookEffect()
/******************************************************************************/
{
   
}
/******************************************************************************/
PersistentSpellHookEffect::~PersistentSpellHookEffect()
/******************************************************************************/
{
}

/******************************************************************************/
// Creates a SpellEffect object.
SpellEffect *PersistentSpellHookEffect::NewFunc(LPSPELL_STRUCT lpSpell) // The spell structure.
/******************************************************************************/
{
    // Register this spell's effect function.
    REGISTER_EFFECT( lpSpell->dwNextEffectID, PersistentSpellTrigger );    
    
    CREATE_EFFECT_HANDLE( PersistentSpellHookEffect, 1 )
}
/******************************************************************************/
// Called when spell hook is triggered.
void PersistentSpellHookEffect::PersistentSpellTrigger(EFFECT_FUNC_PROTOTYPE)
/******************************************************************************/
{
   LPSPELLHOOK lpSpellHook = (LPSPELLHOOK)( lpEffectData );

   if( lpEffectData == NULL )
   {
      return;        
   }

   if( wMessageID == MSG_OnTimer )
   {
      BOOL boKill = FALSE;

      // If this item is a timer hook.
      if( lpSpellHook->wHook == MSG_OnTimer )
      {
         WorldPos wlPos = { 0,0,0 };

         LPSPELLHOOK lpNewSpellHook = new SPELLHOOK;
         memcpy( lpNewSpellHook, lpSpellHook, sizeof( SPELLHOOK ) );
         lpSpellHook = lpNewSpellHook;

         // Check if spell is still running.
         if( lpSpellHook->dwTotalDuration > TFCMAIN::GetRound() )
         {
            // Re-create the effect using its frequency.
            CREATE_EFFECT(
               self,
               (BYTE)lpSpellHook->wHook,
               lpSpellHook->dwEffectID,
               PersistentSpellTrigger,
               lpSpellHook,
               lpSpellHook->dwFrequency MILLISECONDS TDELAY,
               lpSpellHook->dwTotalDuration,
               lpSpellHook->wOriginalSpellID,
               0
               );
         }
         else
         {
            // Otherwise kill effect. The timer will remove the effect itself.
            DispellEffectStatus( self, lpSpellHook->wOriginalSpellID ,13);
         }

         // Activate the hooked spell only after its re-instatiation has been done.
         SpellMessageHandler::ActivateSpell(
            lpSpellHook->wSpellID,
            self,
            medium,
            target,
            wlPos
            );

      }
      else
      {
         // Otherwise destroy effect, time elapsed.
         DispellEffectStatus( self, lpSpellHook->wOriginalSpellID ,14);
      }    
   }
   // If the spell hook got dispelled.    
   else if( wMessageID == MSG_OnDispell )//now destroy effect directly here
   {
      DispellEffectStatus( self, lpSpellHook->wOriginalSpellID ,15);

      // Simply remove the effect (it won't trigger anymore).
      //self->Remove_Effect( dwEffect );
      
      if (lpSpellHook != NULL) 
      {
         delete lpSpellHook;               
         lpSpellHook = NULL;
      }
   }
   else if( wMessageID == MSG_OnSavePlayer )
   {
      DATA_SAVE *lpDataSave = (DATA_SAVE *)( lpUserData );
      // Prepare buffer to save on player
      lpDataSave->bBufferSize = sizeof( SPELLHOOK );
      lpDataSave->lpbData = new BYTE[ sizeof( SPELLHOOK ) ];

      if (lpSpellHook->dwTotalDuration != 0xFFFFFFFF) 
      {
         // Fix duration to relative times.
         lpSpellHook->dwTotalDuration -= TFCMAIN::GetRound();

         memcpy( lpDataSave->lpbData, lpSpellHook, sizeof( SPELLHOOK ) );

         lpSpellHook->dwTotalDuration += TFCMAIN::GetRound();
      }
      else 
      {
         // infinite spells need no relative time
         memcpy( lpDataSave->lpbData, lpSpellHook, sizeof( SPELLHOOK ) );
      }

      //delete lpSpellHook;
   }
   // If player re-enters the game.
   else if( wMessageID == MSG_OnLoadPlayer )
   {
      LPUNIT_EFFECT lpUnitEffect = (LPUNIT_EFFECT)lpEffectData;
      LPDATA_SAVE lpDataSave = (LPDATA_SAVE)lpUserData;

      LPSPELLHOOK lpSpellHook = new SPELLHOOK;

      // If the structure comes from the first version of the spell hook data.
      if( lpDataSave->bBufferSize == sizeof( SPELLHOOKv1 ) )
      {
         // Copy the data into the old structure.
         SPELLHOOKv1 spellHook1;
         memcpy( &spellHook1, lpDataSave->lpbData, sizeof( SPELLHOOKv1 ) );

         // Translate the old structure into the new one.
         lpSpellHook->dwEffectID = spellHook1.dwEffectID;
         lpSpellHook->dwFrequency = spellHook1.dwFrequency;
         lpSpellHook->dwTotalDuration = spellHook1.dwTotalDuration;
         lpSpellHook->wHook = spellHook1.wHook;
         lpSpellHook->wSpellID = spellHook1.wSpellID;
         lpSpellHook->wOriginalSpellID = 0;
      }
      // If this comes from the current effect data version.
      else if( lpDataSave->bBufferSize == sizeof( SPELLHOOK ) )
      {
         memcpy( lpSpellHook, lpDataSave->lpbData, sizeof( SPELLHOOK ) );

         // Get the spell
         LPSPELL_STRUCT lpSpell = SpellMessageHandler::GetSpell( lpSpellHook->wOriginalSpellID );

         if( lpSpell != NULL ){
            // Create an effect status update for the client.
            CreateEffectStatus(
               self,
               lpSpellHook->wOriginalSpellID,
               lpSpellHook->dwTotalDuration != 0xFFFFFFFF ? lpSpellHook->dwTotalDuration * 50 : 0xFFFFFFFF,
               lpUnitEffect->dwTotalDuration,
               lpSpell
               );
         }

      }
      else
      {
         _LOG_DEBUG
            LOG_DEBUG_LVL1,
            "\r\nLoading inconsistent spell hook data (size=%u).",
            lpDataSave->bBufferSize
            LOG_
      }

      if (lpSpellHook->dwTotalDuration != 0xFFFFFFFF) 
      {
         // Reset duration to absolute times.
         lpSpellHook->dwTotalDuration += TFCMAIN::GetRound();
      }

      // Attach the flag adding structure to the effect.
      lpUnitEffect->lpData = lpSpellHook;
   }
   // UnitEffect got destroyed.
   else if( wMessageID == MSG_OnDestroy )
   {
      // Destroy binded spellhook structure.
      if (lpSpellHook != NULL) 
      {
         delete lpSpellHook;               
         lpSpellHook = NULL;
      }
   }
   // If player died.
   else if( wMessageID == MSG_OnDeath )
   {
      if (self->GetType() != U_PC)
      {
         // Remove this effect.
         self->RemoveEffect( dwEffect );
         DispellEffectStatus( self, lpSpellHook->wOriginalSpellID ,16);
      }
      // For any other wMessageID corresponding, activate hooked spell.    
   }
   else
   {            
      WorldPos wlPos = { 0,0,0 };

      TRACE( "\r\n--SpellID=%u.", lpSpellHook->wSpellID );
      // Activate the hooked spell
      SpellMessageHandler::ActivateSpell(
         lpSpellHook->wSpellID,
         self,
         medium,
         target,
         wlPos
         );
   } 
}
/******************************************************************************/
// Hooks a spell on a unit.
void PersistentSpellHookEffect::CallEffect(SPELL_EFFECT_PROTOTYPE)
/******************************************************************************/
{
   TRACE("***PersistentSpellHookEffect\n");
   if( target != NULL )
   {
      // If spell hook can be hooked.
      int iSuccess = (int)cSpellHookChance.GetBoost( self, target ) ;
      if( rnd( 0, 100 ) <= iSuccess && iSuccess > 0 )
      {
         // Remove previous effect
         target->RemoveEffect( dwEffectID );

         // Get spell duration now.
         double duration = lpSpell->bfDuration.GetBoost( self, target );
         DWORD dwSpellDuration;
         if( duration > 1 * pow( 10.0, 300.0 ) )
         {
            dwSpellDuration = 0xFFFFFFFF;
         }
         else
         {
            dwSpellDuration = duration;
         }

         DWORD dwInitialDelay;

         // If this is a timer hook
         if( wHook == MSG_OnTimer )
         {
            // Then set first trigger to the initial delay.
            dwInitialDelay = initialDelay.GetBoost( self, target ) MILLISECONDS TDELAY;
         }
         else
         {
            // Otherwise use the total spell duration at the time.
            if( dwSpellDuration == 0xFFFFFFFF )
            {
               dwInitialDelay = 0xFFFFFFFF;
            }
            else
            {
               dwInitialDelay = dwSpellDuration MILLISECONDS TDELAY;
            }
         }

         // Create a spell structure.
         LPSPELLHOOK lpSpellHook = new SPELLHOOK;

         lpSpellHook->wHook = wHook;
         lpSpellHook->wSpellID = wSpellID;        
         lpSpellHook->dwFrequency = lpSpell->bfTimerFrequency.GetBoost( self, target );
         if( dwSpellDuration == 0xFFFFFFFF )
         {
            lpSpellHook->dwTotalDuration = 0xFFFFFFFF;
         }
         else
         {
            lpSpellHook->dwTotalDuration = dwSpellDuration MILLISECONDS TDELAY;
         }
         lpSpellHook->dwEffectID = dwEffectID;
         lpSpellHook->wOriginalSpellID = lpSpell->wSpellID;

         TRACE( "\r\nSpellID=%u, Frequency=%u, TotalDuration=%u, EffectID=%u.", 
            lpSpellHook->wSpellID,
            lpSpellHook->dwFrequency,
            lpSpellHook->dwTotalDuration,
            lpSpellHook->dwEffectID
            );

         // Create an effect status update for the client.
         CreateEffectStatus(
            target,
            lpSpell->wSpellID,
            dwSpellDuration,
            dwSpellDuration,
            lpSpell
            );

         CREATE_EFFECT(
            target,
            (BYTE)wHook,
            dwEffectID,
            PersistentSpellTrigger,
            lpSpellHook,
            dwInitialDelay,
            dwSpellDuration,
            lpSpell->wSpellID,
            0
            );
      }
   }
}

