/******************************************************************************
Modify for vs2008 (23/04/2009)
******************************************************************************/


#ifndef __ISLANDPORTAL24_H
#define __ISLANDPORTAL24_H

class IslandPortal24 : public NPCstructure{
public:   
	IslandPortal24();
	~IslandPortal24();
	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
    void OnAttacked( UNIT_FUNC_PROTOTYPE );
    void OnInitialise( UNIT_FUNC_PROTOTYPE );
};

#endif
