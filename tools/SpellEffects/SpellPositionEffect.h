/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#if !defined(AFX_SPELLPOSITION_H__D3A55185_0F58_11D2_8362_00E02922FA40__INCLUDED_)
#define AFX_SPELLPOSITION_H__D3A55185_0F58_11D2_8362_00E02922FA40__INCLUDED_

#if _MSC_VER >= 1000
	#pragma once
#endif // _MSC_VER >= 1000

#include "../SpellEffectManager.h"

/******************************************************************************/
class SpellPositionEffect : public SpellEffect  
/******************************************************************************/
{
public:
	SpellPositionEffect();
	virtual ~SpellPositionEffect();
    BOOL InputParameter( CString csParam, WORD wParamID );
    void CallEffect( SPELL_EFFECT_PROTOTYPE );
    static void Init( void );
    static SpellEffect *NewFunc( LPSPELL_STRUCT lpSpell );

private:
    DWORD dwSpellID;
    DWORD dwFreq;
    DWORD dwNbrRepeat;
};

#endif // !defined(AFX_SPELLPOSITION_H__D3A55185_0F58_11D2_8362_00E02922FA40__INCLUDED_)
