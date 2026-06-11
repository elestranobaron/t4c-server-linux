/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "WDAWorlds.h"
#include "../Format.h"
#include <iterator>

using namespace std;

/******************************************************************************/
WDAWorlds::WDAWorlds( Logger &cTraceLogger ) : WDATable( cTraceLogger )
/******************************************************************************/
{
}
/******************************************************************************/
WDAWorlds::~WDAWorlds()
/******************************************************************************/
{
}
/******************************************************************************/
// Creates the worlds from a WDA file.
void WDAWorlds::CreateFrom(WDAFile &wdaFile, bool createReadOnly)
/******************************************************************************/
{
    cOutput.Log(
        dlInfo,
        "\n--WDA--------------"
        "\nLoading worlds."
        "\n"
    );

    // Read the quantity of worlds
    DWORD dwSize = 0xABCDEF;
    wdaFile.Read( dwSize );

    ///TRACE( "\ndwSize=%u.", dwSize );

    // Scroll through the list of worlds.
    DWORD i;
    for( i = 0; i != dwSize; i++ )
	{
        WorldData cWorldData;

        wdaFile.Read( cWorldData.wWorldID );
        wdaFile.Read( cWorldData.csWorldName );
        wdaFile.Read( cWorldData.wWorldSizeX );
        wdaFile.Read( cWorldData.wWorldSizeY );

        TFormat tFormat;
        cOutput.Log(
            dlDebug,
            tFormat( "Found world %u, %s [%ux%u].",
                cWorldData.wWorldID,    
                cWorldData.csWorldName.c_str(),                
                cWorldData.wWorldSizeX,
                cWorldData.wWorldSizeY
            )
        );
        
        cWorldData.lpbDataW    = new BYTE[ cWorldData.wWorldSizeX * cWorldData.wWorldSizeY / 2 ];
        cWorldData.lpbDataWOri = new BYTE[ cWorldData.wWorldSizeX * cWorldData.wWorldSizeY / 2 ];
        wdaFile.Read( cWorldData.lpbDataW, cWorldData.wWorldSizeX * cWorldData.wWorldSizeY / 2 );
        memcpy(cWorldData.lpbDataWOri,cWorldData.lpbDataW,(cWorldData.wWorldSizeX * cWorldData.wWorldSizeY / 2));
                    
        cWorldData.m_ReadOnly = createReadOnly;
        // Add world to list.
        vWorlds.push_back( cWorldData );
    }

}
/******************************************************************************/
// Returns the worlds.
vector< WDAWorlds::WorldData > &WDAWorlds::GetWorlds( void )
/******************************************************************************/
{
    return vWorlds;
}
