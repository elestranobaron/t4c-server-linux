/******************************************************************************
Modify for vs2008 (23/04/2009)
******************************************************************************/


#ifndef __EFNISIEN_H
#define __EFNISIEN_H

class Efnisien : public NPCstructure{
public:
	Efnisien();
	~Efnisien();
	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );

};

#endif