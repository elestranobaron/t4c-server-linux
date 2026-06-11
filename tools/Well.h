/******************************************************************************
Modify for vs2008 (24/04/2009)
******************************************************************************/


#ifndef __WELL_H
#define __WELL_H

class Well : public NPCstructure{
public:   
	Well();
	~Well();
	void Create( void );
	void OnAttacked( UNIT_FUNC_PROTOTYPE ); 
	void OnInitialise( UNIT_FUNC_PROTOTYPE );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif
