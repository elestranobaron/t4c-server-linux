/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA2MOBXP50_H
#define __ARENA2MOBXP50_H

class Arena2MobXP50 : public SimpleMonster{
public:
	Arena2MobXP50();
	~Arena2MobXP50();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif