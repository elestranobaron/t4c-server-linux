/******************************************************************************
Modify for vs2008 (23/04/2009)
******************************************************************************/

#include "SimpleMonster.h"

#ifndef __OLINHAADCOMMANDER_H
#define __OLINHAADCOMMANDER_H

class OlinHaadCommander : public SimpleMonster{
public:
	OlinHaadCommander();
	~OlinHaadCommander();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
};

#endif