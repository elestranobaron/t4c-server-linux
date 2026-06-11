// EvadeView.cpp: implementation of the EvadeView class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "EvadeView.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

EvadeView::EvadeView()
{	
	s_saAttrib.skLevel = 28;
	s_saAttrib.skAGI = 53;
	s_saAttrib.skSTR = 0;
	s_saAttrib.skEND = 0;
	s_saAttrib.skINT = 0;
	s_saAttrib.skWIS = 0;
	s_saAttrib.skWIL = 0;
	s_saAttrib.skLCK = 54;
//	ADD_REQUIRED_SKILL( __SKILL_DISARM_TRAP, 56 );
}

void EvadeView::Destroy( void )
{
	s_saAttrib.tlskSkillRequired.AnnihilateList();
}

//////////////////////////////////////////////////////////////////////////////////////////
void EvadeView::OnAddPoints
//////////////////////////////////////////////////////////////////////////////////////////
// Function which will add EvadeView points
// 
(
 Unit *lpuTrained,			// Unit that trained.
 LPUSER_SKILL lpUserSkill,	// Skill's current points.
 DWORD dwNbPoints			// New skill points.
)
//////////////////////////////////////////////////////////////////////////////////////////
{
	int nEvasive = lpuTrained->ViewFlag( __FLAG_EVASIVNESS );
	nEvasive += dwNbPoints;
	lpuTrained->SetFlag( __FLAG_EVASIVNESS, nEvasive );
}

LPSKILLPNTFUNC EvadeView::lpOnAddPnts = OnAddPoints;

//////////////////////////////////////////////////////////////////////////////////////////
int EvadeView::Func
//////////////////////////////////////////////////////////////////////////////////////////
// EvadeView's function, does nothing
// -- Hook_None -- 
(
 DWORD dwReason,			// Hook which called this strike.
 Unit *self,				// Unused.
 Unit *medium,				// Unused.
 Unit *target,				// Unused.
 void *valueIN,				// Unused.
 void *valueOUT,			// Unused.
 LPUSER_SKILL lpusUserSkill // Skill strength.
)
// Return: int, SKILL_NO_FEEDBACK
//////////////////////////////////////////////////////////////////////////////////////////
{
	return SKILL_NO_FEEDBACK;
}