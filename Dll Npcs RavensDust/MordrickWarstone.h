/******************************************************************************
Modify for vs2008 (24/04/2009)
******************************************************************************/


#ifndef __MORDRICKWARSTONE_H
#define __MORDRICKWARSTONE_H

class MordrickWarstone : public NPCstructure{
public:   
    MordrickWarstone();
    ~MordrickWarstone();
    void Create( void );
    void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif
