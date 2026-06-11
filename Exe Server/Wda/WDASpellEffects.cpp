/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "WDASpellEffects.h"
#include "../Format.h"

using namespace std;

/******************************************************************************/
WDASpellEffects::WDASpellEffects( Logger &cLogger, DEBUG_LEVEL dlMapDebugHigh ) : WDATable( cLogger )
/******************************************************************************/
{
    MapDebugHighLogLevel( dlMapDebugHigh );
}
/******************************************************************************/
WDASpellEffects::~WDASpellEffects()
/******************************************************************************/
{
}
/******************************************************************************/
// The ID of the effect structure.
DWORD WDASpellEffects::GetEffectStructureID( void )
/******************************************************************************/
{
    return dwEffectStructureID;
}
/******************************************************************************/
// The spell effect parameters.
vector< WDASpellEffectParameters > &WDASpellEffects::GetParams( void )
/******************************************************************************/
{
    return vParams;
}
/******************************************************************************/
// Creates from a wdaFile.
void WDASpellEffects::CreateFrom(WDAFile &wdaFile, bool createReadOnly)
/******************************************************************************/
{
    // Load the effect structure ID
    wdaFile.Read( dwEffectStructureID );

    TFormat cFormat;
    cOutput.Log(
        dlDebugHigh,
        cFormat(
            "\n  Effect %u: ",
            dwEffectStructureID
        )
    );

    DWORD dwSize = 0xABCDEF;

    // Get the quantity of effect parameters.
    wdaFile.Read( dwSize );

    // Scroll through all found parameters.
    DWORD i;
    for( i = 0; i < dwSize; i++ )
	{
        // Load the spell effect parameter
        WDASpellEffectParameters cParam( cOutput, dlDebugHigh );
        cParam.CreateFrom( wdaFile, createReadOnly );
        
        // Add the spell effect parameter to the list of parameters.
        vParams.push_back( cParam );
    }
}
/******************************************************************************/
bool WDASpellEffects::operator == (const WDASpellEffects &l )
/******************************************************************************/
{
    if( dwEffectStructureID != l.dwEffectStructureID )
	{
        return false;
    }

    if( vParams.size() != l.vParams.size() )
	{
        return false;
    }

    int i;
    for( i = 0; i < vParams.size(); i++ )
	{
        if( !( vParams[ i ] == l.vParams[ i ] ) )
		{
            return false;
        }
    }
    return true;
}

