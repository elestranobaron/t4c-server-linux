/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#if !defined(AFX_WDAOBJECTSATTRBOOSTS_H__FECB9F3B_D648_11D2_84B2_00E02922FA40__INCLUDED_)
#define AFX_WDAOBJECTSATTRBOOSTS_H__FECB9F3B_D648_11D2_84B2_00E02922FA40__INCLUDED_

#if _MSC_VER >= 1000
	#pragma once
#endif // _MSC_VER >= 1000

#include "WDATable.h"

class WDAObjects;
/******************************************************************************/
class WDAObjectsAttrBoosts : public WDATable  
/******************************************************************************/
{
public:
    // Only usable by WDAObjects.
    friend WDAObjects;
	
    WDAObjectsAttrBoosts( vir::Logger &cLogger, vir::DEBUG_LEVEL dlDebugHigh );
	virtual ~WDAObjectsAttrBoosts();

    // Accessors
    DWORD       GetStat( void );
    std::string GetBoost( void );
    DWORD       GetMinInt( void );
    DWORD       GetMinWis( void );
    DWORD       GetID( void );

    void Setup( DWORD stat, std::string boost, DWORD minInt, DWORD minWis, DWORD id )
	{
        dwStat = stat;
        bsBoost = boost;
        dwMinInt = minInt;
        dwMinWis = minWis;
        dwID = id;
    }    

    // Deep-copy operator
    void operator = ( const WDAObjectsAttrBoosts &cParam )
	{
        dwStat = cParam.dwStat;
        bsBoost = cParam.bsBoost;
        dwMinInt = cParam.dwMinInt;
        dwMinWis = cParam.dwMinWis;
        dwID     = cParam.dwID;
    };

private:    
    
    // Creates from a wdaFile.
    virtual void CreateFrom( WDAFile &wdaFile, bool createReadOnly );
    // Data.
    DWORD       dwID;
    DWORD       dwStat;
    std::string bsBoost;
    DWORD       dwMinInt;
    DWORD       dwMinWis;
};

#endif // !defined(AFX_WDAOBJECTSATTRBOOSTS_H__FECB9F3B_D648_11D2_84B2_00E02922FA40__INCLUDED_)
