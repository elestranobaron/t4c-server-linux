/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/


#ifndef __GEORGETHEIIIRD_H
#define __GEORGETHEIIIRD_H

class GeorgeTheIIIrd : public NPCstructure  
{
public:
	GeorgeTheIIIrd();
	virtual ~GeorgeTheIIIrd();

	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif // __GEORGETHEIIIRD_H
