/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "WDAObjectsContainerItems.h"
#include "../Format.h"

using namespace std;

/******************************************************************************/
WDAObjectsContainerItems::WDAObjectsContainerItems( Logger &cLogger, DEBUG_LEVEL dlDebugHigh ) : WDATable( cLogger )
/******************************************************************************/
{
    MapDebugHighLogLevel( dlDebugHigh );
}
/******************************************************************************/
WDAObjectsContainerItems::~WDAObjectsContainerItems()
/******************************************************************************/
{
}
/******************************************************************************/
// Returns the item ID.
const string &WDAObjectsContainerItems::GetItemID( void ) const
/******************************************************************************/
{
    return csItemID;
}
/******************************************************************************/
// Creates from a wdaFile.
void WDAObjectsContainerItems::CreateFrom(WDAFile &wdaFile, bool createReadOnly)
/******************************************************************************/
{
    wdaFile.Read( csItemID );

    TFormat cFormat;
    cOutput.Log(
        dlDebugHigh,
        cFormat( 
            " %s",
            csItemID.c_str()
        )
    );
}

