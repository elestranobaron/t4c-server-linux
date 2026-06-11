/******************************************************************************
Modify for vs2008 (24/04/2009)
******************************************************************************/


#ifndef __BROTHER9_H
#define __BROTHER9_H

class Brother9 : public NPCstructure{
public:   
    Brother9();
    ~Brother9();
    void Create( void );
    void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif