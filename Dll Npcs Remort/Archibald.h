/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/


#ifndef __ARCHIBALD_H
#define __ARCHIBALD_H

class Archibald : public NPCstructure{
public:   
    Archibald();
   ~Archibald();
    void Create( void );
    void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif
