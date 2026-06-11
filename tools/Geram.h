/******************************************************************************
Modify for vs2008 (24/04/2009)
******************************************************************************/


#ifndef __GERAM_H
#define __GERAM_H

class Geram : public NPCstructure{
public:   
    Geram();
   ~Geram();
    void Create( void );
    void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif
