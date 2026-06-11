/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/


#ifndef __MUSHN_H
#define __MUSHN_H

class Mushn : public NPCstructure  
{
public:
	Mushn();
	virtual ~Mushn();

	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif // __MUSHN_H
