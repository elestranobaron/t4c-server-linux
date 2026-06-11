/******************************************************************************
Modify for vs2008 (24/04/2009)
******************************************************************************/


#ifndef __THEOPOLD_H
#define __THEOPOLD_H

class Theopold : public NPCstructure{
public:   
    Theopold();
   ~Theopold();
    void Create( void );
    void OnTalk( UNIT_FUNC_PROTOTYPE );
};

#endif