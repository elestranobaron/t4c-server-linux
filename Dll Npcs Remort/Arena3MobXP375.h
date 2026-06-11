/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA3MOBXP375_H
#define __ARENA3MOBXP375_H

class Arena3MobXP375 : public SimpleMonster{
public:
	Arena3MobXP375();
	~Arena3MobXP375();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif