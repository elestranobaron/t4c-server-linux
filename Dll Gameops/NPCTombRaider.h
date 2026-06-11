/******************************************************************************
Modify for vs2008 (23/04/2009)
******************************************************************************/
#ifndef __NPCTOMBRAIDER_H
#define __NPCTOMBRAIDER_H

class NPCTombRaider : public NPCstructure{
public:   
    NPCTombRaider();
   ~NPCTombRaider();
    void Create( void );
    void OnDeath( UNIT_FUNC_PROTOTYPE );
    void OnAttack( UNIT_FUNC_PROTOTYPE );
	 void OnAttacked( UNIT_FUNC_PROTOTYPE );
};

#endif
