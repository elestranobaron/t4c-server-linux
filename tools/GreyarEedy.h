/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/


#ifndef __GREYAREEDY_H
#define __GREYAREEDY_H

class GreyarEedy : public NPCstructure  
{
public:
	GreyarEedy();
	virtual ~GreyarEedy();

	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif // __GREYAREEDY_H
