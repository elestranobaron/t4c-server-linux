/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/


#ifndef __DOKTORSPINE_H
#define __DOKTORSPINE_H

class DoktorSpine : public NPCstructure{
public:   
    DoktorSpine();
   ~DoktorSpine();
    void Create( void );   
    void OnAttack( UNIT_FUNC_PROTOTYPE );

};

#endif
