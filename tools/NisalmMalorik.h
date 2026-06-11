/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/


#ifndef __NISALMALORIK_H
#define __NISALMALORIK_H

class NisalmMalorik : public NPCstructure  
{
public:
	NisalmMalorik();
	virtual ~NisalmMalorik();

	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif // __NISALMALORIK_H
