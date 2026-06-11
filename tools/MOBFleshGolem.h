/******************************************************************************
Modify for vs2008 (24/04/2009)
******************************************************************************/


#ifndef __MOBFLESHGOLEM_H
#define __MOBFLESHGOLEM_H

class MOBFleshGolem : public NPCstructure{
public:   
    MOBFleshGolem();
    ~MOBFleshGolem();
    void Create( void );
    void OnPopup( UNIT_FUNC_PROTOTYPE );
    void OnAttacked( UNIT_FUNC_PROTOTYPE );
};

#endif
 
