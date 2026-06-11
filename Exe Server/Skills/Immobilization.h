// Resurect.h: interface for the Resurect class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IMMOBILIZATION_H__CB86BB9B_74D6_11D2_8447_00E02922FA41__INCLUDED_)
#define AFX_IMMOBILIZATION_H__CB86BB9B_74D6_11D2_8447_00E02922FA41__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "Skills.h"

#define SKILL_IMMOB_CYCLE_TIME 2000 
#define SKILL_IMMOB_TARGET_EXHAUST 1745

class Immobilization  
{
public:
   Immobilization();
   void Destroy( void );

   static LPSKILLPNTFUNC lpOnAddPnts;
   
   static void ImmobilizationCallback( EFFECT_FUNC_PROTOTYPE );
   static void ImmobilizationBoustRemovalCallback( EFFECT_FUNC_PROTOTYPE );
   static void ExhaustRemovallCallback( EFFECT_FUNC_PROTOTYPE );

   static int Func(DWORD dwReason, Unit *self, Unit *medium, Unit *target, 
      void *valueIN, void *valueOUT, LPUSER_SKILL lpusUserSkill);
   
   SKILL_ATTRIBUTES s_saAttrib;
   
};

#endif // !defined(AFX_RESURECT_H__CB86BB9B_74D6_11D2_8447_00E02922FA40__INCLUDED_)
