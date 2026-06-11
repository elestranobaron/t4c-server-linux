/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "WDAObjectsWorldObjects.h"
#include "../Format.h"

using namespace std;

/******************************************************************************/
WDAObjectsWorldObjects::WDAObjectsWorldObjects( Logger &cLogger, DEBUG_LEVEL dlHigh ) : WDATable( cLogger )
/******************************************************************************/
{
    m_ReadOnly = false;
    MapDebugHighLogLevel( dlHigh );
}
/******************************************************************************/
WDAObjectsWorldObjects::~WDAObjectsWorldObjects()
/******************************************************************************/
{
}
/******************************************************************************/
// Returns the position of this item.
WDAObjectsWorldObjects::WorldPos WDAObjectsWorldObjects::GetPos( void )
/******************************************************************************/
{
    return wlPos;
}
/******************************************************************************/
//  Returns the ID of this item
const std::string &WDAObjectsWorldObjects::GetItemID( void )
/******************************************************************************/
{
    return csItemID;
}
/******************************************************************************/
// Creates from a wdaFile.
void WDAObjectsWorldObjects::CreateFrom(WDAFile &wdaFile, bool createReadOnly)
/******************************************************************************/
{
    wdaFile.Read( csItemID );
    wdaFile.Read( wlPos.X );
    wdaFile.Read( wlPos.Y );
    wdaFile.Read( wlPos.world );

    TFormat cFormat;
    cOutput.Log(
        dlDebugHigh,
        cFormat(
            "\n Found object %s at %u, %u, %u.",
            csItemID.c_str(),
            wlPos.X,
            wlPos.Y,
            wlPos.world
        )
    );

    m_ReadOnly = createReadOnly;
}

