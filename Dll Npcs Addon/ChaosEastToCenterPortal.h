/******************************************************************************
Modify for vs2008 (23/04/2009)
******************************************************************************/


#ifndef __CHAOSEASTTOCENTERPORTAL_H
#define __CHAOSEASTTOCENTERPORTAL_H
              
class ChaosEastToCenterPortal : public NPCstructure{
public:   
  ChaosEastToCenterPortal();
  ~ChaosEastToCenterPortal();
	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
	void OnAttacked( UNIT_FUNC_PROTOTYPE );
	void OnInitialise( UNIT_FUNC_PROTOTYPE );
};

#endif
