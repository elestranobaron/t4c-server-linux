/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENA2MOBXP200_H
#define __ARENA2MOBXP200_H

class Arena2MobXP200 : public SimpleMonster{
public:
	Arena2MobXP200();
	~Arena2MobXP200();
	void Create( void );
	void OnPopup( UNIT_FUNC_PROTOTYPE );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif