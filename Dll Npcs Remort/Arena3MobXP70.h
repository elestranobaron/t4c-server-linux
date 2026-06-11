/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA3MOBXP70_H
#define __ARENA3MOBXP70_H

class Arena3MobXP70 : public SimpleMonster{
public:
	Arena3MobXP70();
	~Arena3MobXP70();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif