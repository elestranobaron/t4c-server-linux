/******************************************************************************
Modify for vs2008 (24/04/2009)
******************************************************************************/


#ifndef __TOBIAS_H
#define __TOBIAS_H

class Tobias : public NPCstructure{
public:   
    Tobias();
    ~Tobias();
    void Create( void );
    void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif
