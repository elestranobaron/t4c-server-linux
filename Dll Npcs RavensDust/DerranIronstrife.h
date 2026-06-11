/******************************************************************************
Modify for vs2008 (24/04/2009)
******************************************************************************/


#ifndef __DERRANIRONSTRIFE_H
#define __DERRANIRONSTRIFE_H

class DerranIronstrife : public NPCstructure{
public:   
	DerranIronstrife();
	~DerranIronstrife();
	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
	void OnNPCDataExchange( UNIT_FUNC_PROTOTYPE );
};

#endif
