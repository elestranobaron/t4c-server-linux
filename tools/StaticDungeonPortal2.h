/******************************************************************************
Modify for vs2008 (23/04/2009)
******************************************************************************/
#ifndef __STATICDUNGEONPORTAL2_H
#define __STATICDUNGEONPORTAL2_H

class StaticDungeonPortal2 : public NPCstructure{
public:   
    StaticDungeonPortal2();
    ~StaticDungeonPortal2();
    void Create( void );
    void OnTalk( UNIT_FUNC_PROTOTYPE );
    void OnInitialise( UNIT_FUNC_PROTOTYPE);
    void OnAttacked( UNIT_FUNC_PROTOTYPE );
};

#endif
