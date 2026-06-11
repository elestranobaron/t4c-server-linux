/******************************************************************************
Modify for vs2008 (23/04/2009)
******************************************************************************/


#ifndef __NEXUSSTONE5_H
#define __NEXUSSTONE5_H

class NexusStone5 : public NPCstructure{
public:   
    NexusStone5();
   ~NexusStone5();
    void Create( void );
    void OnTalk( UNIT_FUNC_PROTOTYPE );
	 void OnAttacked( UNIT_FUNC_PROTOTYPE );
    void OnInitialise( UNIT_FUNC_PROTOTYPE );
};

#endif