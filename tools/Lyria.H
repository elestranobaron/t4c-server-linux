/******************************************************************************
Modify for vs2008 (23/04/2009)
******************************************************************************/
#include "NPCStructure.h"

#ifndef __LYRIA_H
#define __LYRIA_H

#include "NPCstructure.h"

class Lyria : public NPCstructure  
{
public:
	Lyria();
	virtual ~Lyria();

	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
	void OnNPCDataExchange( UNIT_FUNC_PROTOTYPE );
};

#endif 
