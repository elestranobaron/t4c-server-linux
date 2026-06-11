/******************************************************************************
Modify for vs2008 (23/04/2009)
******************************************************************************/


#ifndef __RAMIRGO_H
#define __RAMIRGO_H

class Ramirgo : public NPCstructure{
public:
	Ramirgo();
	~Ramirgo();
	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
	void OnAttacked ( UNIT_FUNC_PROTOTYPE ); 
	void OnInitialise( UNIT_FUNC_PROTOTYPE );

};

#endif