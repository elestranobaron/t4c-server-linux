/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA4MOBXP100_H
#define __ARENA4MOBXP100_H

class Arena4MobXP100 : public SimpleMonster{
public:
	Arena4MobXP100();
	~Arena4MobXP100();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif