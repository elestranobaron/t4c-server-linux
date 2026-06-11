/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#if !defined(AFX_WDASPELLEFFECTS_H__FFB498A8_D575_11D2_84B1_00E02922FA40__INCLUDED_)
#define AFX_WDASPELLEFFECTS_H__FFB498A8_D575_11D2_84B1_00E02922FA40__INCLUDED_

#if _MSC_VER >= 1000
	#pragma once
#endif // _MSC_VER >= 1000

#include "WDATable.h"
#include "WDASpellEffectParameters.h"

class WDASpells;
/******************************************************************************/
class WDASpellEffects : public WDATable  
/******************************************************************************/
{
public:
    // WDASpells CONTAINS-MANY WDASpellEffects, make it a friend.
    friend WDASpells;
    WDASpellEffects( vir::Logger &cLogger, vir::DEBUG_LEVEL dlMapHighDebug );
	virtual ~WDASpellEffects();
    // Accessors
    DWORD GetEffectStructureID( void );
    std::vector< WDASpellEffectParameters > &GetParams( void );
    // Provide a deep-copy operator.
    void operator = ( const WDASpellEffects &cParam )
	{
        dwEffectStructureID = cParam.dwEffectStructureID;
        vParams = cParam.vParams;
    }
    void Setup( DWORD structureID )
	{
        dwEffectStructureID = structureID;        
    }
    bool operator == (const WDASpellEffects &l );

private:
    // Creates from a wdaFile.
    virtual void CreateFrom( WDAFile &wdaFile, bool createReadOnly );
    // Data
    DWORD                                   dwEffectStructureID;
    std::vector< WDASpellEffectParameters > vParams;
};

#endif // !defined(AFX_WDASPELLEFFECTS_H__FFB498A8_D575_11D2_84B1_00E02922FA40__INCLUDED_)
