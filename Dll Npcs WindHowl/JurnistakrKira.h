/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/


#ifndef __JURNISTAKRKIRA_H
#define __JURNISTAKRKIRA_H

class JurnistakrKira : public NPCstructure  
{
public:
	JurnistakrKira();
	virtual ~JurnistakrKira();

	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif // __JURNISTAKRKIRA_H
