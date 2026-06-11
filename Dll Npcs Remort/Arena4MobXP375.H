/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA4MOBXP375_H
#define __ARENA4MOBXP375_H

class Arena4MobXP375 : public SimpleMonster{
public:
	Arena4MobXP375();
	~Arena4MobXP375();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif