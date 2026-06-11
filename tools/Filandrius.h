/******************************************************************************
Modify for vs2008 (23/04/2009)
******************************************************************************/


#ifndef __FILANDRIUS_H
#define __FILANDRIUS_H

class Filandrius : public NPCstructure{
public:
	Filandrius();
	~Filandrius();
	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
	void OnAttacked ( UNIT_FUNC_PROTOTYPE ); 
	void OnInitialise( UNIT_FUNC_PROTOTYPE );
	void OnNPCDataExchange( UNIT_FUNC_PROTOTYPE );

};

#endif