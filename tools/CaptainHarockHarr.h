/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/


#ifndef __CAPTAINHAROCKHARR_H
#define __CAPTAINHAROCKHARR_H

class CaptainHarockHarr : public NPCstructure  
{
public:
	CaptainHarockHarr();
	virtual ~CaptainHarockHarr();

	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif // __CAPTAINHAROCKHARR_H
