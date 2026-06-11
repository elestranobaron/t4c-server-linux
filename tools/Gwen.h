/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/


#ifndef __GWEN_H
#define __GWEN_H

class Gwen : public NPCstructure  
{
public:
	Gwen();
	virtual ~Gwen();

	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
	void OnNPCDataExchange( UNIT_FUNC_PROTOTYPE );
};

#endif // __GWEN_H
