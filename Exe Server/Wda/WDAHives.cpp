/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "WDAHives.h"
#include "../format.h"

using namespace std;

/******************************************************************************/
WDAHives::WDAHives( Logger &cLogger ) : WDATable( cLogger )
/******************************************************************************/
{
}
/******************************************************************************/
WDAHives::~WDAHives()
/******************************************************************************/
{
}
/******************************************************************************/
// Returns all the hives.
std::vector< WDAHives::HiveData > &WDAHives::GetHives( void )
/******************************************************************************/
{
    return vHives;
}
/******************************************************************************/
// Creates from a wdaFile.
void WDAHives::CreateFrom(WDAFile &wdaFile, bool createReadOnly)
/******************************************************************************/
{
    cOutput.Log(
        dlInfo,
        "\n--WDA--------------"
        "\nLoading hives."
        "\n"
    );
    
    // Read the quantity of hives.
    DWORD dwSize = 0xABCDEF;
    wdaFile.Read( dwSize );

    // Scroll through all hives.
    DWORD i;
    for( i = 0; i != dwSize; i++ )
	{
        HiveData cHive;

        wdaFile.Read( cHive.dwMinEmergeTime );
        wdaFile.Read( cHive.dwMaxEmergeTime );
        wdaFile.Read( cHive.dwMaxChildren );
        wdaFile.Read( cHive.dwEmergenceRange );
        wdaFile.Read( cHive.m_HiveName );

        // Write the hive data.
        TFormat cFormat;
        cOutput.Log(
            dlDebugHigh,
            cFormat(
                "\nFound hive. EmergeTime: min %u, max %u. Max Child %u, range %u.",
                cHive.dwMinEmergeTime,
                cHive.dwMaxEmergeTime,
                cHive.dwMaxChildren,
                cHive.dwEmergenceRange
            )
        );

        // Load the larvae
        {
            // Write the quantity of larvae.
            DWORD dwQ = 0xABCDEF;
            wdaFile.Read( dwQ );

            // Scroll through all larvae for this hive.
            DWORD j;
            for( j = 0; j != dwQ; j++ )
			{
                // Load that larva
                WDALarvae cLarva( cOutput, dlDebugHigh );
                cLarva.CreateFrom( wdaFile, createReadOnly );

                // Add this larva to the list of larvae in this hive.
                cHive.vLarvae.push_back( cLarva );
            }
        }
        // Load the hive's locations
        {
            // Read the quantity of locations.
            DWORD dwQ = 0xABCDEF;
            wdaFile.Read( dwQ );

            // Scroll through all locations.
            DWORD j;
            for( j = 0; j != dwQ; j++ )
			{
                WorldPos wlPos;
                // Directly read the world pos.
                wdaFile.Read( wlPos.X );
                wdaFile.Read( wlPos.Y );
                wdaFile.Read( wlPos.world );

                // Add this location
                cHive.vLocations.push_back( wlPos );
            }
        }
    
        cHive.m_ReadOnly = createReadOnly;
        // Add this hive to the list of hives.
        vHives.push_back( cHive );   
    }
}
