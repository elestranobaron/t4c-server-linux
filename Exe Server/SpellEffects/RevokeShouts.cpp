/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "RevokeShouts.h"
#include "../Unit.h"
#include "../Players.h"
#include "../T4CLog.h"
#include "../TFC_MAIN.h"

/******************************************************************************/
// Compare
#define SAME( __text )  csParam.CompareNoCase( __text ) == 0

// Boost struct
typedef struct _ADD_BOOST
{
    DWORD dwBoostID;
    DWORD dwSpellID;
} ADD_BOOST, *LPADD_BOOST;

/******************************************************************************/
REGISTER_SPELL_EFFECT( REVOKE_UNIT, RevokeShouts::NewFunc, REVOKE_UNIT_EFFECT, __noop );

/******************************************************************************/
RevokeShouts::RevokeShouts()
/******************************************************************************/
{
	wRevokeWhat = -1;
}
/******************************************************************************/
RevokeShouts::~RevokeShouts()
/******************************************************************************/
{
}
/******************************************************************************/
// Initialization
BOOL RevokeShouts::InputParameter( CString csParam, WORD wParamID )
/******************************************************************************/
{
	BOOL boReturn = TRUE;

	if( SAME( "Shouts" ) )			wRevokeWhat = 1;
	else if( SAME( "Pages" ) )		wRevokeWhat = 2;
	else if( SAME( "Overhead" ) )	wRevokeWhat = 3;
	else boReturn = FALSE;

	return boReturn;
}
/******************************************************************************/
// When spell is casted
void RevokeShouts::CallEffect( SPELL_EFFECT_PROTOTYPE )
/******************************************************************************/
{
    TRACE("***RevokeShouts\n");
	// Only if caster is a player and target not null
	if( self != NULL && self->GetType() == U_PC && target != NULL )
	{
		Character *lpChTarget  = static_cast< Character* >( target );
        Character *lpCharacter = static_cast< Character* >( self );

        Players   *lpPlayer = lpCharacter->GetPlayer();
		Players   *lpTarget = lpChTarget->GetPlayer();
		if( lpPlayer!= NULL && lpTarget != NULL && lpPlayer->GetGodFlags() & GOD_CAN_REMOVE_SHOUTS )
		{
			// Remove previous effect
			target->RemoveEffect( dwEffectID );

			// Get effect duration
			DWORD dwDuration = lpSpell->bfDuration.GetBoost( self, target );

			if( dwDuration != 0 )
			{				
				if( wRevokeWhat == 1 )
				{
					lpTarget->boCanShout = FALSE;

					_LOG_GAMEOP
						LOG_SYSOP,
						"Gameop %s (%s) cast revoke shouts on unit named %s.",
						(LPCTSTR)lpCharacter->GetTrueName(),
						(LPCTSTR)lpPlayer->GetFullAccountName(),
						(LPCTSTR)target->GetName( _DEFAULT_LNG )
					LOG_
				}
				else if( wRevokeWhat == 2 )
				{
					lpTarget->boCanPage = FALSE;

					_LOG_GAMEOP
						LOG_SYSOP,
						"Gameop %s (%s) cast revoke pages on unit named %s.",
						(LPCTSTR)lpCharacter->GetTrueName(),
						(LPCTSTR)lpPlayer->GetFullAccountName(),
						(LPCTSTR)target->GetName( _DEFAULT_LNG )
					LOG_
				}
				else if( wRevokeWhat == 3 )
				{
					lpTarget ->boCanTalk = FALSE;

					_LOG_GAMEOP
						LOG_SYSOP,
						"Gameop %s (%s) cast revoke overheads on unit named %s.",
						(LPCTSTR)lpCharacter->GetTrueName(),
						(LPCTSTR)lpPlayer->GetFullAccountName(),
						(LPCTSTR)target->GetName( _DEFAULT_LNG )
					LOG_
				}

				_LOG_GAMEOP
					LOG_SYSOP,
					"New struct (spellID = %d).",
					lpSpell->wSpellID
				LOG_

				// Setup the add boost structure
                LPADD_BOOST lpAddBoost = new ADD_BOOST;
                lpAddBoost->dwBoostID = dwEffectID;
                lpAddBoost->dwSpellID = lpSpell->wSpellID;

				_LOG_GAMEOP
					LOG_SYSOP,
					"Create effect status. (%d ms)",
					dwDuration
				LOG_

				// Create an effect status update for client
				CreateEffectStatus( target, lpSpell->wSpellID, dwDuration, dwDuration, lpSpell );
				// Setup the effect on the target

				_LOG_GAMEOP
					LOG_SYSOP,
					"Create effect. %d)",
					lpSpell->wSpellID
				LOG_

				CREATE_EFFECT( target, MSG_OnTimer, dwEffectID, BoostRemoval, lpAddBoost, dwDuration MILLISECONDS TDELAY, dwDuration, lpSpell->wSpellID, 0 );
			}
		}
	}
}
/******************************************************************************/
// Called when timer expires
void RevokeShouts::BoostRemoval( EFFECT_FUNC_PROTOTYPE )
/******************************************************************************/
{
	if( wMessageID == MSG_OnTimer )
	{
		_LOG_GAMEOP
			LOG_SYSOP,
			"OnTimer."
		LOG_

		LPADD_BOOST lpAddBoost = (LPADD_BOOST)lpEffectData;

		Character* ch = static_cast< Character* >( self );
		Players* lpPlayer = ch->GetPlayer();

		lpPlayer->boCanShout = TRUE;
		lpPlayer->boCanPage = TRUE;
		lpPlayer->boCanTalk = TRUE;

		_LOG_GAMEOP
			LOG_SYSOP,
			"Removing."
		LOG_

		// Remove the effect from the client.
        DispellEffectStatus( self, lpAddBoost->dwSpellID ,17);

		_LOG_GAMEOP
			LOG_SYSOP,
			"Removed."
		LOG_
	}
	else if( wMessageID == MSG_OnDispell )//now destroy effect directly here
	{
		_LOG_GAMEOP
			LOG_SYSOP,
			"OnDispell."
		LOG_

		Character* ch = static_cast< Character* >( self );
		Players* lpPlayer = ch->GetPlayer();

		lpPlayer->boCanShout = TRUE;
		lpPlayer->boCanPage = TRUE;
		lpPlayer->boCanTalk = TRUE;

		_LOG_GAMEOP
			LOG_SYSOP,
			"Removing."
		LOG_

      // Remove the effect from the client.
      LPADD_BOOST lpAddBoost = (LPADD_BOOST)lpEffectData;
      DispellEffectStatus( self, lpAddBoost->dwSpellID ,18);

      _LOG_GAMEOP
         LOG_SYSOP,
         "Removed (%d).",
         lpAddBoost->dwSpellID
      LOG_

      // Remove the effect
      //self->Remove_Effect( dwEffect );

      if(lpAddBoost)
         delete lpAddBoost;
      lpAddBoost = NULL;

	}
	else if( wMessageID == MSG_OnSavePlayer )
	{
		_LOG_GAMEOP
			LOG_SYSOP,
			"OnSave."
		LOG_

		LPADD_BOOST lpAddBoost = (LPADD_BOOST)lpEffectData;
		LPDATA_SAVE lpDataSave = (LPDATA_SAVE)lpUserData;
		// Prepare buffer to save on player
        lpDataSave->bBufferSize = sizeof( ADD_BOOST );
        lpDataSave->lpbData = new BYTE[ sizeof( ADD_BOOST ) ];        
        memcpy( lpDataSave->lpbData, lpAddBoost, sizeof( ADD_BOOST ) );

		_LOG_GAMEOP
			LOG_SYSOP,
			"Saved. (%d)",
			lpAddBoost->dwSpellID
		LOG_
	}
	else if( wMessageID == MSG_OnLoadPlayer )
	{
		_LOG_GAMEOP
			LOG_SYSOP,
			"OnLoad."
		LOG_

		LPUNIT_EFFECT lpUnitEffect = (LPUNIT_EFFECT)lpEffectData;
        LPDATA_SAVE lpDataSave = (LPDATA_SAVE)lpUserData;
		
		LPADD_BOOST lpAddBoost = new ADD_BOOST;

        // Fetch the LPADD_BOOST
        if( lpDataSave->bBufferSize == sizeof( ADD_BOOST ) )
		{
			memcpy( lpAddBoost, lpDataSave->lpbData, sizeof( ADD_BOOST ) );

            // Get the spell
            LPSPELL_STRUCT lpSpell = SpellMessageHandler::GetSpell( lpAddBoost->dwSpellID );

            if( lpSpell != NULL )
			{
				_LOG_GAMEOP
					LOG_SYSOP,
					"CreateEffect %d.",
					lpAddBoost->dwSpellID
				LOG_

                // Create an effect status update for the client.
                CreateEffectStatus( self, lpAddBoost->dwSpellID,
                    lpUnitEffect->dwTotalDuration == 0xFFFFFFFF ? 0xFFFFFFFF : ( lpUnitEffect->dwTimer - TFCMAIN::GetRound() ) * 50,
                    lpUnitEffect->dwTotalDuration, lpSpell );
            }
        }
		
		// Attach the boost structure to the effect.
        lpUnitEffect->lpData = lpAddBoost;
    }
   else if( wMessageID == MSG_OnDestroy )
   {
      _LOG_GAMEOP
         LOG_SYSOP,
         "OnDestroy."
         LOG_

      LPADD_BOOST lpAddBoost = (LPADD_BOOST)lpEffectData;
      if(lpAddBoost)
         delete lpAddBoost;
      lpAddBoost = NULL;
   }
}
/******************************************************************************/
// Create spelleffect object
SpellEffect* RevokeShouts::NewFunc( LPSPELL_STRUCT lpSpell )
/******************************************************************************/
{
	// Register this spell's effect function.
  REGISTER_EFFECT( lpSpell->dwNextEffectID, BoostRemoval );
   
  CREATE_EFFECT_HANDLE( RevokeShouts, 1 )
}