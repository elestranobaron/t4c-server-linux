/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#if !defined(AFX_GIVEGOLDXP_H__4E8474F2_77C7_4490_8F4E_3BF124F979E8__INCLUDED_)
#define AFX_GIVEGOLDXP_H__4E8474F2_77C7_4490_8F4E_3BF124F979E8__INCLUDED_

#if _MSC_VER > 1000
	#pragma once
#endif // _MSC_VER > 1000

#include "../SpellEffectManager.h"
#include "../BoostFormula.h"
#include "../NPCmacroScriptLng.h"

/******************************************************************************/
class GiveGoldXp: public SpellEffect
/******************************************************************************/
{
public:
	GiveGoldXp();
	virtual ~GiveGoldXp();
	BOOL InputParameter( CString csParam, WORD wParamID );
	void CallEffect( SPELL_EFFECT_PROTOTYPE );
	static SpellEffect *NewFunc( LPSPELL_STRUCT lpSpell );

private:
	enum
	{
		GOLD = 1,
		XP = 2
	};
	enum
	{
		NOT_LOADED,
		FAILED,
		LOADED
	};
	BYTE bError;
	WORD wGive;
	BoostFormula Quantity;
};

#endif // !defined(AFX_GIVEGOLDXP_H__4E8474F2_77C7_4490_8F4E_3BF124F979E8__INCLUDED_)
