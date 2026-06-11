/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#if !defined(AFX_REVOKESHOUTS_H__8FD2D296_B1E0_439A_9F81_306CC0992D80__INCLUDED_)
#define AFX_REVOKESHOUTS_H__8FD2D296_B1E0_439A_9F81_306CC0992D80__INCLUDED_

#if _MSC_VER >= 1000
	#pragma once
#endif // _MSC_VER > 1000

#include "..\SpellEffectManager.h"

/******************************************************************************/
class RevokeShouts : SpellEffect 
/******************************************************************************/
{
public:
	RevokeShouts();
	virtual ~RevokeShouts();
	BOOL InputParameter( CString csParam, WORD wParamID );
    void CallEffect( SPELL_EFFECT_PROTOTYPE );
    static SpellEffect *NewFunc( LPSPELL_STRUCT lpSpell );
    static void BoostRemoval( EFFECT_FUNC_PROTOTYPE );

private:
	WORD wRevokeWhat;	
};

#endif // !defined(AFX_REVOKESHOUTS_H__8FD2D296_B1E0_439A_9F81_306CC0992D80__INCLUDED_)
