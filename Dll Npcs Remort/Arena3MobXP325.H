/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA3MOBXP325_H
#define __ARENA3MOBXP325_H

class Arena3MobXP325 : public SimpleMonster{
public:
	Arena3MobXP325();
	~Arena3MobXP325();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif