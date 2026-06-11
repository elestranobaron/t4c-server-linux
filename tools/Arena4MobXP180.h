/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA4MOBXP180_H
#define __ARENA4MOBXP180_H

class Arena4MobXP180 : public SimpleMonster{
public:
	Arena4MobXP180();
	~Arena4MobXP180();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif