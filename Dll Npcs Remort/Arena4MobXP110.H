/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA4MOBXP110_H
#define __ARENA4MOBXP110_H

class Arena4MobXP110 : public SimpleMonster{
public:
	Arena4MobXP110();
	~Arena4MobXP110();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif