/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA4MOBXP350_H
#define __ARENA4MOBXP350_H

class Arena4MobXP350 : public SimpleMonster{
public:
	Arena4MobXP350();
	~Arena4MobXP350();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif