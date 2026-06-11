/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA2MOBXP110_H
#define __ARENA2MOBXP110_H

class Arena2MobXP110 : public SimpleMonster{
public:
	Arena2MobXP110();
	~Arena2MobXP110();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif