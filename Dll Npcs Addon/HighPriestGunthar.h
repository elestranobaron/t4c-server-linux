/******************************************************************************
Modify for vs2008 (23/04/2009)
******************************************************************************/


#ifndef __HIGHPRIESTGUNTHAR_H
#define __HIGHPRIESTGUNTHAR_H

class HighPriestGunthar : public NPCstructure{
public:
	HighPriestGunthar();
	~HighPriestGunthar();
	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );

};

#endif