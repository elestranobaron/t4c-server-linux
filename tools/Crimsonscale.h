/******************************************************************************
Modify for vs2008 (23/04/2009)
******************************************************************************/


#ifndef __CRIMSONSCALE_H
#define __CRIMSONSCALE_H

class Crimsonscale : public NPCstructure{
public:   
	Crimsonscale();
   ~Crimsonscale();

   void Create( void );
   void OnTalk( UNIT_FUNC_PROTOTYPE );
   void OnAttacked ( UNIT_FUNC_PROTOTYPE ); 
   void OnInitialise( UNIT_FUNC_PROTOTYPE );
};

#endif