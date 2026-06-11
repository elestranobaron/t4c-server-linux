/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA3MOBXP50_H
#define __ARENA3MOBXP50_H

class Arena3MobXP50 : public SimpleMonster{
public:
	Arena3MobXP50();
	~Arena3MobXP50();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif