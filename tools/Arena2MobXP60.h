/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA2MOBXP60_H
#define __ARENA2MOBXP60_H

class Arena2MobXP60 : public SimpleMonster{
public:
	Arena2MobXP60();
	~Arena2MobXP60();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif