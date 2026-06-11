/******************************************************************************
Modify for vs2008 (23/04/2009)
******************************************************************************/


#ifndef __WINDHOWLSENTRY_H
#define __WINDHOWLSENTRY_H

class WindhowlSentry : public NPCstructure{
public:
	WindhowlSentry();
	~WindhowlSentry();
	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
	void OnAttacked ( UNIT_FUNC_PROTOTYPE ); 
	void OnInitialise( UNIT_FUNC_PROTOTYPE );

};

#endif