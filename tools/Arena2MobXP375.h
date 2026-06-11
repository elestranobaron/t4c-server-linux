/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA2MOBXP375_H
#define __ARENA2MOBXP375_H

class Arena2MobXP375 : public SimpleMonster{
public:
	Arena2MobXP375();
	~Arena2MobXP375();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif