/******************************************************************************
Modify for vs2008 (23/04/2009)
******************************************************************************/


#ifndef __NEXUSSTONE1_H
#define __NEXUSSTONE1_H

class NexusStone1 : public NPCstructure{
public:   
    NexusStone1();
   ~NexusStone1();
    void Create( void );
    void OnTalk( UNIT_FUNC_PROTOTYPE );
	 void OnAttacked( UNIT_FUNC_PROTOTYPE );
    void OnInitialise( UNIT_FUNC_PROTOTYPE );
};

#endif