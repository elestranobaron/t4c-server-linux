/******************************************************************************
Modify for vs2008 (23/04/2009)
******************************************************************************/

#ifndef __ASARR_H
#define __ASARR_H

class Asarr : public NPCstructure  
{
public:
	Asarr();
	virtual ~Asarr();

	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
	void OnNPCDataExchange( UNIT_FUNC_PROTOTYPE );
};

#endif 
