/******************************************************************************
Modify for vs2008 (23/04/2009)
******************************************************************************/


#ifndef __OLINTOSTRONGHOLDPORTAL_H
#define __OLINTOSTRONGHOLDPORTAL_H

class OlinToStrongholdPortal : public NPCstructure{
public:   
  OlinToStrongholdPortal();
  ~OlinToStrongholdPortal();
	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
	void OnAttacked( UNIT_FUNC_PROTOTYPE );
	void OnInitialise( UNIT_FUNC_PROTOTYPE );
};

#endif
