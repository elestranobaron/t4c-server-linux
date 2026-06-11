/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#if !defined(AFX_WDASPELLEFFECTPARAMETERS_H__FFB498A9_D575_11D2_84B1_00E02922FA40__INCLUDED_)
#define AFX_WDASPELLEFFECTPARAMETERS_H__FFB498A9_D575_11D2_84B1_00E02922FA40__INCLUDED_

#if _MSC_VER >= 1000
	#pragma once
#endif // _MSC_VER >= 1000

#include "WDATable.h"

class WDASpellEffects;
/******************************************************************************/
class WDASpellEffectParameters : public WDATable  
/******************************************************************************/
{
public:
    WDASpellEffectParameters( vir::Logger &cLogger, vir::DEBUG_LEVEL dlMapDebugHigh );
    friend WDASpellEffects;
	virtual ~WDASpellEffectParameters();
    // Accessors
    std::string &GetParamValue();
    DWORD        GetParamID();
    // Provide a deep-copy = operator.
    void operator =( const WDASpellEffectParameters &cParam )
	{
        dwParamID = cParam.dwParamID;
        csParamValue = cParam.csParamValue;
    };
    bool operator==( const WDASpellEffectParameters &l );
    void Setup( DWORD paramID, std::string paramValue )
	{
        dwParamID = paramID;
        csParamValue = paramValue;
    }
private:    
    // Creates from a wdaFile.
    virtual void CreateFrom( WDAFile &wdaFile, bool createReadOnly );
    std::string csParamValue;
    DWORD       dwParamID;
};

#endif // !defined(AFX_WDASPELLEFFECTPARAMETERS_H__FFB498A9_D575_11D2_84B1_00E02922FA40__INCLUDED_)
