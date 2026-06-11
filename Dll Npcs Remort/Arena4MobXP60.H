/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA4MOBXP60_H
#define __ARENA4MOBXP60_H

class Arena4MobXP60 : public SimpleMonster{
public:
	Arena4MobXP60();
	~Arena4MobXP60();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif