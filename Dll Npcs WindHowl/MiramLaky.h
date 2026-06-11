/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/


#ifndef __MIRAMLAKY_H
#define __MIRAMLAKY_H

class MiramLaky : public NPCstructure  
{
public:
	MiramLaky();
	virtual ~MiramLaky();

	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif // __MIRAMLAKY_H
