/******************************************************************************
Modify for vs2008 (23/04/2009)
******************************************************************************/


#ifndef __HARVESTEROFLIFE_H
#define __HARVESTEROFLIFE_H

class HarvesterOfLife : public NPCstructure{
public:
	HarvesterOfLife();
	~HarvesterOfLife();
	void Create( void );
	void OnDeath( UNIT_FUNC_PROTOTYPE );

};

#endif