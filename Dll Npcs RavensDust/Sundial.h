/******************************************************************************
Modify for vs2008 (24/04/2009)
******************************************************************************/


#ifndef __SUNDIAL_H
#define __SUNDIAL_H

class Sundial : public NPCstructure{
public:   
    Sundial();
    ~Sundial();
    void Create( void );
    void OnTalk( UNIT_FUNC_PROTOTYPE );
	void OnAttacked( UNIT_FUNC_PROTOTYPE );
    void OnInitialise( UNIT_FUNC_PROTOTYPE );
};

#endif