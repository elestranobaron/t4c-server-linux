/******************************************************************************
Modify for vs2008 (23/04/2009)
******************************************************************************/


#ifndef __DEFILEMENTPORTAL2_H
#define __DEFILEMENTPORTAL2_H

class DefilementPortal2 : public NPCstructure{
public:   
  DefilementPortal2();
  ~DefilementPortal2();
	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
	void OnAttacked( UNIT_FUNC_PROTOTYPE );
	void OnInitialise( UNIT_FUNC_PROTOTYPE );
};

#endif
