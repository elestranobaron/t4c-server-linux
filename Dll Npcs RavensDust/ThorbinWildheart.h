/******************************************************************************
Modify for vs2008 (24/04/2009)
******************************************************************************/


#ifndef __THORBINWILDHEART_H
#define __THORBINWILDHEART_H

class ThorbinWildheart : public NPCstructure{
public:   
    ThorbinWildheart();
    ~ThorbinWildheart();
    void Create( void );
    void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif
