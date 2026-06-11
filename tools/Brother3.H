/******************************************************************************
Modify for vs2008 (24/04/2009)
******************************************************************************/


#ifndef __BROTHER3_H
#define __BROTHER3_H

class Brother3 : public NPCstructure{
public:   
	Brother3();
	~Brother3();
	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif