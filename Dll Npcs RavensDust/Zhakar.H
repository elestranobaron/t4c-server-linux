/******************************************************************************
Modify for vs2008 (24/04/2009)
******************************************************************************/


#ifndef __ZHAKAR_H
#define __ZHAKAR_H

class Zhakar : public NPCstructure{
public:   
	Zhakar();
	~Zhakar();
	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
	void OnInitialise( UNIT_FUNC_PROTOTYPE );
	void OnAttacked( UNIT_FUNC_PROTOTYPE );
	void OnNPCDataExchange( UNIT_FUNC_PROTOTYPE );
};

#endif
