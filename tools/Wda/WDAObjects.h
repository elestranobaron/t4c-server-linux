/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#if !defined(AFX_WDAOBJECTS_H__FECB9F3C_D648_11D2_84B2_00E02922FA40__INCLUDED_)
#define AFX_WDAOBJECTS_H__FECB9F3C_D648_11D2_84B2_00E02922FA40__INCLUDED_

#if _MSC_VER >= 1000
	#pragma once
#endif // _MSC_VER >= 1000

#include "WDATable.h"
#include "WDAObjectsContainerGroups.h"
#include "WDAObjectsAttrBoosts.h"
#include "WDAObjectsSpells.h"
#include "WDAObjectsWorldObjects.h"

/******************************************************************************/
class WDAObjects : public WDATable  
/******************************************************************************/
{
public:
    WDAObjects( vir::Logger &cLogger );
	virtual ~WDAObjects();

    enum DropFlags
	{
        eCannotDrop = 0x01,
        eCannotGive = 0x02,
        eCannotRob = 0x04,
        eCannotJunk = 0x08,
        eCannotGet = 0x10,
        eCannotHide = 0x20,
        eJunkAtSeraph = 0x40,
		eCannotMule = 0x80
    };
    
    // Main structure
    struct ObjectData
	{
        ObjectData() : 
            m_ReadOnly(false),
            dwBindedID(0),
            dwStructureID(0),
            dwAppearance(0),
            dwSellType(0),
            dwEquipPos(0),
            dwBuyFlagID(0),
            dwSellPrice(0),
            dwSize(0),
            dblArmor_AC(0),
            dblParadePC(0),
            dwArmor_DodgeMalus(0),
            dwArmor_MinEnd(0),
            dwWeapon_Attack(0),
            dwWeapon_Str(0),
            dwWeapon_Agi(0),
            dwLockDifficulty(0),
            dwContainer_Gold(0),
            dwContainer_GlobalRespawn(0),
            dwContainer_UserRespawn(0),
            dwRadiance(0),
            lCharges(0),
            dwMinInt(0),
            dwMinWis(0),
            dwIntlID(0),     
            dwDropFlags(0),
            boUnique(false),
            boCanSummon(true),
            boWeapon_Ranged(false),
            boWeapon_RangedInfiniteAmmo(true)
        {}

        // Vector of container groups
        std::vector< WDAObjectsContainerGroups > vContainers;
        std::vector< WDAObjectsAttrBoosts >      vAttrBoosts;
        std::vector< WDAObjectsSpells >          vSpells;

        bool        m_ReadOnly;
        std::string csID;
        DWORD       dwBindedID;
        DWORD       dwStructureID;
        std::string csName;
        DWORD       dwAppearance;
        DWORD       dwSellType;
        DWORD       dwEquipPos;
        DWORD       dwBuyFlagID;
        DWORD       dwSellPrice;
        std::string strSellPrice;
        DWORD       dwSize;
        double      dblArmor_AC;
        double      dblParadePC;
        DWORD       dwArmor_DodgeMalus;
        DWORD       dwArmor_MinEnd;
        std::string csWeapon_DmgRoll;
        DWORD       dwWeapon_Attack;
        DWORD       dwWeapon_Str;
        DWORD       dwWeapon_Agi;
        std::string csLock_KeyID;
        DWORD       dwLockDifficulty;
        std::string csBook_Text;
        DWORD       dwContainer_Gold;
        DWORD       dwContainer_GlobalRespawn;
        DWORD       dwContainer_UserRespawn;
        std::string csWeapon_Exhaust;
        DWORD       dwRadiance;
        signed long lCharges;
        DWORD       dwMinInt;
        DWORD       dwMinWis;
        DWORD       dwIntlID;        
        DWORD       dwDropFlags;
        bool        boUnique;
        std::string csGmItemLocation;
        bool        boCanSummon;
        bool        boWeapon_Ranged;
        bool        boWeapon_RangedInfiniteAmmo;
    };

    // Accessors
    ObjectData *GetWritableObject( std::string objID );
    ObjectData *GetReadOnlyObject( std::string objID );
    ObjectData *GetWritableObject( DWORD objID );
    ObjectData *GetReadOnlyObject( DWORD objID );
    
    std::vector< ObjectData > &GetObjects( void );
    std::vector< WDAObjectsWorldObjects > &GetLocations( void );

    // Creates from a wdaFile.
    virtual void CreateFrom( WDAFile &wdaFile, bool createReadOnly );
    DWORD GetNextHighestBoostID();

private:
    DWORD m_NextHighestBoostID;
    void ComputeHighestBoostID();
    // Lists all possible objects.
    std::vector< ObjectData > vObjects;
    // Lists all possible item locations.
    std::vector< WDAObjectsWorldObjects > vLocations;
};

#endif // !defined(AFX_WDAOBJECTS_H__FECB9F3C_D648_11D2_84B2_00E02922FA40__INCLUDED_)
