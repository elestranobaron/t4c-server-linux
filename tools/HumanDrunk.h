/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/


#ifndef __HUMANDRUNK_H
#define __HUMANDRUNK_H

class HumanDrunk : public NPCstructure  
{
public:
	HumanDrunk();
	virtual ~HumanDrunk();

	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif // __HUMANDRUNK_H
