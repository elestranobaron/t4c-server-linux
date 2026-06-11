/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/


#ifndef __COLOSSEUMOWNER_H
#define __COLOSSEUMOWNER_H

class ColosseumOwner : public NPCstructure{
public:
	ColosseumOwner();
	~ColosseumOwner();
	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
	void OnInitialise( UNIT_FUNC_PROTOTYPE );
};

#endif