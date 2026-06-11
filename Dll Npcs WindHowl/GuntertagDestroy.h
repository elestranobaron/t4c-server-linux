/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/


#ifndef __GUNTHARGDESTROY_H
#define __GUNTHARGDESTROY_H

class GuntertagDestroy : public NPCstructure  
{
public:
	GuntertagDestroy();
	virtual ~GuntertagDestroy();

	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif // __GUNTHARGDESTROY_H
