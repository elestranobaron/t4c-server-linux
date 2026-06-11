/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA4MOBXP50_H
#define __ARENA4MOBXP50_H

class Arena4MobXP50 : public SimpleMonster{
public:
	Arena4MobXP50();
	~Arena4MobXP50();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif