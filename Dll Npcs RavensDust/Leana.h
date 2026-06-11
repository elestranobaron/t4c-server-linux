/******************************************************************************
Modify for vs2008 (24/04/2009)
******************************************************************************/


#ifndef __LEANA_H
#define __LEANA_H

class Leana : public NPCstructure{
public:   
    Leana();
    ~Leana();
    void Create( void );
    void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif
