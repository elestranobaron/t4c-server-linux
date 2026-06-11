/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/


#ifndef __BORTHERHARFORR_H
#define __BORTHERHARFORR_H

class BrotherHarforr : public NPCstructure  
{
public:
	BrotherHarforr();
	virtual ~BrotherHarforr();

	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif // __BORTHERHARFORR_H
