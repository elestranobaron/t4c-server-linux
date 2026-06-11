/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/


#ifndef __ASSYSTANTER_H
#define __ASSYSTANTER_H

class AssystAnter : public NPCstructure  
{
public:
	AssystAnter();
	virtual ~AssystAnter();

	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif // __ASSYSTANTER_H
