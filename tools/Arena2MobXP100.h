/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA2MOBXP100_H
#define __ARENA2MOBXP100_H

class Arena2MobXP100 : public SimpleMonster{
public:
	Arena2MobXP100();
	~Arena2MobXP100();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif