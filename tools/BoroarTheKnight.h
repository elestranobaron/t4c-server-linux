/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/


#ifndef __BOROARTHEKNIGHT_H
#define __BOROARTHEKNIGHT_H

class BoroarTheKnight : public NPCstructure  
{
public:
	BoroarTheKnight();
	virtual ~BoroarTheKnight();

	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif // __BOROARTHEKNIGHT_H
