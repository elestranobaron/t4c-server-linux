/******************************************************************************
Modify for vs2008 (23/04/2009)
******************************************************************************/

#ifndef __DORKENROTSMELL_H
#define __DORKENROTSMELL_H

class DorkenRotsmell : public NPCstructure{
public:   
    DorkenRotsmell();
   ~DorkenRotsmell();
    void Create( void );
    void OnDeath( UNIT_FUNC_PROTOTYPE );
    void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif
