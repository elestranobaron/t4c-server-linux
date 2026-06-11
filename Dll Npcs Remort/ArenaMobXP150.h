/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "SimpleMonster.h"

#ifndef __ARENAMOBXP150_H
#define __ARENAMOBXP150_H

class ArenaMobXP150 : public SimpleMonster{
public:
	ArenaMobXP150();
	~ArenaMobXP150();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );
};

#endif