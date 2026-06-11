/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/


#ifndef __LORDTRAVIS_H
#define __LORDTRAVIS_H

class LordTravis : public NPCstructure  
{
public:
	LordTravis();
	virtual ~LordTravis();

	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif // __LORDTRAVIS_H
