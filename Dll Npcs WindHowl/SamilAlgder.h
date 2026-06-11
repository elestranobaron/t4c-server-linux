/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/


#ifndef __SAMILALGDER_H
#define __SAMILALGDER_H

class SamilAlgder : public NPCstructure  
{
public:
	SamilAlgder();
	virtual ~SamilAlgder();

	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif // __SAMILALGDER_H
