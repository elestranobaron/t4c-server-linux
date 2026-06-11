/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/


#ifndef __GARNIR_H
#define __GARNIR_H

class Garnir : public NPCstructure  
{
public:
	Garnir();
	virtual ~Garnir();

	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
	void OnNPCDataExchange( UNIT_FUNC_PROTOTYPE );
};

#endif // __GARNIR_H
