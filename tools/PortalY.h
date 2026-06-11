/******************************************************************************
Modify for vs2008 (23/04/2009)
******************************************************************************/


#ifndef __PORTALY_H
#define __PORTALY_H

class PortalY : public NPCstructure{
public:   
	PortalY();
	~PortalY();
	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
	void OnAttacked( UNIT_FUNC_PROTOTYPE );
	void OnInitialise( UNIT_FUNC_PROTOTYPE );
};

#endif
