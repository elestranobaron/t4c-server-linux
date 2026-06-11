/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA3MOBXP150_H
#define __ARENA3MOBXP150_H

class Arena3MobXP150 : public SimpleMonster{
public:
	Arena3MobXP150();
	~Arena3MobXP150();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif