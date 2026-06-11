/******************************************************************************
Modify for vs2008 (24/04/2009)
******************************************************************************/


#ifndef __EDDEN_H
#define __EDDEN_H

class Edden : public NPCstructure{
public:   
    Edden();
   ~Edden();
    void Create( void );
    void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif
