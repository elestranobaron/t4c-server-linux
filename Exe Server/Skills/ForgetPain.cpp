// ForgetPain.cpp: implementation of the ForgetPain class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "ForgetPain.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ForgetPain::ForgetPain()
{
	s_saAttrib.skLevel = 40;
	s_saAttrib.skAGI = 0;
	s_saAttrib.skSTR = 62;
	s_saAttrib.skEND = 70;
	s_saAttrib.skINT = 0;
	s_saAttrib.skWIS = 0;
	s_saAttrib.skWIL = 0;
	s_saAttrib.skLCK = 0;

//	ADD_REQUIRED_SKILL( __SKILL_TWO_WEAPONS, 80 );
}

ForgetPain::~ForgetPain()
{
	s_saAttrib.tlskSkillRequired.AnnihilateList();
}

//////////////////////////////////////////////////////////////////////////////////////////
void ForgetPain::OnAddPoints
//////////////////////////////////////////////////////////////////////////////////////////
// Function which will add parry points
// 
(
 Unit *lpuTrained,			// Unit that trained.
 LPUSER_SKILL lpUserSkill,	// Skill's current points.
 DWORD dwNbPoints			// New skill points.
)
//////////////////////////////////////////////////////////////////////////////////////////
{
	lpUserSkill->SetSkillPnts( lpUserSkill->GetTrueSkillPnts() + dwNbPoints );
	lpUserSkill->SetSkillPnts( lpUserSkill->GetTrueSkillPnts() - dwNbPoints );
}

LPSKILLPNTFUNC ForgetPain::lpOnAddPnts = OnAddPoints;

//////////////////////////////////////////////////////////////////////////////////////////
int ForgetPain::Func
//////////////////////////////////////////////////////////////////////////////////////////
// Parry's function, does nothing
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