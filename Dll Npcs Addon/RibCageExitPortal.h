/******************************************************************************
Modify for vs2008 (23/04/2009)
******************************************************************************/


#ifndef __RIBCAGEEXITPORTAL_H
#define __RIBCAGEEXITPORTAL_H

class RibCageExitPortal : public NPCstructure{
public:   
  RibCageExitPortal();
  ~RibCageExitPortal();
	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
	void OnAttacked( UNIT_FUNC_PROTOTYPE );
	void OnInitialise( UNIT_FUNC_PROTOTYPE );
};

#endif
