/******************************************************************************
Modify for vs2008 (23/04/2009)
******************************************************************************/


#ifndef __BEDNOMAD1_H
#define __BEDNOMAD1_H

class BedNomad1 : public NPCstructure{
public:   
   BedNomad1();
   ~BedNomad1();
	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
   void OnAttacked( UNIT_FUNC_PROTOTYPE );
   void OnInitialise( UNIT_FUNC_PROTOTYPE );

};

#endif