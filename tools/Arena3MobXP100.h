/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA3MOBXP100_H
#define __ARENA3MOBXP100_H

class Arena3MobXP100 : public SimpleMonster{
public:
	Arena3MobXP100();
	~Arena3MobXP100();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif