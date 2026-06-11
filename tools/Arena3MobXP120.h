/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA3MOBXP120_H
#define __ARENA3MOBXP120_H

class Arena3MobXP120 : public SimpleMonster{
public:
	Arena3MobXP120();
	~Arena3MobXP120();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif