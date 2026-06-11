/******************************************************************************
Modify for vs2008 (24/04/2009)
******************************************************************************/


#ifndef __BROTHER8_H
#define __BROTHER8_H

class Brother8 : public NPCstructure{
public:   
    Brother8();
    ~Brother8();
    void Create( void );
    void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif