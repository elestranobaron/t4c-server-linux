/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA4MOBXP170_H
#define __ARENA4MOBXP170_H

class Arena4MobXP170 : public SimpleMonster{
public:
	Arena4MobXP170();
	~Arena4MobXP170();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif