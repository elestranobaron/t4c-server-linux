/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA2MOBXP350_H
#define __ARENA2MOBXP350_H

class Arena2MobXP350 : public SimpleMonster{
public:
	Arena2MobXP350();
	~Arena2MobXP350();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif