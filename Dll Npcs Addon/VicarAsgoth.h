/******************************************************************************
Modify for vs2008 (23/04/2009)
******************************************************************************/


#ifndef __VICARASGOTH_H
#define __VICARASGOTH_H

class VicarAsgoth : public NPCstructure{
public:   
    VicarAsgoth();
   ~VicarAsgoth();
    void Create( void );
    void OnDeath( UNIT_FUNC_PROTOTYPE); 
	void OnAttack( UNIT_FUNC_PROTOTYPE); 
};

#endif
