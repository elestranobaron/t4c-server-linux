/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/


#ifndef __ERAKA_H
#define __ERAKA_H

class Eraka : public NPCstructure  
{
public:
	Eraka();
	virtual ~Eraka();

	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif // __ERAKA_H
