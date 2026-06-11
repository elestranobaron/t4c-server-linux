/******************************************************************************
Modify for vs2008 (24/04/2009)
******************************************************************************/


#ifndef __IDAS_H
#define __IDAS_H

class Idas : public NPCstructure{
public:   
    Idas();
   ~Idas();
    void Create( void );
    void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif
