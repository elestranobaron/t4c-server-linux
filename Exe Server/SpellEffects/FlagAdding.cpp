/******************************************************************************
Modify for vs2008 (26/04/2009)
Remove in CallEffect the dwDuration affect, is already read in InputParameter by Nightmare (28/06/2009)
/******************************************************************************/
#include "stdafx.h"
#include "../NPCmacroScriptLng.h"
#include "FlagAdding.h"
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

/******************************************************************************/
REGISTER_SPELL_EFFECT( FLAG_ADDING, FlagAdding::NewFunc, FLAG_ADDING_EFFECT, FlagAdding::Init );

/******************************************************************************/
FlagAdding::FlagAdding()
/******************************************************************************/
{
    dwFlagID   = 0;
    dwDuration = 0;
    dwGlobal   = 0;
}
/******************************************************************************/
FlagAdding::~FlagAdding()
/******************************************************************************/
{
}
/******************************************************************************/
// Called by REGISTER_SPELL_EFFECT to complete spell initialisation
void FlagAdding::Init( void )
/******************************************************************************/
{
}
/******************************************************************************/
// Enters the different effect parameters
BOOL FlagAdding::InputParameter(
 CString csParam,           // String parameter
 WORD wParamID              // ID of parameter
)
/******************************************************************************/
{       
    const int FlagID = 1;
    const int FlagValue = 2;
    const int Duration = 3;
	const int Global = 4;

    BOOL boOK = TRUE;
    
    switch( wParamID )
	{
		// FlagID
		case FlagID:
			dwFlagID = atoi( (LPCTSTR)csParam );

			// FlagID cannot be 0
			if( dwFlagID == 0 )
			{
				boOK = FALSE;
			}     
			break;
		// FlagValue
		case FlagValue:
			if( !flagValue.SetFormula( csParam ) )
			{
				boOK = FALSE;
			}
			break;
		// Duration
		case Duration:
			dwDuration = atoi( (LPCTSTR)csParam );
			break;
		// Global Flag //set at 1 if a global flag
		case Global:
			dwGlobal = atoi( (LPCTSTR)csParam );
			break;
		// Any other parameter is incorrect
		default:         
			boOK = FALSE; 
			break;
	}

    return boOK;
}
/******************************************************************************/
// Called when timer expires and flag must be removed.
void FlagAdding::FlagRemoval(EFFECT_FUNC_PROTOTYPE)
/******************************************************************************/
{
	// Timer make the call to the function.
	if( wMessageID == MSG_OnTimer )
	{
        LPFLAG_ADDING lpAddFlag = (LPFLAG_ADDING)lpEffectData;
        self->RemoveFlag( lpAddFlag->dwFlagID );        

        DispellEffectStatus( self, lpAddFlag->spellID ,7);
    }
    // If the spell got dispelled
    else if( wMessageID == MSG_OnDispell )//now destroy effect directly here
	{
        // Remove the flag
        LPFLAG_ADDING lpAddFlag = (LPFLAG_ADDING)lpEffectData;
        self->RemoveFlag( lpAddFlag->dwFlagID );        
                             
        DispellEffectStatus( self, lpAddFlag->spellID ,8);
        
        // Remove the effect.
        //self->Remove_Effect( dwEffect );        

        if (lpAddFlag != NULL)
        {
           delete lpAddFlag;
           lpAddFlag = NULL;
        }
    }
    // Player exited
    else if( wMessageID == MSG_OnSavePlayer )
	{
        // Fetch the pointers
        LPFLAG_ADDING lpAddFlag = (LPFLAG_ADDING)lpEffectData;
		LPDATA_SAVE lpDataSave = (LPDATA_SAVE)lpUserData;
		// Prepare buffer to save on player
        lpDataSave->bBufferSize = sizeof( FLAG_ADDING );
        lpDataSave->lpbData = new BYTE[ sizeof( FLAG_ADDING ) ];        
        memcpy( lpDataSave->lpbData, lpAddFlag, sizeof( FLAG_ADDING ) );        
    }
	// Player loads
    else if( wMessageID == MSG_OnLoadPlayer )
	{
        LPUNIT_EFFECT lpUnitEffect = (LPUNIT_EFFECT)lpEffectData;
        LPDATA_SAVE lpDataSave = (LPDATA_SAVE)lpUserData;

        LPFLAG_ADDING lpAddFlag = new FLAG_ADDING;

        // Fetch the LPFLAG_ADDING;
        if( lpDataSave->bBufferSize == sizeof( FLAG_ADDING ) )
		{
            memcpy( lpAddFlag, lpDataSave->lpbData, sizeof( FLAG_ADDING ) );
            // Get the spell
            LPSPELL_STRUCT lpSpell = SpellMessageHandler::GetSpell( lpAddFlag->spellID );

            if( lpSpell != NULL )
			{
                // Create an effect status update for the client.
                CreateEffectStatus(
                    self,
                    lpAddFlag->spellID,
                    ( lpUnitEffect->dwTimer - TFCMAIN::GetRound() ) * 50,
                    lpUnitEffect->dwTotalDuration,
                    lpSpell
                );
            }
        }
		else if( lpDataSave->bBufferSize == sizeof( FLAG_ADDINGv1 ) )
		{
            FLAG_ADDINGv1 f;
            memcpy( &f, lpDataSave->lpbData, sizeof( FLAG_ADDING ) );
            lpAddFlag->dwFlagID = f.dwFlagID;
            lpAddFlag->effectID = 0;
            lpAddFlag->spellID = 0;
        }

        // Attach the flag adding structure to the effect.
        lpUnitEffect->lpData = lpAddFlag;
    }
    // Effect is getting destroyed.
    else if( wMessageID == MSG_OnDestroy )
    {
       // Delete the AddFlag structure associated to the effect.
       LPFLAG_ADDING lpAddFlag = (LPFLAG_ADDING)lpEffectData;
       if (lpAddFlag != NULL)
       {
          delete lpAddFlag;
          lpAddFlag = NULL;
       }
    }
}
/******************************************************************************/
// Does the flag adding effect
void FlagAdding::CallEffect(SPELL_EFFECT_PROTOTYPE)
/******************************************************************************/
{
   TRACE("***FlagAdding\n");

	// If a target unit is specified
	if( target != NULL )
	{
    	// If user isn't already using this spell


		//si un effet est deja present on la flush, et on cree la nouvelle...
		LPUNIT_EFFECT pEffectTst = target->GetEffect( dwEffectID );
		if( pEffectTst != NULL )
		{
         LPFLAG_ADDING lpAddFlag = (LPFLAG_ADDING)pEffectTst->lpData;
			target->RemoveEffect( pEffectTst->dwEffect ,false);      
         if (lpAddFlag != NULL)
         {
            delete lpAddFlag;
            lpAddFlag = NULL;
         }
		}

		//if( target->GetEffect( dwEffectID ) == NULL )
		{
			int thisFlagValue = static_cast< int >(flagValue.GetBoost(self, target ));

			// The effect is simple, add the flag to the target unit.
			if(dwGlobal)
			{ 
				GiveGlobalFlag( dwFlagID, thisFlagValue );
			}
			else
			{
				target->SetFlag( dwFlagID, thisFlagValue );
            if(dwFlagID == __FLAG_PJ_VS_MONSTER_FRIENDLY) //CV:FACTIONID CHANGE
            {
               TFCPacket sending;
               sending << (RQ_SIZE)RQ_UpdateFactionID;
               sending << (short)thisFlagValue;
               target->SendPlayerMessage( sending );
            }

				// Get duration now.
				//DWORD dwDuration = static_cast< int >(lpSpell->bfDuration.GetBoost( self, target ));

				// If duration is non-null
				if( dwDuration != 0 )
				{
					// Setup the add flag
					LPFLAG_ADDING lpAddFlag = new FLAG_ADDING;
					lpAddFlag->dwFlagID = dwFlagID;
					lpAddFlag->effectID = 0;
					lpAddFlag->spellID  = lpSpell->wSpellID;

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
						FlagRemoval,
						lpAddFlag,
						dwDuration MILLISECONDS TDELAY,
						dwDuration,
						lpSpell->wSpellID,
						0
					);
				}
			}
		}
      
	}
}
/******************************************************************************/
// Returns an instance of FlagAdding
SpellEffect *FlagAdding::NewFunc(LPSPELL_STRUCT lpSpell)
/******************************************************************************/
{
	// Register this spell's effect function.
	REGISTER_EFFECT( lpSpell->dwNextEffectID, FlagRemoval );

	CREATE_EFFECT_HANDLE( FlagAdding, 1 )
}