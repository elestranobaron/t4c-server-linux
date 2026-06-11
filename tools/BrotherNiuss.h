/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/


#ifndef __BROTHERNIUSS_H
#define __BROTHERNIUSS_H

class BrotherNiuss : public NPCstructure  
{
public:
	BrotherNiuss();
	virtual ~BrotherNiuss();

	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif // __BROTHERNIUSS_H
