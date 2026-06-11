/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA4MOBXP225_H
#define __ARENA4MOBXP225_H

class Arena4MobXP225 : public SimpleMonster{
public:
	Arena4MobXP225();
	~Arena4MobXP225();
	void Create( void );
	void OnPopup( UNIT_FUNC_PROTOTYPE );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif