/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "WDALarvae.h"
#include "../Format.h"

using namespace std;

/******************************************************************************/
WDALarvae::WDALarvae( Logger &cLogger, DEBUG_LEVEL dlDebugHigh ) : WDATable( cLogger )
/******************************************************************************/
{
    MapDebugHighLogLevel( dlDebugHigh );
}
/******************************************************************************/
WDALarvae::~WDALarvae()
/******************************************************************************/
{
}
/******************************************************************************/
//  Returns the larva ID
const string &WDALarvae::GetLarvaID( void )
/******************************************************************************/
{
    return csLarvaID;
}
/******************************************************************************/
// Creates from a wdaFile.
void WDALarvae::CreateFrom(WDAFile &wdaFile, bool createReadOnly)
/******************************************************************************/
{
    wdaFile.Read( csLarvaID );

    TFormat cFormat;
    cOutput.Log(
        dlDebugHigh,
        cFormat(
            "\n  Found larva %s.",
            csLarvaID.c_str()
        )
    );
}

