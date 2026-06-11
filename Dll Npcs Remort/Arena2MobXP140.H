/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA2MOBXP140_H
#define __ARENA2MOBXP140_H

class Arena2MobXP140 : public SimpleMonster{
public:
	Arena2MobXP140();
	~Arena2MobXP140();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif