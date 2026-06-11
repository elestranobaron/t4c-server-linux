/******************************************************************************
Modify for vs2008 (23/04/2009)
******************************************************************************/


#ifndef __PORTALR_H
#define __PORTALR_H

class PortalR : public NPCstructure{
public:   
	PortalR();
	~PortalR();
	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
	void OnAttacked( UNIT_FUNC_PROTOTYPE );
	void OnInitialise( UNIT_FUNC_PROTOTYPE );
};

#endif
