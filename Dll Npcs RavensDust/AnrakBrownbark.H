/******************************************************************************
Modify for vs2008 (24/04/2009)
******************************************************************************/


#ifndef __ANRAKBROWNBARK_H
#define __ANRAKBROWNBARK_H

class AnrakBrownbark : public NPCstructure{
public:   
	AnrakBrownbark();
	~AnrakBrownbark();
	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif
