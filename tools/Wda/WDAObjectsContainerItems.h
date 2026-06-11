/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#if !defined(AFX_WDAOBJECTSCONTAINERITEMS_H__FECB9F38_D648_11D2_84B2_00E02922FA40__INCLUDED_)
#define AFX_WDAOBJECTSCONTAINERITEMS_H__FECB9F38_D648_11D2_84B2_00E02922FA40__INCLUDED_

#if _MSC_VER >= 1000
	#pragma once
#endif // _MSC_VER >= 1000

#include "WDATable.h"
#include <string>

class WDAObjectsContainerGroups;
/******************************************************************************/
class WDAObjectsContainerItems : public WDATable  
/******************************************************************************/
{
public:
    // Only WDAObjectsContainerGroups can access this class. See WDAObjects.h
    friend WDAObjectsContainerGroups;
    WDAObjectsContainerItems( vir::Logger &cLogger, vir::DEBUG_LEVEL dlDebugHigh );
    virtual ~WDAObjectsContainerItems();
    // Accessors
    const std::string &GetItemID() const;
    // Deep copy operator
    void operator = ( const WDAObjectsContainerItems &cParam )
	{
        csItemID = cParam.csItemID;
    };
    bool operator == (const WDAObjectsContainerItems &cParam )
	{
        return csItemID == cParam.csItemID;
    }
    void Setup( std::string itemID )
	{
        csItemID = itemID;
    }
private:
    
    // Creates from a wdaFile.
    virtual void CreateFrom( WDAFile &wdaFile, bool createReadOnly );
    // Data
    std::string csItemID;
};

#endif // !defined(AFX_WDAOBJECTSCONTAINERITEMS_H__FECB9F38_D648_11D2_84B2_00E02922FA40__INCLUDED_)
