/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA3MOBXP250_H
#define __ARENA3MOBXP250_H

class Arena3MobXP250 : public SimpleMonster{
public:
	Arena3MobXP250();
	~Arena3MobXP250();
	void Create( void );
	void OnPopup( UNIT_FUNC_PROTOTYPE );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif