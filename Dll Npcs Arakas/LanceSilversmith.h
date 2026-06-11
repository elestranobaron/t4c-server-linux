/******************************************************************************
Modify for vs2008 (23/04/2009)
******************************************************************************/

#ifndef __LANCESILVERSMITH_H
#define __LANCESILVERSMITH_H

class LanceSilversmith : public NPCstructure  
{
public:
	LanceSilversmith();
	virtual ~LanceSilversmith();

	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );

};

#endif 
