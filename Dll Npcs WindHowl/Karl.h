/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/


#ifndef __KARL_H
#define __KARL_H

class Karl : public NPCstructure  
{
public:
	Karl();
	virtual ~Karl();

	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
	void OnNPCDataExchange( UNIT_FUNC_PROTOTYPE );
};

#endif // __KARL_H
