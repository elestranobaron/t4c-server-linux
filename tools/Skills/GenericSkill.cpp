// GenericSkill.cpp: implementation of the GenericSkill class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "GenericSkill.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

GenericSkill::GenericSkill()
{
// #/Name            Class     Cost Skills Fr  Level Stats Requirements Class Pts
// --------          -----     ---- ------ --  ----- ------------------ ---------
// 6/Pray:            Priests.   26  1- 500 WU    12  WIS: 40.           25P.
	
	s_saAttrib.skLevel = 0;
	s_saAttrib.skAGI = 0;
	s_saAttrib.skSTR = 0;
	s_saAttrib.skEND = 0;
	s_saAttrib.skINT = 0;
	s_saAttrib.skWIS = 0;
	s_saAttrib.skWIL = 0;
	s_saAttrib.skLCK = 0;
}

void GenericSkill::Destroy( void )
{
}


LPSKILLPNTFUNC GenericSkill::lpOnAddPnts = NULL;

//////////////////////////////////////////////////////////////////////////////////////////
int GenericSkill::Func
//////////////////////////////////////////////////////////////////////////////////////////
// This is the pray skill. Regenerates faith for priests.
// 
(
 DWORD dwReason,			// Hook which made the call to this function
 Unit *self,				// Unit using/regen the skill.
 Unit *medium,				// Unused.
 Unit *target,				// Unused.
 void *valueIN,				// Unused.
 void *valueOUT,			// Unused.
 LPUSER_SKILL lpusUserSkill // Strength of the skill
)
// Return: int, SKILL_PERSONNAL_FEEBACK_SUCCESSFULL or SKILL_NO_FEEDBACK
//////////////////////////////////////////////////////////////////////////////////////////
{
    return SKILL_NO_FEEDBACK;
}