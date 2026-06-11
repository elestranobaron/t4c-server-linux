/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/


#ifndef __MARSACCRED_H
#define __MARSACCRED_H

class MarsacCred : public NPCstructure  
{
public:
	MarsacCred();
	virtual ~MarsacCred();

	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
    void OnNPCDataExchange( UNIT_FUNC_PROTOTYPE );
};

#endif // __MARSACCRED_H
