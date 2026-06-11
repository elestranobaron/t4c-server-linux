/******************************************************************************
Modify for vs2008 (24/04/2009)
******************************************************************************/


#ifndef __BROTHER2_H
#define __BROTHER2_H

class Brother2 : public NPCstructure{
public:   
	Brother2();
	~Brother2();
	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif