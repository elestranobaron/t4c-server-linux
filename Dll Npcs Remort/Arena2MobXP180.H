/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA2MOBXP180_H
#define __ARENA2MOBXP180_H

class Arena2MobXP180 : public SimpleMonster{
public:
	Arena2MobXP180();
	~Arena2MobXP180();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif