/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/


#ifndef __XOUNDEDGUARD_H
#define __XOUNDEDGUARD_H

class WoundedGuard : public NPCstructure  
{
public:
	WoundedGuard();
	virtual ~WoundedGuard();

	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif // __XOUNDEDGUARD_H
