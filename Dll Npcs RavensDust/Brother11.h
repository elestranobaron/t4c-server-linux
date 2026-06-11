/******************************************************************************
Modify for vs2008 (24/04/2009)
******************************************************************************/


#ifndef __BROTHER11_H
#define __BROTHER11_H

class Brother11 : public NPCstructure{
public:   
	Brother11();
	~Brother11();
	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif