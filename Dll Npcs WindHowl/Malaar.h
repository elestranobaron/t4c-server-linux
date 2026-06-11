/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/


#ifndef __MALAAR_H
#define __MALAAR_H

class Malaar : public NPCstructure  
{
public:
	Malaar();
	virtual ~Malaar();

	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif // __MALAAR_H
