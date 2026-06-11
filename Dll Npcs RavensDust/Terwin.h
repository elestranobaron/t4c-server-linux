/******************************************************************************
Modify for vs2008 (24/04/2009)
******************************************************************************/


#ifndef __TERWIN_H
#define __TERWIN_H

class Terwin : public NPCstructure{
public:   
    Terwin();
    ~Terwin();
    void Create( void );
    void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif
