/******************************************************************************
Modify for vs2008 (24/04/2009)
******************************************************************************/


#ifndef __BROTHER12_H
#define __BROTHER12_H

class Brother12 : public NPCstructure{
public:   
	Brother12();
	~Brother12();
	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif