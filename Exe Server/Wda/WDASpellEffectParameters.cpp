/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "WDASpellEffectParameters.h"
#include "../Format.h"

using namespace std;

/******************************************************************************/
WDASpellEffectParameters::WDASpellEffectParameters( Logger &cLogger, DEBUG_LEVEL dlMapDebugHigh ) : WDATable( cLogger )
/******************************************************************************/
{
    MapDebugHighLogLevel( dlMapDebugHigh );
}
/******************************************************************************/
WDASpellEffectParameters::~WDASpellEffectParameters()
/******************************************************************************/
{
}
/******************************************************************************/
//  Returns the paramter value
string &WDASpellEffectParameters::GetParamValue( void )
/******************************************************************************/
{
    return csParamValue;
}
/******************************************************************************/
//  Returns the parameterID.
DWORD WDASpellEffectParameters::GetParamID( void )
/******************************************************************************/
{
    return dwParamID;
}
/******************************************************************************/
// Creates from a wdaFile.
void WDASpellEffectParameters::CreateFrom(WDAFile &wdaFile, bool createReadOnly)
/******************************************************************************/
{
    // Load the parameter ID
    wdaFile.Read( dwParamID );

    // Load the parameter value.
    wdaFile.Read( csParamValue );
    
    TFormat cFormat;
    cOutput.Log(
        dlDebugHigh,
        cFormat(
            "Param( %u, %s ) ",
            dwParamID,
            csParamValue.c_str()
        )
    );
}
/******************************************************************************/
bool WDASpellEffectParameters::operator==( const WDASpellEffectParameters &l )
/******************************************************************************/
{
    return dwParamID == l.dwParamID && csParamValue == l.csParamValue;
}

