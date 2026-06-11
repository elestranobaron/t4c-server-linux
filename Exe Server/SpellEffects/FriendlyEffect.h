/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#if !defined(AFX_FRIENDLY_H__D3A55185_0F58_11D2_8362_00E02922FA40__INCLUDED_)
#define AFX_FRIENDLY_H__D3A55185_0F58_11D2_8362_00E02922FA40__INCLUDED_

#if _MSC_VER >= 1000
	#pragma once
#endif // _MSC_VER >= 1000

#include "../SpellEffectManager.h"

/******************************************************************************/
class FriendlyEffect : public SpellEffect  
/******************************************************************************/
{
public:
	FriendlyEffect();
	virtual ~FriendlyEffect();
    BOOL InputParameter( CString csParam, WORD wParamID );
    void CallEffect( SPELL_EFFECT_PROTOTYPE );
    static void Init( void );
    static SpellEffect *NewFunc( LPSPELL_STRUCT lpSpell );

private:
    DWORD dwMonsterFID;
    DWORD dwValue;
};

#endif // !defined(AFX_FRIENDLY_H__D3A55185_0F58_11D2_8362_00E02922FA40__INCLUDED_)
