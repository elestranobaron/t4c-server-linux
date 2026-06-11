/******************************************************************************
Modify for vs2008 (24/04/2009)
******************************************************************************/


#ifndef __KIADUS_H
#define __KIADUS_H

class Kiadus : public NPCstructure{
public:   
	Kiadus();
	~Kiadus();
	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
    void OnNPCDataExchange( UNIT_FUNC_PROTOTYPE );
};

#endif
