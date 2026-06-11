/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/


#ifndef __MIRAKNIRA_H
#define __MIRAKNIRA_H

class MirakNira : public NPCstructure  
{
public:
	MirakNira();
	virtual ~MirakNira();

	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif // __MIRAKNIRA_H
