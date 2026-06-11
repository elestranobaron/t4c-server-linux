/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA2MOBXP475_H
#define __ARENA2MOBXP475_H

class Arena2MobXP475 : public SimpleMonster{
public:
	Arena2MobXP475();
	~Arena2MobXP475();
	void Create( void );
	void OnPopup( UNIT_FUNC_PROTOTYPE );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif