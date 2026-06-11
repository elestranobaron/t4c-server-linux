/******************************************************************************
Modify for vs2008 (24/04/2009)
******************************************************************************/


#ifndef __JAMAR_H
#define __JAMAR_H

class Jamar : public NPCstructure{
public:   
    Jamar();
   ~Jamar();
    void Create( void );
    void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif
