/******************************************************************************
Modify for vs2008 (24/04/2009)
******************************************************************************/


#ifndef __LAREN_H
#define __LAREN_H

class Laren : public NPCstructure{
public:   
    Laren();
   ~Laren();
    void Create( void );
    void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif
