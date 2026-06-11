/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA4MOBXP300_H
#define __ARENA4MOBXP300_H

class Arena4MobXP300 : public SimpleMonster{
public:
	Arena4MobXP300();
	~Arena4MobXP300();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif