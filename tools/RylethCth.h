/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/


#ifndef __RULETHCTH_H
#define __RULETHCTH_H

class RylethCth : public NPCstructure  
{
public:
	RylethCth();
	virtual ~RylethCth();

	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
	void OnNPCDataExchange( UNIT_FUNC_PROTOTYPE );
};

#endif // __RULETHCTH_H
