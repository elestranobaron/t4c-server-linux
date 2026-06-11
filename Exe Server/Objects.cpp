/******************************************************************************
Modify for vs2008 (01/05/2009)
Add Created by for Profession on a item by NightMare (27/06/2009)
/******************************************************************************/
//  This file will handle objects, at least dynamic objects!
//  Francois Leblanc 1997
/******************************************************************************/
#include "stdafx.h"
#include "Objects.h"
#include "TFC_MAIN.h"
#include "T4CLog.h"
#include "TFCTimers.h"

#ifdef _DEBUG
	#undef THIS_FILE
	static char THIS_FILE[]=__FILE__;
	#define new DEBUG_NEW
#endif

/******************************************************************************/
Objects::Objects() : uniqueState( false ), qty( 1 )
/******************************************************************************/
{
   csCreatedBy.Format("");
	backpack = NULL;
	wUndead = 0;
}
/******************************************************************************/
Objects::~Objects()
/******************************************************************************/
{
	if(backpack != NULL)
	{
		backpack->ToHead();
		while(backpack->QueryNext())
		{
            backpack->Object()->DeleteUnit();
            backpack->Remove();
			//backpack->DeleteAbsolute();
		}

		delete backpack;
		backpack = NULL;
	}
}
/******************************************************************************/
BOOL Objects::Create(UINT UnitType, UINT BaseReferenceID)
/******************************************************************************/
{
	// first load the object the normal way
	if(Unit::Create(UnitType, BaseReferenceID))
	{
		return TRUE;
	}
	return FALSE;
}
/******************************************************************************/
int Objects::hit(LPATTACK_STRUCTURE blow, Unit *WhoHit)
/******************************************************************************/
{
	SendUnitMessage( MSG_OnAttacked, this, NULL, WhoHit );
		
	return 0;
}
/******************************************************************************/
TemplateList <Unit> *Objects::GetBackpack()
/******************************************************************************/
{
	return backpack;
}
/******************************************************************************/
void Objects::SetBackpack(TemplateList <Unit> *newBackpack)
/******************************************************************************/
{
	backpack = newBackpack;
}
/******************************************************************************/
// Sets the undead to assign to this object upon resurrection.
void Objects::SetUndead( WORD wNewUndead ) // the new undead.
/******************************************************************************/
{
	wUndead = wNewUndead;
}
/******************************************************************************/
// Returns the undead which should be created when resurrecting this object
WORD Objects::GetUndead( void )
/******************************************************************************/
{
	return wUndead;
}
/******************************************************************************/
//  returns the name of the object.
CString Objects::GetName( WORD wLang )
/******************************************************************************/
{
    // If an overriden name was specified
    if( IsNameOverriden() )
	{
        // return that name
        return csOverridenName.c_str();
    }
    // Otherwise return the default name.
    CString strTmp;
    strTmp = Unit::GetName( wLang );
    if(csCreatedBy != "")
    {
       CString csNameProf;
       csNameProf.Format("%s  (Fabriqué par: %s)",strTmp,csCreatedBy);
       return csNameProf;
    }
    else
      return strTmp;
}
/******************************************************************************/
bool Objects::IsNameOverriden()
/******************************************************************************/
{
    return( !csOverridenName.empty() );
}
/******************************************************************************/
//  Sets the overriden name of an object.
void Objects::SetName( CString csName ) // The new name.
/******************************************************************************/
{
    csOverridenName = (LPCTSTR)csName;
}
/******************************************************************************/
//  Called to terminate the object's existence.
void Objects::VaporizeUnit( bool bLog )
/******************************************************************************/
{
    // Get the object's host world.
    WorldMap *wlWorld = TFCMAIN::GetWorld( GetWL().world );

    if(bLog)
    {
       _LOG_GAMEOP
          LOG_SYSOP,
          "Item named %s (ID %u) was vaporized.",
          (LPCTSTR)GetName( _DEFAULT_LNG ),
          GetID()
          LOG_
    }
    

    // If a world was found.
    if( wlWorld != NULL )
	{
        // Remove it from the world.
        wlWorld->remove_world_unit( GetWL(), GetID() );

		// Broadcast object dissappearance.
        Broadcast::BCObjectRemoved( GetWL(), _DEFAULT_RANGE_REMOVE,GetID()); //Valorize unit
    }

    TFCTimerManager::RemoveTimersByParameter( this ); //oki peu etre une porte en close...

    // Annihilate self.
    DeleteUnit();
}
/******************************************************************************/
//  Returns the quantity of objects
DWORD Objects::GetQty( void )
/******************************************************************************/
{
    return qty;
}
/******************************************************************************/
//  Sets the quantity of items
void Objects::SetQty( DWORD newQty )// The quantity
/******************************************************************************/
{
    // Non-unique items always have as much charges as there are items in the stack.
    if( !IsUnique() )
	{
        // If the charges are not unlimited.
        if( ViewFlag( __FLAG_CHARGES ) >= 0 )
		{
            SetFlag( __FLAG_CHARGES, 1 );
        }
    }
    qty = newQty;
}
/******************************************************************************/
//  Adds items.
void Objects::Add( DWORD addQty )// Quantity to add
/******************************************************************************/
{
    if( !IsUnique() )
	{
        qty += addQty;

        // If the charges are not unlimited.
        if( ViewFlag( __FLAG_CHARGES ) >= 0 )
		{
            // Non-unique items always have as much charges as there are items in the stack.
            SetFlag( __FLAG_CHARGES, 1 );
        }
    }
}
/******************************************************************************/
//  Removes items.
void Objects::Remove( DWORD remQty ) // Quantity to remove
/******************************************************************************/
{
    // Removed unique items get a 0 qty.
    if( qty < remQty || IsUnique() )
	{
        qty = 0;
    }
	else
	{
        qty -= remQty;
    }

    // Non-unique items always have as much charges as there are items in the stack.
    if( !IsUnique() )
	{
        // If the charges are not unlimited.
        if( ViewFlag( __FLAG_CHARGES ) >= 0 )
		{
            SetFlag( __FLAG_CHARGES, 1 );
        }
    }
}
/******************************************************************************/
// Determines wheter the object is unique (non-stackable).
bool Objects::IsUnique( void )
/******************************************************************************/
{
    return uniqueState;
}
/******************************************************************************/
//  Sets an object as being unique
void Objects::SetUnique( void )
/******************************************************************************/
{
    uniqueState = true;
}
/******************************************************************************/
// Override ViewFlag to catch __FLAG_CHARGES and set it to -1 if the Unlimited
// charges value is set on the item reference.
long Objects::ViewFlag(
 unsigned long dwFlagID, // The flag ID
 long lDefaultValue      // The default value to return.
)
/******************************************************************************/
{
    if( dwFlagID == __FLAG_CHARGES )
	{
        // Get the unit structure
        _item *lpItem = NULL;

        SendUnitMessage( MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &lpItem );

        // If it has unlimited charges return -1.
        if( lpItem != NULL && lpItem->magic.charges < 0 )
		{
            return -1;
        }
    }
    return DynamicFlags::ViewFlag( dwFlagID, lDefaultValue );
}

void    Objects::SetCreatedBy( char * pName )
{
   csCreatedBy.Format("%s",pName);
}


CString Objects::GetCreatedBy( )
{
   return csCreatedBy;
}