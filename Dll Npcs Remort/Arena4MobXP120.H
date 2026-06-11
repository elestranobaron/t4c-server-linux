/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA4MOBXP120_H
#define __ARENA4MOBXP120_H

class Arena4MobXP120 : public SimpleMonster{
public:
	Arena4MobXP120();
	~Arena4MobXP120();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif