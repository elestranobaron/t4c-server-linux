/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA4MOBXP150_H
#define __ARENA4MOBXP150_H

class Arena4MobXP150 : public SimpleMonster{
public:
	Arena4MobXP150();
	~Arena4MobXP150();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif