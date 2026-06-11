/******************************************************************************
Modify for vs2008 (23/04/2009)
******************************************************************************/
#ifndef __MIRANDUL_H
#define __MIRANDUL_H

#include "NPCstructure.h"

class Mirandul : public NPCstructure  
{
public:
	Mirandul();
	virtual ~Mirandul();

	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif
