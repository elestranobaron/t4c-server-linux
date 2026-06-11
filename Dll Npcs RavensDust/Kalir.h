/******************************************************************************
Modify for vs2008 (24/04/2009)
******************************************************************************/


#ifndef __KALIR_H
#define __KALIR_H

class Kalir : public NPCstructure{
public:   
    Kalir();
   ~Kalir();
    void Create( void );
    void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif
