/******************************************************************************
Modify for vs2008 (23/04/2009)
******************************************************************************/
#include "NPCstructure.h"

#ifndef __TOTAK_H
#define __TOTAK_H

class Totak : public NPCstructure  
{
public:
	Totak();
	virtual ~Totak();

	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif
