/******************************************************************************
Modify for vs2008 (23/04/2009)
******************************************************************************/


#ifndef __ISLANDPORTAL35_H
#define __ISLANDPORTAL35_H

class IslandPortal35 : public NPCstructure{
public:   
	IslandPortal35();
	~IslandPortal35();
	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
    void OnAttacked( UNIT_FUNC_PROTOTYPE );
    void OnInitialise( UNIT_FUNC_PROTOTYPE );
};

#endif
