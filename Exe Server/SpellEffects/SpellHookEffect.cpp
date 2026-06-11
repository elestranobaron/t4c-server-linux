/******************************************************************************
Modify for vs2008 (26/04/2009)
Add spell hook on OnEquip and OnUnequip message by NightMare (28/06/2009)
/******************************************************************************/
#include "stdafx.h"
#include "SpellHookEffect.h"
#include "../SpellMessageHandler.h"
#include "../T4CLog.h"
#include "../tfc_main.h"
#include "../random.h"

/******************************************************************************/
static Random rnd;

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
REGISTER_SPELL_EFFECT( SPELL, SpellHookEffect::NewFunc, SPELL_HOOK_EFFECT, __noop );

/******************************************************************************/
SpellHookEffect::SpellHookEffect()
/******************************************************************************/
{ 
   wSpellID   = 0;
   wHook      = 0;
   dwEffectID = 0;
}
/******************************************************************************/
SpellHookEffect::~SpellHookEffect()
/******************************************************************************/
{
}
/******************************************************************************/
// Inputs parameters
BOOL SpellHookEffect::InputParameter(
 CString csParam,   // The parameter
 WORD wParamID      // Its ID.
)
/******************************************************************************/
{
    const int SpellHook = 2;
    const int SpellID = 1;
    const int SpellHookChance = 3;
    const int InitialDelay = 4;

    BOOL boReturn = TRUE;

    switch( wParamID )
	{
		case SpellHook:
			{  
            if( csParam.CompareNoCase( "OnAttack" ) == 0 )			wHook = MSG_OnAttack;
            else if( csParam.CompareNoCase( "OnAttacked" ) == 0 )   wHook = MSG_OnAttacked;
            else if( csParam.CompareNoCase( "OnTimer" ) == 0 )		wHook = MSG_OnTimer;
            else if( csParam.CompareNoCase( "OnDisturbed" ) == 0 )	wHook = MSG_OnDisturbed;
            else if( csParam.CompareNoCase( "OnEquip" ) == 0 )      wHook = MSG_OnEquip;
            else if( csParam.CompareNoCase( "OnUnequip" ) == 0 )    wHook = MSG_OnUnequip;
            else if( csParam.CompareNoCase( "OnHit" ) == 0 )		wHook = MSG_OnHit;
            else if( csParam.CompareNoCase( "OnAttackHit" ) == 0 )	wHook = MSG_OnAttackHit;
            else if( csParam.CompareNoCase( "OnDeath" ) == 0 )		wHook = MSG_OnDeath;		
				else boReturn = FALSE;
			}
			break;
		case SpellID:
			{
				wSpellID = atoi( (LPCTSTR)csParam );
				TRACE( "\r\nwSpellID=%u.", wSpellID );
				if( wSpellID == 0 )
				{
					boReturn = FALSE;
				}
			} 
			break;
		case SpellHookChance:
			{
				if( !cSpellHookChance.SetFormula( csParam ) )
				{
					boReturn = FALSE;
				}
			}
			break;
		case InitialDelay:
			{
				if( !initialDelay.SetFormula( csParam ) )
				{
					boReturn = FALSE;
				}
			}
			break;
		default:
			boReturn = FALSE;
			break;
	}
    return boReturn;
}
/******************************************************************************/
// Hooks a spell on a unit.
void SpellHookEffect::CallEffect(SPELL_EFFECT_PROTOTYPE)
/******************************************************************************/
{
   TRACE("***SpellHookEffect\n");
   if( target != NULL )
   {
      // If spell hook can be hooked.
	  int iSuccess = (int)cSpellHookChance.GetBoost( self, target );
      if( rnd( 0, 100 ) <= iSuccess && iSuccess >0  )
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
            SpellTrigger,
            lpSpellHook,
            dwInitialDelay,
            dwSpellDuration,
            lpSpell->wSpellID,
            0
            );			
      }
   }
}
/******************************************************************************/
// Creates a SpellEffect object.
SpellEffect *SpellHookEffect::NewFunc(LPSPELL_STRUCT lpSpell) // The spell structure.
/******************************************************************************/
{
    // Register this spell's effect function.
    REGISTER_EFFECT( lpSpell->dwNextEffectID, SpellTrigger );    
    
    CREATE_EFFECT_HANDLE( SpellHookEffect, 1 )
}
/******************************************************************************/
// Called when spell hook is triggered.
void SpellHookEffect::SpellTrigger(EFFECT_FUNC_PROTOTYPE)
/******************************************************************************/
{
    LPSPELLHOOK lpSpellHook = (LPSPELLHOOK)( lpEffectData );	

    if( lpEffectData == NULL ){
        return;        
    }

    if( lpSpellHook == NULL ){
       return;        
    }

	/*
	if( wMessageID == MSG_OnDeath )
	{
		// If the hook must be called on Death
		if( lpSpellHook->wHook == MSG_OnDeath )
		{
			self->SendSystemMessage( "OnDeath2" );
			
			lpSpellHook->wHook = MSG_OnTimer;
			lpSpellHook->dwTotalDuration = 2000;
			lpSpellHook->dwFrequency = 0;
		}
	}
	*/

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
                    SpellTrigger,
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
                DispellEffectStatus( self, lpSpellHook->wOriginalSpellID ,19);
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
            DispellEffectStatus( self, lpSpellHook->wOriginalSpellID ,20);
        }    
    }
    // If the spell hook got dispelled.    
    else if( wMessageID == MSG_OnDispell )//now destroy effect directly here
    {
       DispellEffectStatus( self, lpSpellHook->wOriginalSpellID ,21); 

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

        // Fix duration to relative times.
        lpSpellHook->dwTotalDuration -= TFCMAIN::GetRound();

        memcpy( lpDataSave->lpbData, lpSpellHook, sizeof( SPELLHOOK ) );

        lpSpellHook->dwTotalDuration += TFCMAIN::GetRound();

		//delete lpSpellHook;
    }// If player re-enters the game.
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
        else if( lpDataSave->bBufferSize == sizeof( SPELLHOOK ) ){
            memcpy( lpSpellHook, lpDataSave->lpbData, sizeof( SPELLHOOK ) );

            // Get the spell
            LPSPELL_STRUCT lpSpell = SpellMessageHandler::GetSpell( lpSpellHook->wOriginalSpellID );

            if( lpSpell != NULL ){
                // Create an effect status update for the client.
                CreateEffectStatus(
                    self,
                    lpSpellHook->wOriginalSpellID,
                    lpSpellHook->dwTotalDuration * 50,
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

        // Reset duration to absolute times.
        lpSpellHook->dwTotalDuration += TFCMAIN::GetRound();

        // Attach the flag adding structure to the effect.
        lpUnitEffect->lpData = lpSpellHook;
    }
    // UnitEffect got destroyed.
    else if( wMessageID == MSG_OnDestroy ){
        TRACE( "\r\nDestroyed Effect..!!" );
        // Destroy binded spellhook structure.
        if (lpSpellHook != NULL) 
        {
           delete lpSpellHook;               
           lpSpellHook = NULL;
        }
    }
    // If player died.
    else if( wMessageID == MSG_OnDeath && lpSpellHook->wHook != MSG_OnDeath )
	{		
		// Remove this effect.
		self->RemoveEffect( dwEffect );

		DispellEffectStatus( self, lpSpellHook->wOriginalSpellID ,22);		
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