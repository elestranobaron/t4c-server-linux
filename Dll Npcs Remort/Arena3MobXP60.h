/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA3MOBXP60_H
#define __ARENA3MOBXP60_H

class Arena3MobXP60 : public SimpleMonster{
public:
	Arena3MobXP60();
	~Arena3MobXP60();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif