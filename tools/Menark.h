/******************************************************************************
Modify for vs2008 (24/04/2009)
******************************************************************************/


#ifndef __MENARK_H
#define __MENARK_H

class Menark : public NPCstructure{
public:   
    Menark();
    ~Menark();
    void Create( void );
    void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif
