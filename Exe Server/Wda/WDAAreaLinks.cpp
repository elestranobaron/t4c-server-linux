/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "WDAAreaLinks.h"
#include "../Format.h"

using namespace std;

/******************************************************************************/
WDAAreaLinks::WDAAreaLinks( Logger &cTraceLogger ) : WDATable( cTraceLogger )
{
}
/******************************************************************************/
WDAAreaLinks::~WDAAreaLinks()
{
}
/******************************************************************************/
// Create from a WDA file.
void WDAAreaLinks::CreateFrom(WDAFile &wdaFile, bool createReadOnly)
/******************************************************************************/
{
    DWORD dwSize = 0xABCDEF;

    TFormat cFormat;
    cOutput.Log(
        dlInfo,
        "\n--WDA--------------"
        "\nLoading area links."
        "\n"
    );

    // Read the quantity of area links.
    wdaFile.Read( dwSize );

    cOutput.Log(
        dlDebugHigh,
        cFormat(
            "\nFound %u area links.",
            dwSize
        )
    );

    // The quantity wasn't read.
    //ASSERT( dwSize != 0xABCDEF );

    DWORD i;
    for( i = 0; i < dwSize; i++ )
	{
        AreaLinkData cAreaData;

        // Fetch source pos.
        wdaFile.Read( cAreaData.wlSourcePos.X );
        wdaFile.Read( cAreaData.wlSourcePos.Y );
        wdaFile.Read( cAreaData.wlSourcePos.world );
        // Fetch target pos.
        wdaFile.Read( cAreaData.wlTargetPos.X );
        wdaFile.Read( cAreaData.wlTargetPos.Y );
        wdaFile.Read( cAreaData.wlTargetPos.world );
        
        cOutput.Log( dlDebugHigh,
            cFormat( "\r\nFound area link from %u, %u, %u to %u, %u, %u.",
                cAreaData.wlSourcePos.X, cAreaData.wlSourcePos.Y, cAreaData.wlSourcePos.world,
                cAreaData.wlTargetPos.X, cAreaData.wlTargetPos.Y, cAreaData.wlTargetPos.world
            )
        );

        cAreaData.m_ReadOnly = createReadOnly;

        // Add area link
        vAreaLinks.push_back( cAreaData );
    }
}
/******************************************************************************/
// Returns the loaded data.
vector< WDAAreaLinks::AreaLinkData > &WDAAreaLinks::GetAreaLinks( void )
/******************************************************************************/
{
    return vAreaLinks;
}