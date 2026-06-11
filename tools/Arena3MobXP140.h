/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA3MOBXP140_H
#define __ARENA3MOBXP140_H

class Arena3MobXP140 : public SimpleMonster{
public:
	Arena3MobXP140();
	~Arena3MobXP140();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif