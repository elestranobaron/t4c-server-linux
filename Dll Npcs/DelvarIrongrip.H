/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#ifndef __DELVARIRONGRIP_H
#define __DELVARIRONGRIP_H

class DelvarIrongrip : public NPCstructure{
public:   
    DelvarIrongrip();
   ~DelvarIrongrip();
    void Create( void );
    void OnTalk( UNIT_FUNC_PROTOTYPE );
    //void OnInitialise( UNIT_FUNC_PROTOTYPE );
    void OnAttacked( UNIT_FUNC_PROTOTYPE );
    void OnAttack( UNIT_FUNC_PROTOTYPE );
};

#endif
