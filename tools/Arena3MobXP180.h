/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA3MOBXP180_H
#define __ARENA3MOBXP180_H

class Arena3MobXP180 : public SimpleMonster{
public:
	Arena3MobXP180();
	~Arena3MobXP180();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif