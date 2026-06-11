/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "WDAObjects.h"
#include "../Format.h"

using namespace std;

/******************************************************************************/
WDAObjects::WDAObjects( Logger &cLogger ) : 
    WDATable( cLogger ), 
    m_NextHighestBoostID( 0 )
/******************************************************************************/
{
}
/******************************************************************************/
WDAObjects::~WDAObjects()
/******************************************************************************/
{
}
/******************************************************************************/
// Returns the list of all items.
std::vector< WDAObjects::ObjectData > &WDAObjects::GetObjects( void )
/******************************************************************************/
{
    return vObjects;
}
/******************************************************************************/
// Returns the list of all items scattered on the ground at load time.
std::vector< WDAObjectsWorldObjects > &WDAObjects::GetLocations( void )
/******************************************************************************/
{
    return vLocations;
}
/******************************************************************************/
// Creates from a wdaFile.
void WDAObjects::CreateFrom(WDAFile &wdaFile, bool createReadOnly)
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
        // Create a new object.
        ObjectData cObject;
                
        // Write the object's data
        wdaFile.Read( cObject.csID );
        wdaFile.Read( cObject.dwBindedID );
        wdaFile.Read( cObject.dwStructureID );
        wdaFile.Read( cObject.csName );
        wdaFile.Read( cObject.dwAppearance );
        wdaFile.Read( cObject.dwSellType );
        wdaFile.Read( cObject.dwEquipPos );
        wdaFile.Read( cObject.dwBuyFlagID );
        wdaFile.Read( cObject.dwSellPrice );
        wdaFile.Read( cObject.strSellPrice );
        wdaFile.Read( cObject.dwSize );
        wdaFile.Read( cObject.dblArmor_AC );
        wdaFile.Read( cObject.dblParadePC );
        wdaFile.Read( cObject.dwArmor_DodgeMalus );
        wdaFile.Read( cObject.dwArmor_MinEnd );
        wdaFile.Read( cObject.csWeapon_DmgRoll );
        wdaFile.Read( cObject.dwWeapon_Attack );
        wdaFile.Read( cObject.dwWeapon_Str );
        wdaFile.Read( cObject.dwWeapon_Agi );
        wdaFile.Read( cObject.csLock_KeyID );
        wdaFile.Read( cObject.dwLockDifficulty );
        wdaFile.Read( cObject.csBook_Text );
        wdaFile.Read( cObject.dwContainer_Gold );
        wdaFile.Read( cObject.dwContainer_GlobalRespawn );
        wdaFile.Read( cObject.dwContainer_UserRespawn );
        wdaFile.Read( cObject.csWeapon_Exhaust );
        wdaFile.Read( cObject.dwRadiance );
        wdaFile.Read( cObject.lCharges );
        wdaFile.Read( cObject.dwMinInt );
        wdaFile.Read( cObject.dwMinWis );
        wdaFile.Read( cObject.dwIntlID );
        wdaFile.Read( cObject.dwDropFlags );
        wdaFile.Read( cObject.boUnique );
        wdaFile.Read( cObject.csGmItemLocation );
        wdaFile.Read( cObject.boCanSummon );
        wdaFile.Read( cObject.boWeapon_Ranged );
        wdaFile.Read( cObject.boWeapon_RangedInfiniteAmmo );

        cObject.csName = GetBareString( cObject.csName );
        cObject.csBook_Text = GetBareString( cObject.csBook_Text );

        TFormat cFormat;
        cOutput.Log(
            dlDebug,
            cFormat(
                "\nFound item %s.",
                cObject.csID.c_str()
            )
        );
        cOutput.Log(
            dlDebugHigh,
            cFormat(
                "\n BindedID: %u"
                "\n StructureID: %u"
                "\n Name: %s"
                "\n Appearance: %u"
                "\n SellType: %u"
                "\n EquipPos: %u"
                "\n BuyFlagID: %u"
                "\n SellPrice: %u"
                "\n SellPriceFormula: %s"
                "\n Size: %u"
                "\n Armor_AC: %f"
                "\n ParadePC: %f"
                "\n Armor_DodgeMalus: %u"
                "\n Armor_MinEnd: %u"
                "\n Weapon_DmgRoll: %s"
                "\n Weapon_Attack: %u"
                "\n Weapon_Str: %u"
                "\n Weapon_Agi: %u"
                "\n Lock_KeyID: %s"
                "\n LockDifficulty: %u"
                "\n Book_Text: %s"
                "\n Container_Gold: %u"
                "\n Container_GlobalRespawn: %u"
                "\n Container_UserRespawn: %u"
                "\n Exhaust: %s"
                "\n Radiance: %u"
                "\n Charges: %d"
                "\n MinInt: %u"
                "\n MinWis: %u"
                "\n IntlID: %u"
                "\n DropFlags: %u",
                cObject.dwBindedID,
                cObject.dwStructureID,
                cObject.csName.c_str(),
                cObject.dwAppearance,
                cObject.dwSellType,
                cObject.dwEquipPos,
                cObject.dwBuyFlagID,
                cObject.dwSellPrice,
                cObject.strSellPrice.c_str(),
                cObject.dwSize,
                cObject.dblArmor_AC,
                cObject.dblParadePC,
                cObject.dwArmor_DodgeMalus,
                cObject.dwArmor_MinEnd,
                cObject.csWeapon_DmgRoll.c_str(),
                cObject.dwWeapon_Attack,
                cObject.dwWeapon_Str,
                cObject.dwWeapon_Agi,
                cObject.csLock_KeyID.c_str(),
                cObject.dwLockDifficulty,
                cObject.csBook_Text.c_str(),
                cObject.dwContainer_Gold,
                cObject.dwContainer_GlobalRespawn,
                cObject.dwContainer_UserRespawn,
                cObject.csWeapon_Exhaust.c_str(),
                cObject.dwRadiance,
                cObject.lCharges,
                cObject.dwMinInt,
                cObject.dwMinWis,
                cObject.dwIntlID,
                cObject.dwDropFlags
            )
        );

        // Load all containers
        {
            // Load the quantity of containers in this item
            DWORD dwQ;
            wdaFile.Read( dwQ );

            // Scroll through all containers.
            DWORD j;
            for( j = 0; j != dwQ; j++ )
			{
                // Load the container
                WDAObjectsContainerGroups cContainer( cOutput, dlDebugHigh );
                cContainer.CreateFrom( wdaFile, createReadOnly );

                // Add the container.
                cObject.vContainers.push_back( cContainer );
            }
        }
        // Load all attribute boosts.
        {
            // Load the quantity of boosts in this item
            DWORD dwQ;
            wdaFile.Read( dwQ );

            // Scroll through all attributes.
            DWORD j;
            for( j = 0; j != dwQ; j++ )
			{
                // Load the boost
                WDAObjectsAttrBoosts cBoost( cOutput, dlDebugHigh );
                cBoost.CreateFrom( wdaFile, createReadOnly );

                // Add boost to object
                cObject.vAttrBoosts.push_back( cBoost );
            }
        }
        // Load all spells.
        {
            // Load the quantity of spells in this item
            DWORD dwQ;
            wdaFile.Read( dwQ );

            // Scroll through all spells.
            DWORD j;
            for( j = 0; j != dwQ; j++ )
			{
                // Load the spell
                WDAObjectsSpells cSpell( cOutput, dlDebugHigh );
                cSpell.CreateFrom( wdaFile, createReadOnly );

                // Add spell to object
                cObject.vSpells.push_back( cSpell );
            }
        }

        cObject.m_ReadOnly = createReadOnly;

        // Add the object to the list of objects.
        vObjects.push_back( cObject );
    }// for( vObjects ...

    // Load the world objects
    {
        DWORD dwQ;
        wdaFile.Read( dwQ );

        DWORD j;
        for( j = 0; j != dwQ; j++ )
		{
            WDAObjectsWorldObjects cPos( cOutput, dlDebugHigh );
            cPos.CreateFrom( wdaFile, createReadOnly );
                    
            // Add the location.
            vLocations.push_back( cPos );
        }
    }

    ComputeHighestBoostID();
}
/******************************************************************************/
void WDAObjects::ComputeHighestBoostID()
/******************************************************************************/
{
	int i;
	for( i = 0; i < vObjects.size(); i++ )
	{
		ObjectData &obj = vObjects[ i ];
        int k;
        for( k = 0; k < obj.vAttrBoosts.size(); k++ )
		{
            WDAObjectsAttrBoosts &boost = obj.vAttrBoosts[ k ];
            if( boost.GetID() > m_NextHighestBoostID )
			{
                m_NextHighestBoostID = boost.GetID() + 1;
            }
        }
    }
}
/******************************************************************************/
DWORD WDAObjects::GetNextHighestBoostID()
/******************************************************************************/
{
    return m_NextHighestBoostID++;
}
/******************************************************************************/
WDAObjects::ObjectData *WDAObjects::GetWritableObject( std::string objID )
/******************************************************************************/
{
    int i;
    for( i = 0; i < vObjects.size(); i++ )
	{
        if( !vObjects[ i ].m_ReadOnly && _stricmp( vObjects[ i ].csID.c_str(), objID.c_str() ) == 0 )
		{
            return &vObjects[ i ];
        }
    }
    return NULL;
}
/******************************************************************************/
WDAObjects::ObjectData *WDAObjects::GetReadOnlyObject( std::string objID )
/******************************************************************************/
{
    int i;
    for( i = 0; i < vObjects.size(); i++ )
	{
        if( vObjects[ i ].m_ReadOnly && _stricmp( vObjects[ i ].csID.c_str(), objID.c_str() ) == 0 )
		{
            return &vObjects[ i ];
        }
    }
    return NULL;
}
/******************************************************************************/
WDAObjects::ObjectData *WDAObjects::GetWritableObject( DWORD objID )
/******************************************************************************/
{
    int i;
    for( i = 0; i < vObjects.size(); i++ ){
        if( !vObjects[ i ].m_ReadOnly && vObjects[ i ].dwBindedID == objID ){
            return &vObjects[ i ];
        }
    }
    return NULL;
}
/******************************************************************************/
WDAObjects::ObjectData *WDAObjects::GetReadOnlyObject( DWORD objID )
/******************************************************************************/
{
    int i;
    for( i = 0; i < vObjects.size(); i++ )
	{
        if( vObjects[ i ].m_ReadOnly && vObjects[ i ].dwBindedID == objID )
		{
            return &vObjects[ i ];
        }
    }
    return NULL;
}


