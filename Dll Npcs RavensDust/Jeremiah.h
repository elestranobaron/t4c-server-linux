/******************************************************************************
Modify for vs2008 (24/04/2009)
******************************************************************************/


#ifndef __JEREMIAH_H
#define __JEREMIAH_H

class Jeremiah : public NPCstructure{
public:   
    Jeremiah();
   ~Jeremiah();
    void Create( void );
    void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif
