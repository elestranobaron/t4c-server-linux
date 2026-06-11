/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/


#ifndef __LORDSUNROCK_H
#define __LORDSUNROCK_H

class LordSunrock : public NPCstructure  
{
public:
	LordSunrock();
	virtual ~LordSunrock();

	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif // __LORDSUNROCK_H
