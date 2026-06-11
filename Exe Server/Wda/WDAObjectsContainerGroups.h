/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#if !defined(AFX_WDAOBJECTSCONTAINERGROUPS_H__FECB9F39_D648_11D2_84B2_00E02922FA40__INCLUDED_)
#define AFX_WDAOBJECTSCONTAINERGROUPS_H__FECB9F39_D648_11D2_84B2_00E02922FA40__INCLUDED_

#if _MSC_VER >= 1000
	#pragma once
#endif // _MSC_VER >= 1000

#include "WDATable.h"
#include "WDAObjectsContainerItems.h"

class WDAObjects;
/******************************************************************************/
class WDAObjectsContainerGroups : public WDATable  
/******************************************************************************/
{
public:
    WDAObjectsContainerGroups( vir::Logger &cLogger, vir::DEBUG_LEVEL dlDebugHigh );

    // Only WDAObjects can interface this class. See WDAObjects.h
    friend WDAObjects;
	virtual ~WDAObjectsContainerGroups();
    // Accessors
    std::vector< WDAObjectsContainerItems > &GetItems( void );
    // Deep-copy operators
    void operator = ( const WDAObjectsContainerGroups &cParam )
	{
        vItems = cParam.vItems;
    }
    bool operator == (const WDAObjectsContainerGroups &o );

private:
    // Creates from a wdaFile.
    virtual void CreateFrom( WDAFile &wdaFile, bool createReadOnly );
    // The items in the group.
    std::vector< WDAObjectsContainerItems > vItems;
};

#endif // !defined(AFX_WDAOBJECTSCONTAINERGROUPS_H__FECB9F39_D648_11D2_84B2_00E02922FA40__INCLUDED_)
