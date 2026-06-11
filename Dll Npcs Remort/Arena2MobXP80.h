/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA2MOBXP80_H
#define __ARENA2MOBXP80_H

class Arena2MobXP80 : public SimpleMonster{
public:
	Arena2MobXP80();
	~Arena2MobXP80();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif