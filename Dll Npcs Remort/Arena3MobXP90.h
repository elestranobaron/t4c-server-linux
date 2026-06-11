/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA3MOBXP90_H
#define __ARENA3MOBXP90_H

class Arena3MobXP90 : public SimpleMonster{
public:
	Arena3MobXP90();
	~Arena3MobXP90();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif