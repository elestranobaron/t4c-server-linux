/******************************************************************************
Modify for vs2008 (26/04/2009)
Add SPAWN Items  by Nightmare (28/06/2009)
/******************************************************************************/
#if !defined(AFX_ITEMACTIONEFFECT_H__7F360D89_351A_4AC6_B238_55403EDE43BD__INCLUDED_)
#define AFX_ITEMACTIONEFFECT_H__7F360D89_351A_4AC6_B238_55403EDE43BD__INCLUDED_

#if _MSC_VER > 1000
	#pragma once
#endif // _MSC_VER > 1000

#include "..\SpellEffectManager.h"
#include "..\NPCmacroScriptLng.h"

/******************************************************************************/
class ItemActionEffect : public SpellEffect 
/******************************************************************************/
{
public:
	ItemActionEffect();
	virtual ~ItemActionEffect();
	BOOL InputParameter( CString csParam, WORD wParamID );
    void CallEffect( SPELL_EFFECT_PROTOTYPE );
    static SpellEffect *NewFunc( LPSPELL_STRUCT lpSpell );

private:
	void HandleError( DWORD dwDaItemID, BYTE bDaResType, CString csDaItemName, Unit *self, Unit *target, BYTE bmUserDefinedPosition );
	enum { NOT_LOADED, FAILED, LOADED };
	BYTE bError;

	enum { TAKE, GIVE, SPAWN };
	DWORD dwItemID;
	WorldPos wlDestinationPos; //Optional position for the summon.
	BoostFormula bfPosX, bfPosY, bfPosW; //Formulas used for position calculation.
	BYTE bmUserDefinedPosition; // 0x07 if the user specified a position for the summon.
	CString csItemName;
	BYTE bResType;
};

#endif // !defined(AFX_ITEMACTIONEFFECT_H__7F360D89_351A_4AC6_B238_55403EDE43BD__INCLUDED_)
