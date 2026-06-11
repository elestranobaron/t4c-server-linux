/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "WDAObjectsAttrBoosts.h"
#include "../Format.h"

using namespace std;

/******************************************************************************/
WDAObjectsAttrBoosts::WDAObjectsAttrBoosts( Logger &cLogger, DEBUG_LEVEL dlDebugHigh ) : WDATable( cLogger )
/******************************************************************************/
{
    MapDebugHighLogLevel( dlDebugHigh );
}
/******************************************************************************/
WDAObjectsAttrBoosts::~WDAObjectsAttrBoosts()
/******************************************************************************/
{
}
/******************************************************************************/
// Returns the boost ID.
DWORD WDAObjectsAttrBoosts::GetID( void )
/******************************************************************************/
{
    return dwID;
}
/******************************************************************************/
// Returns the boosted stat
DWORD WDAObjectsAttrBoosts::GetStat( void )
/******************************************************************************/
{
    return dwStat;
}
/******************************************************************************/
// Returns the numerical boost.
string WDAObjectsAttrBoosts::GetBoost( void )
/******************************************************************************/
{
    return bsBoost;
}
/******************************************************************************/
//  Returns the minimum intellect requirement.
DWORD WDAObjectsAttrBoosts::GetMinInt( void )
/******************************************************************************/
{
    return dwMinInt;
}
/******************************************************************************/
//  Returns the minimum wisdom requirement.
DWORD WDAObjectsAttrBoosts::GetMinWis( void )
/******************************************************************************/
{
    return dwMinWis;
}
/******************************************************************************/
// Creates from a wdaFile.
void WDAObjectsAttrBoosts::CreateFrom(WDAFile &wdaFile, bool createReadOnly)
/******************************************************************************/
{
    wdaFile.Read( dwID );
    wdaFile.Read( dwStat );
    wdaFile.Read( bsBoost );
    wdaFile.Read( dwMinInt );
    wdaFile.Read( dwMinWis );

    TFormat cFormat;
    cOutput.Log(
        dlDebugHigh,
        cFormat(
            "\nBoost %u: Stat %u, value %s, MinInt %u, MinWis %u.",
            dwID,
            dwStat,
            bsBoost.c_str(),
            dwMinInt,
            dwMinWis
        )
    );
}

