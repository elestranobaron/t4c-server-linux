/******************************************************************************
Modify for vs2008 (23/04/2009)
******************************************************************************/


#ifndef __BOOKSHELFNOMAD3_H
#define __BOOKSHELFNOMAD3_H

class BookshelfNomad3 : public NPCstructure{
public:   
   BookshelfNomad3();
   ~BookshelfNomad3();
	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
   void OnAttacked( UNIT_FUNC_PROTOTYPE );
   void OnInitialise( UNIT_FUNC_PROTOTYPE );

};

#endif