/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENAMOB130_H
#define __ARENAMOB130_H

class ArenaMob130 : public SimpleMonster{
public:
	ArenaMob130();
	~ArenaMob130();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif