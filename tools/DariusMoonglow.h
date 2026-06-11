/******************************************************************************
Modify for vs2008 (24/04/2009)
******************************************************************************/


#ifndef __DARIUSMOONGLOW_H
#define __DARIUSMOONGLOW_H

class DariusMoonglow : public NPCstructure{
public:   
    DariusMoonglow();
    ~DariusMoonglow();
    void Create( void );
    void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif
