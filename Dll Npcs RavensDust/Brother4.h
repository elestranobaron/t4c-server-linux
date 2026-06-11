/******************************************************************************
Modify for vs2008 (24/04/2009)
******************************************************************************/


#ifndef __BROTHER4_H
#define __BROTHER4_H

class Brother4 : public NPCstructure{
public:   
    Brother4();
    ~Brother4();
    void Create( void );
    void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif