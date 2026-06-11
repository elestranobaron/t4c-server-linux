// Rundumschlag.h - New Skill for D4O.info.

#ifndef RUNDUMSCHLAGSKILL_H
#define RUNDUMSCHLAGSKILL_H

#include "Skills.h"

class Rundumschlag  
{
public:
	Rundumschlag();
	void Destroy( void );
	
	static LPSKILLPNTFUNC lpOnAddPnts;
   static void RundumschlagCallback( EFFECT_FUNC_PROTOTYPE );
   static void ExhaustRemovallCallback( EFFECT_FUNC_PROTOTYPE );
	static int Func(unsigned long dwReason, Unit *self, Unit *medium, Unit *target, 
				void *valueIN, void *valueOUT, LPUSER_SKILL lpusUserSkill);

	SKILL_ATTRIBUTES s_saAttrib;
};

#endif // RUNDUMSCHLAG_H
