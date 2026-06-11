/******************************************************************************
Modify for vs2008 (24/04/2009)
******************************************************************************/


#ifndef __REYNEN_ASPICDART_H
#define __REYNEN_ASPICDART_H

class Reynen_Aspicdart : public NPCstructure{
public:   
    Reynen_Aspicdart();
   ~Reynen_Aspicdart();
    void Create( void );
    void OnTalk( UNIT_FUNC_PROTOTYPE );
	//void OnDeath( UNIT_FUNC_PROTOTYPE );
};

#endif
