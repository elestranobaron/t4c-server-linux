/******************************************************************************
Modify for vs2008 (24/04/2009)
******************************************************************************/


#ifndef __BROTHER10_H
#define __BROTHER10_H

class Brother10 : public NPCstructure{
public:   
	Brother10();
	~Brother10();
	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif