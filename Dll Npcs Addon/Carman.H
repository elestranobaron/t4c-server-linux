/******************************************************************************
Modify for vs2008 (23/04/2009)
******************************************************************************/


#ifndef __CARMAN_H
#define __CARMAN_H

class Carman : public NPCstructure{
public:
	Carman();
	~Carman();
	void Create( void );
	void OnPopup( UNIT_FUNC_PROTOTYPE );
	void OnAttacked( UNIT_FUNC_PROTOTYPE );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
	void OnDestroy( UNIT_FUNC_PROTOTYPE );

};

#endif