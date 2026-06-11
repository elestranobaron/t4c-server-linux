/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA3MOBXP350_H
#define __ARENA3MOBXP350_H

class Arena3MobXP350 : public SimpleMonster{
public:
	Arena3MobXP350();
	~Arena3MobXP350();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif