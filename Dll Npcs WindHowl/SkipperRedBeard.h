/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/


#ifndef __SKIPPERREDBEARD_H
#define __SKIPPERREDBEARD_H

class SkipperRedBeard : public NPCstructure  
{
public:
	SkipperRedBeard();
	virtual ~SkipperRedBeard();

	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif // __SKIPPERREDBEARD_H
