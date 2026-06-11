/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA3MOBXP170_H
#define __ARENA3MOBXP170_H

class Arena3MobXP170 : public SimpleMonster{
public:
	Arena3MobXP170();
	~Arena3MobXP170();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif