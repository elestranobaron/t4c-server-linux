/******************************************************************************
Modify for vs2008 (06/05/2009)
/******************************************************************************/
#if !defined(AFX_CAPUCHON_H__CC45C3F3_1405_11D1_BCE7_00E029058623__INCLUDED_)
#define AFX_CAPUCHON_H__CC45C3F3_1405_11D1_BCE7_00E029058623__INCLUDED_

#if _MSC_VER >= 1000
	#pragma once
#endif // _MSC_VER >= 1000

#include "Unit.h"

/******************************************************************************/
class Capuchon  
/******************************************************************************/
{
public:
	Capuchon();
	virtual ~Capuchon();

	void  AddUnit(WORD X, WORD Y, Unit* newUnit);
	Unit *ViewTopUnit(WORD X, WORD Y);
	Unit *FetchTopUnit(WORD X, WORD Y);

    Unit *ViewUnitDeep( WORD X, WORD Y, DWORD dwID );
    Unit *FetchUnitDeep ( WORD X, WORD Y, DWORD dwID );

	Unit *SearchUnitType( WORD wX, WORD wY, WORD wUnitType );

	bool IsEmpty();

    void ViewTentacule( WORD wX, WORD wY, vector< Unit * > &vArm );

private:	
    enum { StackGranularity = 4 };

    typedef deque< Unit * > UnitStack;
        
    UnitStack sTentacule[ 4 ][ 4 ];
};

#endif // !defined(AFX_CAPUCHON_H__CC45C3F3_1405_11D1_BCE7_00E029058623__INCLUDED_)
