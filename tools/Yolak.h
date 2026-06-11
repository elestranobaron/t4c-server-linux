/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/


#ifndef __YOLAK_H
#define __YOLAK_H

class Yolak : public NPCstructure  
{
public:
	Yolak();
	virtual ~Yolak();

	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
	void OnNPCDataExchange( UNIT_FUNC_PROTOTYPE );
};

#endif // __YOLAK_H
