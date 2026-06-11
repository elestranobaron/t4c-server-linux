/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "WDAObjectsContainerGroups.h"
#include "../Format.h"

using namespace std;

/******************************************************************************/
WDAObjectsContainerGroups::WDAObjectsContainerGroups( Logger &cLogger, DEBUG_LEVEL dlDebugHigh ) : WDATable( cLogger )
/******************************************************************************/
{
    MapDebugHighLogLevel( dlDebugHigh );
}
/******************************************************************************/
WDAObjectsContainerGroups::~WDAObjectsContainerGroups()
/******************************************************************************/
{
}
/******************************************************************************/
// Returns the vector of items in this container group.
vector< WDAObjectsContainerItems > &WDAObjectsContainerGroups::GetItems( void )
/******************************************************************************/
{
    return vItems;
}
/******************************************************************************/
// Creates from a wdaFile.
void WDAObjectsContainerGroups::CreateFrom(WDAFile &wdaFile, bool createReadOnly)
/******************************************************************************/
{
    DWORD dwSize = 0xABCDEF;

    // Read the quantity of container items.
    wdaFile.Read( dwSize );

    cOutput.Log(
        dlDebugHigh,
        "\n Container: "
    );

    // Load all container items.
    DWORD i;
    for( i = 0; i != dwSize; i++ )
	{        
        // Create a new WDAObjectsContainerItem from the wdaFile
        WDAObjectsContainerItems cItem( cOutput, dlDebugHigh );
        cItem.CreateFrom( wdaFile, createReadOnly );

        // Add it to the list of container items.
        vItems.push_back( cItem );
    }
}
/******************************************************************************/
bool WDAObjectsContainerGroups::operator == ( const WDAObjectsContainerGroups &g )
/******************************************************************************/
{
    if( g.vItems.size() != vItems.size() )
	{
        return false;
    }

    int i;
    for( i = 0; i < vItems.size(); i++ )
	{
        if( !( vItems[ i ] == g.vItems[ i ] ) )
		{
            return false;            
        }
    }
    return true;
}

