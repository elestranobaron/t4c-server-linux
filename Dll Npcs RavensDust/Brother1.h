/******************************************************************************
Modify for vs2008 (24/04/2009)
******************************************************************************/


#ifndef __BROTHER1_H
#define __BROTHER1_H

class Brother1 : public NPCstructure{
public:   
	Brother1();
	~Brother1();
	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif