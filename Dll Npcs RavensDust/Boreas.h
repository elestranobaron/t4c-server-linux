/******************************************************************************
Modify for vs2008 (24/04/2009)
******************************************************************************/


#ifndef __BOREAS_H
#define __BOREAS_H

class Boreas : public NPCstructure{
public:   
	Boreas();
	~Boreas();
	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
	void OnNPCDataExchange( UNIT_FUNC_PROTOTYPE );
};

#endif
