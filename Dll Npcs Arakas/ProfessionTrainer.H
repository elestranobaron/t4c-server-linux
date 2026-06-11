// ProfessionTrainer.h: interface for the ProfessionTrainer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ProfessionTrainer_H__53CB2377_AA4A_11D1_AD5F_00E029058623__INCLUDED_)
#define AFX_ProfessionTrainer_H__53CB2377_AA4A_11D1_AD5F_00E029058623__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "NPCstructure.h"

class ProfessionTrainer : public NPCstructure  
{
public:
        ProfessionTrainer();
        virtual ~ProfessionTrainer();

	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
   void OnNPCDataExchange( UNIT_FUNC_PROTOTYPE );
   void OnInitialise( UNIT_FUNC_PROTOTYPE );
};

#endif // !defined(AFX_ProfessionTrainer_H__53CB2377_AA4A_11D1_AD5F_00E029058623__INCLUDED_)
