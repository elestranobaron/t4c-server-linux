/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/


#ifndef __LAONIANRIKARR_H
#define __LAONIANRIKARR_H

class LaonianRikarr : public NPCstructure  
{
public:
	LaonianRikarr();
	virtual ~LaonianRikarr();

	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif // __LAONIANRIKARR_H
