/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA3MOBXP160_H
#define __ARENA3MOBXP160_H

class Arena3MobXP160 : public SimpleMonster{
public:
	Arena3MobXP160();
	~Arena3MobXP160();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif