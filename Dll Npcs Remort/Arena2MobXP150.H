/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA2MOBXP150_H
#define __ARENA2MOBXP150_H

class Arena2MobXP150 : public SimpleMonster{
public:
	Arena2MobXP150();
	~Arena2MobXP150();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif