/******************************************************************************
Modify for vs2008 (23/04/2009)
******************************************************************************/


#ifndef __VICARVHARMES_H
#define __VICARVHARMES_H

class VicarVharmes : public NPCstructure{
public:   
    VicarVharmes();
   ~VicarVharmes();
    void Create( void );
    void OnDeath( UNIT_FUNC_PROTOTYPE);
	void OnAttack( UNIT_FUNC_PROTOTYPE); 
};

#endif
