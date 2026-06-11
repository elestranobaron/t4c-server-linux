/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA3MOBXP130_H
#define __ARENA3MOBXP130_H

class Arena3MobXP130 : public SimpleMonster{
public:
	Arena3MobXP130();
	~Arena3MobXP130();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif