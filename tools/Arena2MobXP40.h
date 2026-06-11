/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA2MOBXP40_H
#define __ARENA2MOBXP40_H

class Arena2MobXP40 : public SimpleMonster{
public:
	Arena2MobXP40();
	~Arena2MobXP40();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif