/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA4MOBXP90_H
#define __ARENA4MOBXP90_H

class Arena4MobXP90 : public SimpleMonster{
public:
	Arena4MobXP90();
	~Arena4MobXP90();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif