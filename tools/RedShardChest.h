/******************************************************************************
Modify for vs2008 (23/04/2009)
******************************************************************************/


#ifndef __REDSHARDCHEST_H
#define __REDSHARDCHEST_H

class RedShardChest : public NPCstructure{
public:   
   RedShardChest();
   ~RedShardChest();
	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
   void OnAttacked( UNIT_FUNC_PROTOTYPE );
   void OnInitialise( UNIT_FUNC_PROTOTYPE );

};

#endif
