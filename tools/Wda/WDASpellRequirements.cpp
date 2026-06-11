/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "WDASpellRequirements.h"
#include "../Format.h"

using namespace std;

/******************************************************************************/
WDASpellRequirements::WDASpellRequirements( Logger &cLogger, DEBUG_LEVEL dlMapDebugHigh ) : WDATable( cLogger )
/******************************************************************************/
{
    MapDebugHighLogLevel( dlMapDebugHigh );
}
/******************************************************************************/
WDASpellRequirements::~WDASpellRequirements()
/******************************************************************************/
{

}
/******************************************************************************/
// The required spell ID.
DWORD WDASpellRequirements::GetRequiredSpellID( void )
/******************************************************************************/
{
    return dwRequiredSpellID;
}
/******************************************************************************/
// Creates from a wdaFile.
void WDASpellRequirements::CreateFrom(WDAFile &wdaFile, bool createReadOnly)
/******************************************************************************/
{
    wdaFile.Read( dwRequiredSpellID );

    TFormat cFormat;
    cOutput.Log(
        dlDebugHigh,
        cFormat( 
            "\n  Requires spell %u.",
            dwRequiredSpellID
        )
    );
}
