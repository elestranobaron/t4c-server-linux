/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA2MOBXP190_H
#define __ARENA2MOBXP190_H

class Arena2MobXP190 : public SimpleMonster{
public:
	Arena2MobXP190();
	~Arena2MobXP190();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif