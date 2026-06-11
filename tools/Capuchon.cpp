/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "TFC Server.h"
#include "Capuchon.h"
#include "Unit.h"

#ifdef _DEBUG
	#undef THIS_FILE
	static char THIS_FILE[]=__FILE__;
	#define new DEBUG_NEW
#endif

/******************************************************************************/
Capuchon::Capuchon()
/******************************************************************************/
{
}
/******************************************************************************/
Capuchon::~Capuchon()
/******************************************************************************/
{
   int i, j;
   for( i = 0; i < 4; i++ )
   {
      for( j = 0; j < 4; j++ )
      {
         while( !sTentacule[ i ][ j ].empty() )
         {
            Unit *lpUnit = sTentacule[ i ][ j ].back();
            sTentacule[ i ][ j ].pop_back();
            try
            {
               if(lpUnit)
               {
                  lpUnit->DeleteUnit();
                  lpUnit = NULL;
               }
            }
            catch(...)
            {
            }
         }
      }
   }
}
/******************************************************************************/
void Capuchon::AddUnit(WORD X, WORD Y, Unit *newUnit)
/******************************************************************************/
{
	// Cannot add NULL unit to a tentacule!
	ASSERT( newUnit != NULL );
	if( newUnit != NULL )
	{
		sTentacule[ X % 4 ][ Y % 4 ].push_front(newUnit);
	}
}
/******************************************************************************/
// this function returns the top stacked unit without removing it.
Unit *Capuchon::ViewTopUnit(WORD X, WORD Y)
/******************************************************************************/
{
	// If the stack is empty, return a NULL unit.
	if( sTentacule[ X % 4 ][ Y % 4 ].empty() )
	{
		return NULL;
	}

	// Otherwise return the stack's top.
	return sTentacule[ X % 4 ][ Y % 4 ].front();
}
/******************************************************************************/
Unit *Capuchon::FetchTopUnit(WORD X, WORD Y)
/******************************************************************************/
{
	// If the stack is empty, return a NULL unit.
	if( sTentacule[ X % 4 ][ Y % 4 ].empty() )
	{
		return NULL;
	}
    // Otherwise fetch the top object and pop it.
    Unit *lpTopUnit = sTentacule[ X % 4 ][ Y % 4 ].front();
    sTentacule[ X % 4 ][ Y % 4 ].pop_front();

    // Units on tentacules cannot be NULL!
    ASSERT( lpTopUnit != NULL );
    
    return lpTopUnit;
}
/******************************************************************************/
// Views a unit anywhere in the capuchon's stack.
Unit *Capuchon::ViewUnitDeep(
 WORD wX,    // X pos of stack.
 WORD wY,    // Y pos of stack.
 DWORD dwID // Unit ID to search for.
)
/******************************************************************************/
{
    UnitStack &sArm = sTentacule[ wX % 4 ][ wY % 4 ];

    UnitStack::iterator z;
    for( z = sArm.begin(); z != sArm.end(); z++ )
	{
        Unit *lpUnit = (*z);
    
        // Units on tentacules cannot be NULL!
        ASSERT( lpUnit != NULL );

        if( lpUnit != NULL )
		{
		    // Is unit of type we want?
		    if( lpUnit->GetID() == dwID )
			{
			    return lpUnit;
            }
        }
    }

    return NULL;
}
/******************************************************************************/
//  Returns a unit inside a capchon's stack. If the item was found, it is removed from the stack.
Unit *Capuchon::FetchUnitDeep(
 WORD wX,        // X pos of the capuchon's stack.
 WORD wY,        // Y pos of the capuchon's stack.
 DWORD dwID     // The ID to search for.
)
/******************************************************************************/
{
    UnitStack &sArm = sTentacule[ wX % 4 ][ wY % 4 ];

    UnitStack::iterator z;
    for( z = sArm.begin(); z != sArm.end(); z++ )
	{
        Unit *lpUnit = *z;

        // A unit on a slinky cannot be NULL.
        ASSERT( lpUnit != NULL );       

		// Is unit of type we want?
		if( lpUnit->GetID() == dwID )
		{
            // Remove the unit from the unit stack.
            sArm.erase( z );

            return lpUnit;
        }
	}

    return NULL;
}
/******************************************************************************/
// Returns all the units in an arm, does NOT change the stack's content!
void Capuchon::ViewTentacule(
 WORD wX,               // The arm's X position.
 WORD wY,               // The arm's Y position.
 vector< Unit * > &vArm // The container in which to put the arm's data.
)
/******************************************************************************/
{
    UnitStack &sArm = sTentacule[ wX % 4 ][ wY % 4 ];

    UnitStack::iterator i;

    // Scroll through the whole tentacule.
    for( i = sArm.begin(); i != sArm.end(); i++ )
	{
        Unit *lpUnit = (*i);
        
        // Units on a tentacule cannot be NULL!
        ASSERT( lpUnit != NULL );

        // Add each unit to the provided container.
        vArm.push_back( lpUnit );
    }
}
/******************************************************************************/
bool Capuchon::IsEmpty()
/******************************************************************************/
{
	int i, j;	
	
    // Scans the capuchon and returns TRUE if all tentacules are empty	
	for(i = 0; i < 4; i++)
	{
		for(j = 0; j < 4; j++)
		{
			// returns FALSE if any stack still has something
            if( !sTentacule[ i ][ j ].empty() )
			{
				return false;
			}
		}
	}
    return true;
}
/******************************************************************************/
// Searches for a specific unit type, returns the first occurence if this item.
Unit *Capuchon::SearchUnitType(
 WORD wX,		// X position of item in capuchon
 WORD wY,		// Y position of item in capuchon
 WORD wUnitType // Unit type.
)
/******************************************************************************/
{	
    UnitStack &sArm = sTentacule[ wX % 4 ][ wY % 4 ];
    
    UnitStack::iterator z;
    for( z = sArm.begin(); z != sArm.end(); z++ )
	{
        Unit *lpUnit = *z;

        // Units on a tentacule cannot be NULL!
        ASSERT( lpUnit != NULL );

		// Is unit of type we want?
		if( lpUnit->GetType() == wUnitType ){
			return lpUnit;
        }
	}

	return NULL;
}