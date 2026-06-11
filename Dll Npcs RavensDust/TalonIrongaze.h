/******************************************************************************
Modify for vs2008 (24/04/2009)
******************************************************************************/


#ifndef __TALONIRONGAZE_H
#define __TALONIRONGAZE_H

class TalonIrongaze : public NPCstructure{
public:   
    TalonIrongaze();
    ~TalonIrongaze();
    void Create( void );
    void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif
