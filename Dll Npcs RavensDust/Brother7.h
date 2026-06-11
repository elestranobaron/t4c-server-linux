/******************************************************************************
Modify for vs2008 (24/04/2009)
******************************************************************************/


#ifndef __BROTHER7_H
#define __BROTHER7_H

class Brother7 : public NPCstructure{
public:   
    Brother7();
    ~Brother7();
    void Create( void );
    void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif