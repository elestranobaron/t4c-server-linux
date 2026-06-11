/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/


#ifndef __IROUALKNOWALL_H
#define __IROUALKNOWALL_H

class IroualKnowall : public NPCstructure  
{
public:
	IroualKnowall();
	virtual ~IroualKnowall();

	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif // __IROUALKNOWALL_H
