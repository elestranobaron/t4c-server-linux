/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "WDASpells.h"
#include "../Format.h"

using namespace std;

/******************************************************************************/
WDASpells::WDASpells( Logger &cLogger ) : WDATable( cLogger )
{
}
/******************************************************************************/
WDASpells::~WDASpells()
{
}
/******************************************************************************/
// Returns all the spells.
std::vector< WDASpells::SpellData > &WDASpells::GetSpells( void )
/******************************************************************************/
{
    return vSpells;
}
/******************************************************************************/
// Creates from a wdaFile.
void WDASpells::CreateFrom(WDAFile &wdaFile, bool createReadOnly)
/******************************************************************************/
{
    DWORD dwSize = 0xABCDEF;

    cOutput.Log(
        dlInfo,
        "\n--WDA--------------"
        "\nLoading spells."
        "\n"
    );    

    // Read the quantity of spells.
    wdaFile.Read( dwSize );

    // Load all saved spells.
    DWORD i;
    for( i = 0; i < dwSize; i++ )
	{
        SpellData cSpell;
        
        // Write the spell data.
        wdaFile.Read( cSpell.dwSpellID );
        wdaFile.Read( cSpell.csName );
        wdaFile.Read( cSpell.csMentalExhaust );
        wdaFile.Read( cSpell.csMoveExhaust );
        wdaFile.Read( cSpell.csAttackExhaust );
        wdaFile.Read( cSpell.bsDuration );
        wdaFile.Read( cSpell.bsTimerFrequency );
        wdaFile.Read( cSpell.dwElement );
        wdaFile.Read( cSpell.bsManaCost );
        wdaFile.Read( cSpell.dwAreaRange );
        wdaFile.Read( cSpell.dwVisualEffect );
        wdaFile.Read( cSpell.dwRangeVisualEffect );
        wdaFile.Read( cSpell.dwTargetType );
        wdaFile.Read( cSpell.dwAttackType );
        wdaFile.Read( cSpell.dwMinWis );
        wdaFile.Read( cSpell.dwMinInt );
        wdaFile.Read( cSpell.dwMinLevel );
        wdaFile.Read( cSpell.dwMinStr );
        wdaFile.Read( cSpell.dwMinEnd );
        wdaFile.Read( cSpell.dwMinAgi );
        wdaFile.Read( cSpell.dwMinAttack );
        wdaFile.Read( cSpell.dwMinArchery );
        wdaFile.Read( cSpell.dwMinDodge );
        wdaFile.Read( cSpell.dwMinCS );
        wdaFile.Read( cSpell.boLineOfSight );
        wdaFile.Read( cSpell.boSkillExclusion );
        wdaFile.Read( cSpell.boPVPcheck );
        wdaFile.Read( cSpell.boAttackSpell );
        wdaFile.Read( cSpell.dwIcon );
        wdaFile.Read( cSpell.csSuccessPercentage );
        wdaFile.Read( cSpell.csDesc );

        cSpell.csName = GetBareString( cSpell.csName );
        cSpell.csDesc = GetBareString( cSpell.csDesc );

        TFormat cFormat;
        cOutput.Log(
            dlDebug,
            cFormat(
                "\nFound spell %s (%u)",
                cSpell.csName.c_str(),
                cSpell.dwSpellID
            )
        );

        cOutput.Log(
            dlDebugHigh,
            cFormat(
                "\n  Mental exhaust: %s."
                "\n  MoveExhaust: %s"
                "\n  AttackExhaust: %s"
                "\n  Duration: %s"
                "\n  TimerFrequency: %s"
                "\n  Element: %u"
                "\n  ManaCost: %s"
                "\n  AreaRange: %u"
                "\n  VisualEffect: %u"
                "\n  TargetType: %u"
                "\n  AttackType: %u"
                "\n  MinWis: %u"
                "\n  MinInt: %u"
                "\n  MinLevel: %u"
                "\n  MinStr: %u"
                "\n  MinEnd: %u"
                "\n  MinAgi: %u"
                "\n  MinAttack: %u"
                "\n  MinArchery: %u"
                "\n  MinDodge: %u"
                "\n  MinCS: %u"
                "\n  LineOfSight: %s"
                "\n  SkillExclusion: %s"
                "\n  PVPcheck: %s",
                cSpell.csMentalExhaust.c_str(),
                cSpell.csMoveExhaust.c_str(),
                cSpell.csAttackExhaust.c_str(),
                cSpell.bsDuration.c_str(),
                cSpell.bsTimerFrequency.c_str(),
                cSpell.dwElement,
                cSpell.bsManaCost.c_str(),
                cSpell.dwAreaRange,
                cSpell.dwVisualEffect,
                cSpell.dwTargetType,
                cSpell.dwAttackType,
                cSpell.dwMinWis,
                cSpell.dwMinInt,
                cSpell.dwMinLevel,
                cSpell.dwMinStr,
                cSpell.dwMinEnd,
                cSpell.dwMinAgi,
                cSpell.dwMinAttack,
                cSpell.dwMinArchery,
                cSpell.dwMinDodge,
                cSpell.dwMinCS,
                cSpell.boLineOfSight ? "true" : "false",
                cSpell.boSkillExclusion ? "true" : "false",
                cSpell.boPVPcheck ? "true" : "false"
            )
        );
        
        // Spell effects.
        {
            // Read the quantity of spell effects.
            DWORD dwQ;

            wdaFile.Read( dwQ );
            DWORD j;
            for( j = 0; j < dwQ; j++ )
			{
                // Create the spell effects.
                WDASpellEffects cSpellEffects( cOutput, dlDebugHigh );
                cSpellEffects.CreateFrom( wdaFile, createReadOnly );
                // Add this spell effect to the list of spell effects.
                cSpell.vSpellEffects.push_back( cSpellEffects );
            }
        }
        // Spell requirements.
        {
            // Read the quantity of spell reqs.
            DWORD dwQ;

            wdaFile.Read( dwQ );
            DWORD j;
            for( j = 0; j < dwQ; j++ )
			{
                // Create the spell req.
                WDASpellRequirements cSpellReq( cOutput, dlDebugHigh );
                cSpellReq.CreateFrom( wdaFile, createReadOnly );
                // Add this spell req to the list of spell reqs.
                cSpell.vSpellRequirements.push_back( cSpellReq );
            }
        }

        cSpell.m_ReadOnly = createReadOnly;
        // Add the spell to the list of spells.
        vSpells.push_back( cSpell );
    }
}
/******************************************************************************/
WDASpells::SpellData *WDASpells::GetSpell( DWORD spellID )
/******************************************************************************/
{
    int i;
    for( i = 0; i < vSpells.size(); i++ ){
        if( vSpells[ i ].dwSpellID == spellID ){
            return &vSpells[ i ];
        }
    }
    return NULL;
}
/******************************************************************************/
DWORD WDASpells::GetNextSpellID()
/******************************************************************************/
{
    DWORD highestSpellID = 10000;

    int i;
    for( i = 0; i < vSpells.size(); i++ ){
        if( vSpells[ i ].dwSpellID >= highestSpellID ){
            highestSpellID = vSpells[ i ].dwSpellID + 1;            
        }
    }
    return highestSpellID;
}