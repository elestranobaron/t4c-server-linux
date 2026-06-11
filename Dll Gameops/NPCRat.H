/******************************************************************************
Modify for vs2008 (23/04/2009)
******************************************************************************/
#ifndef __NPCRAT_H
#define __NPCRAT_H

class NPCRat : public NPCstructure{
public:   
    NPCRat();
   ~NPCRat();
    void Create( void );
    void OnDeath( UNIT_FUNC_PROTOTYPE );
};

#endif
