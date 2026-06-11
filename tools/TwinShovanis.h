/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/

#ifndef __TWINSHOVANIS_H
#define __TWINSHOVANIS_H

class TwinShovanis : public NPCstructure{
public:   
        TwinShovanis();
       ~TwinShovanis();

   void Create( void );
   void OnTalk( UNIT_FUNC_PROTOTYPE );
   void OnNPCDataExchange( UNIT_FUNC_PROTOTYPE );
};

#endif
