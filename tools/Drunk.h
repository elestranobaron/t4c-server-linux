/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/


#ifndef __DRUNK_H
#define __DRUNK_H

class Drunk : public NPCstructure  
{
public:
	Drunk();
	virtual ~Drunk();

	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif // __DRUNK_H
