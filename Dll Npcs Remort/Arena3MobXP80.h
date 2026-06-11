/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA3MOBXP80_H
#define __ARENA3MOBXP80_H

class Arena3MobXP80 : public SimpleMonster{
public:
	Arena3MobXP80();
	~Arena3MobXP80();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif