// PrimalScream.h: interface for the Resurect class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PRIMALSCREAM_H__CB86BB9B_74D6_11D2_8447_00E02922FA41__INCLUDED_)
#define AFX_PRIMALSCREAM_H__CB86BB9B_74D6_11D2_8447_00E02922FA41__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "Skills.h"

class PrimalScream  
{
public:
   PrimalScream();
   void Destroy( void );

   static LPSKILLPNTFUNC lpOnAddPnts;

   static void PrimalScreamCallback( EFFECT_FUNC_PROTOTYPE );
   static void ExhaustRemovallCallback( EFFECT_FUNC_PROTOTYPE );

   static int Func(DWORD dwReason, Unit *self, Unit *medium, Unit *target, 
      void *valueIN, void *valueOUT, LPUSER_SKILL lpusUserSkill);
   
   SKILL_ATTRIBUTES s_saAttrib;
   
};

#endif // !defined(AFX_RESURECT_H__CB86BB9B_74D6_11D2_8447_00E02922FA40__INCLUDED_)
