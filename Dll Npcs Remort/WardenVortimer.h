/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/


#ifndef __WARDENVORTIMER_H
#define __WARDENVORTIMER_H

class WardenVortimer : public NPCstructure{
public:   
    WardenVortimer();
   ~WardenVortimer();
    void Create( void );
    void OnTalk( UNIT_FUNC_PROTOTYPE );
    
};

#endif
