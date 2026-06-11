/******************************************************************************
Modify for vs2008 (23/04/2009)
******************************************************************************/


#ifndef __WINDHOWLPORTAL_H
#define __WINDHOWLPORTAL_H

class WindHowlPortal : public NPCstructure{
public:   
	WindHowlPortal();
	~WindHowlPortal();
	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
    void OnAttacked( UNIT_FUNC_PROTOTYPE );
    void OnInitialise( UNIT_FUNC_PROTOTYPE );
};

#endif
