/******************************************************************************
Modify for vs2008 (23/04/2009)
******************************************************************************/


#ifndef __LIGHTHAVENARENACLERK_H
#define __LIGHTHAVENARENACLERK_H

class LighthavenArenaClerk : public NPCstructure{
public:   
	LighthavenArenaClerk();
	~LighthavenArenaClerk();
	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
    void OnAttacked( UNIT_FUNC_PROTOTYPE );
    void OnInitialise( UNIT_FUNC_PROTOTYPE );
};

#endif