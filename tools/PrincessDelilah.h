/******************************************************************************
Modify for vs2008 (24/04/2009)
******************************************************************************/


#ifndef __PRINCESSDELILAH_H
#define __PRINCESSDELILAH_H

class PrincessDelilah : public NPCstructure  
{
public:
	PrincessDelilah();
	virtual ~PrincessDelilah();
	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif

