/******************************************************************************
Modify for vs2008 (23/04/2009)
******************************************************************************/
#ifndef __GMASSISTANT_H
#define __GMASSISTANT_H

class GMAssistant : public NPCstructure{
public:   
    GMAssistant();
    ~GMAssistant();
    void Create( void );
    void OnInitialise( UNIT_FUNC_PROTOTYPE );
    void OnDeath( UNIT_FUNC_PROTOTYPE );
    void OnAttacked( UNIT_FUNC_PROTOTYPE );
    void OnTalk( UNIT_FUNC_PROTOTYPE );   
};

#endif
