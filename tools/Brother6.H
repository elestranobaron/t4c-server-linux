/******************************************************************************
Modify for vs2008 (24/04/2009)
******************************************************************************/


#ifndef __BROTHER6_H
#define __BROTHER6_H

class Brother6 : public NPCstructure{
public:   
    Brother6();
    ~Brother6();
    void Create( void );
    void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif