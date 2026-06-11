/******************************************************************************
Modify for vs2008 (24/04/2009)
******************************************************************************/


#ifndef __ORCARCHMAGE_H
#define __ORCARCHMAGE_H

class Orcarchmage : public NPCstructure{
public:   
    Orcarchmage();
    ~Orcarchmage();
    void Create( void );
    void OnTalk( UNIT_FUNC_PROTOTYPE );
    void OnDeath( UNIT_FUNC_PROTOTYPE );
};

#endif
