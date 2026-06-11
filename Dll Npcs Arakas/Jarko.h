/******************************************************************************
Modify for vs2008 (23/04/2009)
******************************************************************************/

#ifndef __JARKO_H
#define __JARKO_H

class Jarko : public NPCstructure{
public:   
    Jarko();
   ~Jarko();
   void Create( void );
   void OnAttacked( UNIT_FUNC_PROTOTYPE );
   void OnAttack( UNIT_FUNC_PROTOTYPE );
   void OnDeath( UNIT_FUNC_PROTOTYPE );
   void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif
