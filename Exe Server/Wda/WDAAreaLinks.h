/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#ifndef AFX_AREALINKS_H__8D88CCE6_D321_11D2_84AD_00E02922FA40__INCLUDED_
#define AFX_AREALINKS_H__8D88CCE6_D321_11D2_84AD_00E02922FA40__INCLUDED_

#if _MSC_VER >= 1000
	#pragma once
#endif // _MSC_VER >= 1000

#include "WDATable.h"

/******************************************************************************/
class WDAAreaLinks : public WDATable
/******************************************************************************/
{
public:
	// Construction.
	WDAAreaLinks( vir::Logger &cTraceLogger );
	virtual ~WDAAreaLinks();

    // Class structures.
    struct WorldPos
	{
        WorldPos() : X(0),Y(0),world(0)
        {}

        int X;
        int Y;
        int world;
    };

    struct AreaLinkData
	{
        AreaLinkData() : m_ReadOnly( false )
        {}

        bool     m_ReadOnly;
        WorldPos wlSourcePos;
        WorldPos wlTargetPos;
    };

    // Creates from a wdaFile.
    virtual void CreateFrom( WDAFile &wdaFile, bool createReadOnly );
    // Returns the area links.
    virtual std::vector< AreaLinkData > &GetAreaLinks( void );

private:
    std::vector< AreaLinkData > vAreaLinks;

};

#endif // !defined(AFX_WDAAREALINKS_H__0F034DCD_D33D_11D2_84AD_00E02922FA40__INCLUDED_)
