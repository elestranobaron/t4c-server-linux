/******************************************************************************
Modify for vs2008 (23/04/2009)
******************************************************************************/


#ifndef __ANTONIAN_H
#define __ANTONIAN_H

class Antonian : public NPCstructure{
public:
	Antonian();
	~Antonian();
	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );

};

#endif