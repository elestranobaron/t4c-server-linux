/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/


#ifndef __LIURNCLAR_H
#define __LIURNCLAR_H

class LiurnClar : public NPCstructure  
{
public:
	LiurnClar();
	virtual ~LiurnClar();

	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
	void OnNPCDataExchange( UNIT_FUNC_PROTOTYPE );
};

#endif // __LIURNCLAR_H
