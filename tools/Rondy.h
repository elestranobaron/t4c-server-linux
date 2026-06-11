/******************************************************************************
Modify for vs2008 (24/04/2009)
******************************************************************************/


#ifndef __RONDY_H
#define __RONDY_H

class Rondy : public NPCstructure{
public:   
    Rondy();
   ~Rondy();
    void Create( void );
    void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif
