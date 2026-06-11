/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA2MOBXP325_H
#define __ARENA2MOBXP325_H

class Arena2MobXP325 : public SimpleMonster{
public:
	Arena2MobXP325();
	~Arena2MobXP325();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif