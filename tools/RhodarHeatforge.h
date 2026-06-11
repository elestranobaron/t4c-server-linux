/******************************************************************************
Modify for vs2008 (24/04/2009)
******************************************************************************/


#ifndef __RHODARHEATFORGE_H
#define __RHODARHEATFORGE_H

class RhodarHeatforge : public NPCstructure{
public:   
    RhodarHeatforge();
    ~RhodarHeatforge();
    void Create( void );
    void OnTalk( UNIT_FUNC_PROTOTYPE );
    void OnNPCDataExchange( UNIT_FUNC_PROTOTYPE );
};

#endif
