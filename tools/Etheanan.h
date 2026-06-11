/******************************************************************************
Modify for vs2008 (24/04/2009)
******************************************************************************/


#ifndef __ETHEANAN_H
#define __ETHEANAN_H

class Etheanan : public NPCstructure{
public:   
	Etheanan();
	~Etheanan();
	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
	void OnNPCDataExchange( UNIT_FUNC_PROTOTYPE );
};

#endif
