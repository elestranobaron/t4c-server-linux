/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#if !defined(AFX_WDASPELLREQUIREMENTS_H__FFB498AA_D575_11D2_84B1_00E02922FA40__INCLUDED_)
#define AFX_WDASPELLREQUIREMENTS_H__FFB498AA_D575_11D2_84B1_00E02922FA40__INCLUDED_

#if _MSC_VER >= 1000
	#pragma once
#endif // _MSC_VER >= 1000

#include "WDATable.h"

class WDASpells;
/******************************************************************************/
class WDASpellRequirements : public WDATable  
/******************************************************************************/
{
public:
    WDASpellRequirements( vir::Logger &cLogger, vir::DEBUG_LEVEL dlMapHighDebug );
    friend WDASpells;
	virtual ~WDASpellRequirements();
    // Accessor
    DWORD GetRequiredSpellID( void );
    // Deep-copy operator
    void operator = ( const WDASpellRequirements &cParam )
	{
        dwRequiredSpellID = cParam.dwRequiredSpellID;
    }
    void Setup( DWORD spellID ){
        dwRequiredSpellID = spellID;
    }
    
private:
   
    // Creates from a wdaFile.
    virtual void CreateFrom( WDAFile &wdaFile, bool createReadOnly );
    DWORD dwRequiredSpellID;
};

#endif // !defined(AFX_WDASPELLREQUIREMENTS_H__FFB498AA_D575_11D2_84B1_00E02922FA40__INCLUDED_)
