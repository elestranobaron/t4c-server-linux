#include "SimpleMonster.h"

#ifndef __MOBDARKFIEND_H
#define __MOBDARKFIEND_H

class MOBDarkfiend : public SimpleMonster{
public:
	MOBDarkfiend();
	~MOBDarkfiend();
	void Create( void );
	void OnPopup( UNIT_FUNC_PROTOTYPE );
	void OnDeath( UNIT_FUNC_PROTOTYPE );
};

#endif