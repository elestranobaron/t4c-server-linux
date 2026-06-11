/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA2MOBXP130_H
#define __ARENA2MOBXP130_H

class Arena2MobXP130 : public SimpleMonster{
public:
	Arena2MobXP130();
	~Arena2MobXP130();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif