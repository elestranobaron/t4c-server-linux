/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA2MOBXP70_H
#define __ARENA2MOBXP70_H

class Arena2MobXP70 : public SimpleMonster{
public:
	Arena2MobXP70();
	~Arena2MobXP70();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif