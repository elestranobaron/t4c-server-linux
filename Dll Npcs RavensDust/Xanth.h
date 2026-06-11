/******************************************************************************
Modify for vs2008 (24/04/2009)
******************************************************************************/


#ifndef __XANTH_H
#define __XANTH_H

class Xanth : public NPCstructure{
public:   
	Xanth();
	~Xanth();
	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
	void OnNPCDataExchange( UNIT_FUNC_PROTOTYPE );
};

#endif
