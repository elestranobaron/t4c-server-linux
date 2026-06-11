/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA4MOBXP80_H
#define __ARENA4MOBXP80_H

class Arena4MobXP80 : public SimpleMonster{
public:
	Arena4MobXP80();
	~Arena4MobXP80();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif