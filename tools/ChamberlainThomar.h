/******************************************************************************
Modify for vs2008 (24/04/2009)
******************************************************************************/


#ifndef __CHAMBERLAINTHOMAR_H
#define __CHAMBERLAINTHOMAR_H

class ChamberlainThomar : public NPCstructure  
{
public:
	ChamberlainThomar();
	virtual ~ChamberlainThomar();
	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif