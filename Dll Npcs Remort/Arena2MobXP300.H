/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA2MOBXP300_H
#define __ARENA2MOBXP300_H

class Arena2MobXP300 : public SimpleMonster{
public:
	Arena2MobXP300();
	~Arena2MobXP300();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif