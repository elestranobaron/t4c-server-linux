/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/


#ifndef __ARGANORIARGH_H
#define __ARGANORIARGH_H

class ArganorIargh : public NPCstructure  
{
public:
	ArganorIargh();
	virtual ~ArganorIargh();

	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
	void OnNPCDataExchange( UNIT_FUNC_PROTOTYPE );
};

#endif // __ARGANORIARGH_H
