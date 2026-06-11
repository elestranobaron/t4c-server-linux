/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA3MOBXP40_H
#define __ARENA3MOBXP40_H

class Arena3MobXP40 : public SimpleMonster{
public:
	Arena3MobXP40();
	~Arena3MobXP40();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif