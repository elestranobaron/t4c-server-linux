/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#if !defined(AFX_WDAOBJECTSWORLDOBJECTS_H__FECB9F3D_D648_11D2_84B2_00E02922FA40__INCLUDED_)
#define AFX_WDAOBJECTSWORLDOBJECTS_H__FECB9F3D_D648_11D2_84B2_00E02922FA40__INCLUDED_

#if _MSC_VER >= 1000
	#pragma once
#endif // _MSC_VER >= 1000

#include "WDATable.h"
#include <string>

class WDAObjects;
/******************************************************************************/
class WDAObjectsWorldObjects : public WDATable  
/******************************************************************************/
{
public:
    WDAObjectsWorldObjects( vir::Logger &cLogger, vir::DEBUG_LEVEL dlHigh );
    // Only WDAObjects can access this class.
    friend WDAObjects;
	virtual ~WDAObjectsWorldObjects();
    struct WorldPos
	{
        WorldPos() : X(0),Y(0),world(0)
        {}

        int X, Y, world;
    };
    // Accessors
    WorldPos           GetPos( void );
    const std::string &GetItemID( void );
    // Deep-copy operators
    void operator = ( const WDAObjectsWorldObjects &cParam )
	{
        wlPos = cParam.wlPos;
        csItemID = cParam.csItemID;
    }
    void Setup( std::string itemID, int X, int Y, int world )
	{
        wlPos.X = X;
        wlPos.Y = Y;
        wlPos.world = world;
        csItemID = itemID;
    }
    bool operator == (const WDAObjectsWorldObjects &r ) const
	{
        return wlPos.X     == r.wlPos.X && 
               wlPos.Y     == r.wlPos.Y &&
               wlPos.world == r.wlPos.world &&
               csItemID    == r.csItemID;
    }
    bool IsReadOnly(){ return m_ReadOnly; }

private:
    // Creates from a wdaFile.
    virtual void CreateFrom( WDAFile &wdaFile, bool createReadOnly );
    bool     m_ReadOnly;
    WorldPos wlPos;
    std::string csItemID;
};

#endif // !defined(AFX_WDAOBJECTSWORLDOBJECTS_H__FECB9F3D_D648_11D2_84B2_00E02922FA40__INCLUDED_)
