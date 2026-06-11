/******************************************************************************
Modify for vs2008 (23/04/2009)
******************************************************************************/


#ifndef __EMPTYDUNGEONPORTAL_H
#define __EMPTYDUNGEONPORTAL_H

class EmptyDungeonPortal : public NPCstructure{
public:   
   EmptyDungeonPortal();
   ~EmptyDungeonPortal();
	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
    void OnAttacked( UNIT_FUNC_PROTOTYPE );
    void OnInitialise( UNIT_FUNC_PROTOTYPE );
};

#endif
