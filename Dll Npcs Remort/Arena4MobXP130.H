/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA4MOBXP130_H
#define __ARENA4MOBXP130_H

class Arena4MobXP130 : public SimpleMonster{
public:
	Arena4MobXP130();
	~Arena4MobXP130();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif