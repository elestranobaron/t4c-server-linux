/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA2MOBXP160_H
#define __ARENA2MOBXP160_H

class Arena2MobXP160 : public SimpleMonster{
public:
	Arena2MobXP160();
	~Arena2MobXP160();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif