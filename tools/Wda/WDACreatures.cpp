/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "WDACreatures.h"

using namespace std;

/******************************************************************************/
WDACreatures::WDACreatures( vir::Logger &cOutputLogger ) : WDATable( cOutputLogger )
/******************************************************************************/
{
}
/******************************************************************************/
WDACreatures::~WDACreatures()
/******************************************************************************/
{
}
/******************************************************************************/
//  Returns a vector of creature data.
std::vector< WDACreatures::CreatureData > &WDACreatures::GetCreatures( void )
/******************************************************************************/
{
    return vCreatures;
}
/******************************************************************************/
// Creates from a wdaFile.
void WDACreatures::CreateFrom(WDAFile &wdaFile, bool createReadOnly)
/******************************************************************************/
{
    cOutput.Log(
        dlInfo,
        "\n--WDA--------------"
        "\nLoading objects."
        "\n"
    );

    // Get the quantity of objects
    DWORD dwSize;
    wdaFile.Read( dwSize );
       
    // Scroll through the list of objects.
    DWORD i;
    for( i = 0; i != dwSize; i++ )
	{
        CreatureData cCreature;

        wdaFile.Read( cCreature.dwBindedID );
        wdaFile.Read( cCreature.csID );
        wdaFile.Read( cCreature.csName );
        wdaFile.Read( cCreature.dwSTR );
        wdaFile.Read( cCreature.dwEND );
        wdaFile.Read( cCreature.dwAGI );
        wdaFile.Read( cCreature.dwINT );
        wdaFile.Read( cCreature.dwWIL );
        wdaFile.Read( cCreature.dwWIS );        
        wdaFile.Read( cCreature.dwLCK );
        wdaFile.Read( cCreature.dwAirResist );
        wdaFile.Read( cCreature.dwEarthResist );
        wdaFile.Read( cCreature.dwWaterResist );
        wdaFile.Read( cCreature.dwFireResist );
        wdaFile.Read( cCreature.dwDarkResist );
        wdaFile.Read( cCreature.dwLightResist );
        wdaFile.Read( cCreature.dwAirPower );
        wdaFile.Read( cCreature.dwEarthPower );
        wdaFile.Read( cCreature.dwWaterPower );
        wdaFile.Read( cCreature.dwFirePower );
        wdaFile.Read( cCreature.dwDarkPower );
        wdaFile.Read( cCreature.dwLightPower );
        wdaFile.Read( cCreature.dwLevel );
        wdaFile.Read( cCreature.dwHP );
        wdaFile.Read( cCreature.dwDodgeSkill );
        wdaFile.Read( cCreature.dblAC );
        wdaFile.Read( cCreature.dwAppearance );
        wdaFile.Read( cCreature.dwDressBody );
        wdaFile.Read( cCreature.dwDressFeet );
        wdaFile.Read( cCreature.dwDressGloves );
        wdaFile.Read( cCreature.dwDressHelm );
        wdaFile.Read( cCreature.dwDressLegs );
        wdaFile.Read( cCreature.dwDressWeapon );
        wdaFile.Read( cCreature.dwDressShield );
        wdaFile.Read( cCreature.dwDressCape );
        wdaFile.Read( cCreature.dwAggressivness );
        wdaFile.Read( cCreature.dwClanID );
        wdaFile.Read( cCreature.dwSpeed );
        wdaFile.Read( cCreature.dblXPperHit );
        wdaFile.Read( cCreature.dblXPperDeath );
        wdaFile.Read( cCreature.dwMinGiveGold );
        wdaFile.Read( cCreature.dwMaxGiveGold );
        wdaFile.Read( cCreature.boCanAttackWDA );
        wdaFile.Read( cCreature.boChangeTargetAAWDA);
        wdaFile.Read( cCreature.iFriendlyIDWDA);
        

        cCreature.csName = GetBareString( cCreature.csName );

        // Load attacks.
        {
            DWORD dwSize;
            wdaFile.Read( dwSize );            
            DWORD i;
            for( i = 0; i != dwSize; i++ )
			{
                CreatureAttack cAttack;

                wdaFile.Read( cAttack.csDmgRoll );
                wdaFile.Read( cAttack.dwAttackSkill );
                wdaFile.Read( cAttack.dwAttackPercentage );
                wdaFile.Read( cAttack.dwAttackSpell );
                wdaFile.Read( cAttack.dwAttackMinRange );
                wdaFile.Read( cAttack.dwAttackMaxRange );

                cCreature.vAttacks.push_back( cAttack );
            }
        }
        // Save death flags.
        {
            DWORD dwSize;
            wdaFile.Read( dwSize );            
            DWORD i;
            for( i = 0; i != dwSize; i++ )
			{
                CreatureDeathFlag cFlag;

                wdaFile.Read( cFlag.dwFlag );
                wdaFile.Read( cFlag.dwFlagValue );
                wdaFile.Read( cFlag.boIncrement );

                cCreature.vDeathFlags.push_back( cFlag );
            }
        }
        // Save items
        {
            DWORD dwSize;
            wdaFile.Read( dwSize );            
            DWORD i;
            for( i = 0; i != dwSize; i++ )
			{
                CreatureDeathItem cItem;
                
                wdaFile.Read( cItem.dwItemID );
                wdaFile.Read( cItem.dblDropPercentage );

                cCreature.vItems.push_back( cItem );
            }
        }

        cCreature.m_ReadOnly = createReadOnly;

        vCreatures.push_back( cCreature );
    }
}
/******************************************************************************/
WDACreatures::CreatureData *WDACreatures::GetReadOnlyCreature( std::string id )
/******************************************************************************/
{
    int i;
    for( i = 0; i < vCreatures.size(); i++ )
	{
        if( vCreatures[ i ].m_ReadOnly && _stricmp( vCreatures[ i ].csID.c_str(), id.c_str() ) == 0 )
		{
            return &vCreatures[ i ];
        }
    }
    return NULL;
}
/******************************************************************************/
WDACreatures::CreatureData *WDACreatures::GetWritableCreature( std::string id )
/******************************************************************************/
{
    int i;
    for( i = 0; i < vCreatures.size(); i++ )
	{
        if( !vCreatures[ i ].m_ReadOnly && _stricmp( vCreatures[ i ].csID.c_str(), id.c_str() ) == 0 )
		{
            return &vCreatures[ i ];
        }
    }
    return NULL;
}

