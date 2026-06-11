/******************************************************************************
Modify for vs2008 (24/04/2009)
******************************************************************************/


#ifndef __BISHOPCROWBANNER_H
#define __BISHOPCROWBANNER_H

class BishopCrowbanner: public NPCstructure{
public:   
	BishopCrowbanner();
	~BishopCrowbanner();
	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
	void OnNPCDataExchange( UNIT_FUNC_PROTOTYPE );
};

#endif
