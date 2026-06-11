/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "WDAObjectsSpells.h"
#include "../Format.h"

using namespace std;

/******************************************************************************/
WDAObjectsSpells::WDAObjectsSpells( Logger &cLogger, DEBUG_LEVEL dlHigh ) : WDATable( cLogger )
/******************************************************************************/
{
    MapDebugHighLogLevel( dlHigh );
}
/******************************************************************************/
WDAObjectsSpells::~WDAObjectsSpells()
/******************************************************************************/
{
}
/******************************************************************************/
// Returns the spell ID
DWORD WDAObjectsSpells::GetSpellID( void )
/******************************************************************************/
{
    return dwSpellID;
}
/******************************************************************************/
//  Returns the message on which the spell is hooked.
DWORD WDAObjectsSpells::GetMessageHook( void )
/******************************************************************************/
{
    return dwMessageHook;
}
/******************************************************************************/
// Returns the level (??)
DWORD WDAObjectsSpells::GetLevel( void )
/******************************************************************************/
{
    return dwLevel;
}
/******************************************************************************/
// Creates from a wdaFile.
void WDAObjectsSpells::CreateFrom(WDAFile &wdaFile, bool createReadOnly)
/******************************************************************************/
{
    wdaFile.Read( dwSpellID );
    wdaFile.Read( dwMessageHook );
    wdaFile.Read( dwLevel );

    TFormat cFormat;
    cOutput.Log(
        dlDebugHigh,
        cFormat(
            "\n Writing spell %u instilled on %u. Level %u.",
            dwSpellID,
            dwMessageHook,
            dwLevel
        )
    );
}

