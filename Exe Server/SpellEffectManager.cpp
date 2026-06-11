/******************************************************************************
Modify for vs2008 (31/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "TFC Server.h"
#include "SpellEffectManager.h"
#include "IntlText.h"

#ifdef _DEBUG
	#undef THIS_FILE
	static char THIS_FILE[]=__FILE__;
	#define new DEBUG_NEW
#endif

/******************************************************************************/
TemplateList < SpellEffectManager::REGISTERED_SUPERSTRUCTURE > SpellEffectManager::tlRegisteredEffects;

/******************************************************************************/
// Destroys the spell effect manager.
void SpellEffectManager::Destroy( void )
/******************************************************************************/
{
    tlRegisteredEffects.AnnihilateList();
}
/******************************************************************************/
// Registers a superstructure
void SpellEffectManager::RegisterSuperstructure(
 LPNEW_FUNC lpFunc, // Functions which will return the handle to the specific object.
 WORD wID           // The ID of the effect superstructure to fetch
)
/******************************************************************************/
{
    REGISTERED_SUPERSTRUCTURE *lpReg = new REGISTERED_SUPERSTRUCTURE;

    TRACE( "\r\nRegistering spell structure %u.", wID );

    lpReg->lpFunc = lpFunc;
    lpReg->wID    = wID;

    tlRegisteredEffects.AddToHead( lpReg );
}
/******************************************************************************/
// Returns an instance of the derived effect class.
SpellEffect *SpellEffectManager::GetEffectObject(
 WORD wID,                  // ID of the effect superstructure to fetch an object from.
 LPSPELL_STRUCT lpSpell     // Parent spell structure.
)
/******************************************************************************/
{
    SpellEffect *lpEffect = NULL;

    tlRegisteredEffects.ToHead();

    while( tlRegisteredEffects.QueryNext() && lpEffect == NULL )
	{
        if( tlRegisteredEffects.Object()->wID == wID )
		{
            lpEffect = tlRegisteredEffects.Object()->lpFunc( lpSpell );
        }
    }

    return lpEffect;
}