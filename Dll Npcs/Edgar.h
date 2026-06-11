/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
// Edgar.h: interface for the Edgar class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __EDGAR_H
#define __EDGAR_H

class Edgar : public NPCstructure{
public:   
    Edgar();
   ~Edgar();
    void Create( void );
    void OnTalk( UNIT_FUNC_PROTOTYPE );
	void OnDeath( UNIT_FUNC_PROTOTYPE);
};

#endif
