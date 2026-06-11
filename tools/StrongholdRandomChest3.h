/******************************************************************************
Modify for vs2008 (23/04/2009)
******************************************************************************/


#ifndef __STRONGHOLDRANDOMCHEST3_H
#define __STRONGHOLDRANDOMCHEST3_H

class StrongholdRandomChest3 : public NPCstructure{
public:
  StrongholdRandomChest3();
  ~StrongholdRandomChest3();
	void Create( void );
	void OnAttacked (UNIT_FUNC_PROTOTYPE);
	void OnTalk( UNIT_FUNC_PROTOTYPE );
	void OnInitialise( UNIT_FUNC_PROTOTYPE );
};

#endif
