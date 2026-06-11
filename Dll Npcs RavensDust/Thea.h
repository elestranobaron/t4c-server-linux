/******************************************************************************
Modify for vs2008 (24/04/2009)
******************************************************************************/


#ifndef __THEA_H
#define __THEA_H

class Thea : public NPCstructure{
public:   
    Thea();
    ~Thea();
    void Create( void );
    void OnTalk( UNIT_FUNC_PROTOTYPE );    
};

#endif
