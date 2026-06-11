/******************************************************************************
Modify for vs2008 (23/04/2009)
******************************************************************************/


#ifndef __DESKNOMAD1_H
#define __DESKNOMAD1_H

class DeskNomad1 : public NPCstructure{
public:   
   DeskNomad1();
   ~DeskNomad1();
	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
   void OnAttacked( UNIT_FUNC_PROTOTYPE );
   void OnInitialise( UNIT_FUNC_PROTOTYPE );

};

#endif