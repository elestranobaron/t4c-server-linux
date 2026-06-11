/******************************************************************************
Modify for vs2008 (24/04/2009)
******************************************************************************/


#ifndef __BROTHER5_H
#define __BROTHER5_H

class Brother5 : public NPCstructure{
public:   
    Brother5();
    ~Brother5();
    void Create( void );
    void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif