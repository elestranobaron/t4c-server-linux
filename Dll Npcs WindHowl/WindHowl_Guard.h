/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/


#ifndef __WINDHOWLGUARD_H
#define __WINDHOWLGUARD_H

class WindHowl_Guard : public NPCstructure  
{
public:
	WindHowl_Guard();
	virtual ~WindHowl_Guard();

	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif // __WINDHOWLGUARD_H
