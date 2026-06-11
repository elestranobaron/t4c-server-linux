/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "TFC Server.h"
#include "UnitEffectManager.h"

#ifdef _DEBUG
	#undef THIS_FILE
	static char THIS_FILE[]=__FILE__;
	#define new DEBUG_NEW
#endif

/******************************************************************************/
TemplateList< UnitEffectManager::EFFECT_LIST > UnitEffectManager::tlEffectList;


/******************************************************************************/
// Registers an effect.
void UnitEffectManager::RegisterEffect
(
 DWORD dwEffect,			// The effect to register
 LPEFFECT_PROC lpFunc	// The effect proc.
)
/******************************************************************************/
{
	EFFECT_LIST *lpEffect = new EFFECT_LIST;

	lpEffect->dwEffect = dwEffect;
	lpEffect->lpFunc = lpFunc;

	tlEffectList.AddToTail( lpEffect );
}
/******************************************************************************/
// Returns the procedure affected to a spell effect.
LPEFFECT_PROC UnitEffectManager::GetEffectProc
(
 DWORD dwEffect // The effect
)
/******************************************************************************/
{
	EFFECT_LIST *lpEffect;
	BOOL boFound = FALSE;

	tlEffectList.Lock();
	tlEffectList.ToHead();
	while( tlEffectList.QueryNext() && !boFound )
	{
		lpEffect = tlEffectList.Object();

		if( lpEffect->dwEffect == dwEffect )
		{
			boFound = TRUE;
		}
	}
	tlEffectList.Unlock();

	if( boFound )
	{
		TRACE( "\r\nEffect: Found proc!" );
		return lpEffect->lpFunc;
	}

	return NULL;
}
/******************************************************************************/
// Destroys all the spell effects registered.
void UnitEffectManager::Destroy( void )
/******************************************************************************/
{
	tlEffectList.AnnihilateList();
}