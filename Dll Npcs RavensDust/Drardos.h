/******************************************************************************
Modify for vs2008 (24/04/2009)
******************************************************************************/


#ifndef __DRARDOS_H
#define __DRARDOS_H

class Drardos : public NPCstructure{
public:   
    Drardos();
   ~Drardos();
    void Create( void );
    void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif
