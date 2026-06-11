/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA3MOBXP300_H
#define __ARENA3MOBXP300_H

class Arena3MobXP300 : public SimpleMonster{
public:
	Arena3MobXP300();
	~Arena3MobXP300();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif