/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENAMOBXP140_H
#define __ARENAMOBXP140_H

class ArenaMobXP140 : public SimpleMonster{
public:
	ArenaMobXP140();
	~ArenaMobXP140();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif