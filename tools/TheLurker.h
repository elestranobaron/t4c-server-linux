/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/


#ifndef __THELURKER_H
#define __THELURKER_H

class TheLurker : public NPCstructure  
{
public:
	TheLurker();
	virtual ~TheLurker();

	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );

    void OnNPCDataExchange( UNIT_FUNC_PROTOTYPE );
};

#endif // __THELURKER_H
