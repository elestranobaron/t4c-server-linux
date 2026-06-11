/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "WDAClans.h"
#include "../Format.h"

using namespace std;

/******************************************************************************/
WDAClans::WDAClans( Logger &cTraceLogger ) : WDATable( cTraceLogger )
/******************************************************************************/
{
}
/******************************************************************************/
WDAClans::~WDAClans()
/******************************************************************************/
{
}
/******************************************************************************/
// Creates from a wdaFile.
void WDAClans::CreateFrom(WDAFile &wdaFile, bool createReadOnly)
/******************************************************************************/
{
    DWORD dwSize = 0xABCDEF;

    cOutput.Log(
        dlInfo,
        "\n--WDA--------------"
        "\nLoading monster clans."
        "\n"
    );
    
    // Read the highest clan
    wdaFile.Read( dwHighestClan );
    
    // Read the quantity of clans.
    wdaFile.Read( dwSize );

    DWORD i;
    for( i = 0; i < dwSize; i++ )
	{
        ClanRelation cClanRelation;

        wdaFile.Read( cClanRelation.wFirstClanID );
        wdaFile.Read( cClanRelation.wSecondClanID );
        wdaFile.Read( cClanRelation.sClanRelation );

        TFormat cFormat;
        cOutput.Log(
            dlDebug,
            cFormat( "\nFound clan relation between clan %u and %u (%d aggressivity)",
                cClanRelation.wFirstClanID,
                cClanRelation.wSecondClanID,
                cClanRelation.sClanRelation
            )
        );

        cClanRelation.m_ReadOnly = createReadOnly;

        // Add clan to clan relations.
        vClanRelations.push_back( cClanRelation );
    }

    wdaFile.Read( dwSize );
    for( i = 0; i < dwSize; i++ )
	{
        Clan clan;

        wdaFile.Read( clan.m_ID );
        wdaFile.Read( clan.m_Name );

        clan.m_ReadOnly = createReadOnly;

        vClans.push_back( clan );
    }
}
/******************************************************************************/
// Return: std::vector< ClanRelation >, The clan relations.
std::vector< WDAClans::ClanRelation > &WDAClans::GetClanRelations( void )
/******************************************************************************/
{
    return vClanRelations;
}
/******************************************************************************/
std::vector< WDAClans::Clan > &WDAClans::GetClans()
/******************************************************************************/
{
    return vClans;
}
/******************************************************************************/
// Return: DWORD, the highest possible clan ID.
DWORD WDAClans::GetHighestClan( void )
/******************************************************************************/
{
    return dwHighestClan;
}
/******************************************************************************/
void WDAClans::ResetHighestClan()
/******************************************************************************/
{
    dwHighestClan = 0;
    int i;
    for( i = 0; i < vClans.size(); i++ )
	{
        if( vClans[ i ].m_ID > dwHighestClan )
		{
            dwHighestClan = vClans[ i ].m_ID;
        }
    }
}