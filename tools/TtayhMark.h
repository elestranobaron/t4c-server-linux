/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/


#ifndef __TTAYHMARK_H
#define __TTAYHMARK_H

class TtayhMark : public NPCstructure  
{
public:
	TtayhMark();
	virtual ~TtayhMark();

	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
	void OnNPCDataExchange( UNIT_FUNC_PROTOTYPE );
};

#endif // __TTAYHMARK_H
