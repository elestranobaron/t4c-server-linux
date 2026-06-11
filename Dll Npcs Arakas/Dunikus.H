/******************************************************************************
Modify for vs2008 (23/04/2009)
******************************************************************************/

#ifndef __DUNIKUS_H
#define __DUNIKUS_H

class Dunikus : public NPCstructure{
public:   
    Dunikus();
   ~Dunikus();

   void Create( void );
   void OnTalk( UNIT_FUNC_PROTOTYPE );
   void OnNPCDataExchange( UNIT_FUNC_PROTOTYPE );
};

#endif
