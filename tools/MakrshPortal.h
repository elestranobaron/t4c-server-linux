/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/


#ifndef __MAKRSHPORTAL_H
#define __MAKRSHPORTAL_H

class MakrshPortal : public NPCstructure{
public:   
   MakrshPortal();
   ~MakrshPortal();
	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
   void OnAttacked( UNIT_FUNC_PROTOTYPE );
   void OnInitialise( UNIT_FUNC_PROTOTYPE );

};

#endif
