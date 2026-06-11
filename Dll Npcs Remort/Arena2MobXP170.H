/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA2MOBXP170_H
#define __ARENA2MOBXP170_H

class Arena2MobXP170 : public SimpleMonster{
public:
	Arena2MobXP170();
	~Arena2MobXP170();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif