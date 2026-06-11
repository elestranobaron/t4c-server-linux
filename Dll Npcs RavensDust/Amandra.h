/******************************************************************************
Modify for vs2008 (24/04/2009)
******************************************************************************/


#ifndef __AMANDRA_H
#define __AMANDRA_H

class Amandra : public NPCstructure{
public:   
	Amandra();
	~Amandra();
	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif
