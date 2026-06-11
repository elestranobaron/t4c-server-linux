/******************************************************************************
Modify for vs2008 (23/04/2009)
******************************************************************************/


#ifndef __SCHOLAR2_H
#define __SCHOLAR2_H

class Scholar2 : public NPCstructure{
public:   
	Scholar2();
   ~Scholar2();

   void Create( void );
   void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif