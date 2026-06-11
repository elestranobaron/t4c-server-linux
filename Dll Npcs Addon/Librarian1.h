/******************************************************************************
Modify for vs2008 (23/04/2009)
******************************************************************************/


#ifndef __LIBRARIAN1_H
#define __LIBRARIAN1_H

class Librarian1 : public NPCstructure{
public:
	Librarian1();
	~Librarian1();
	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
	void OnAttacked ( UNIT_FUNC_PROTOTYPE ); 
	void OnInitialise( UNIT_FUNC_PROTOTYPE );

};

#endif