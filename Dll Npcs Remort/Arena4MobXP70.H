/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA4MOBXP70_H
#define __ARENA4MOBXP70_H

class Arena4MobXP70 : public SimpleMonster{
public:
	Arena4MobXP70();
	~Arena4MobXP70();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif