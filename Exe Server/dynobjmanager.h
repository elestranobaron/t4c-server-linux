/******************************************************************************
Modify for vs2008 (06/05/2009)
Add GetNbrRegisteredUnit() 
Add GetRegisteredUnitID()
Add GetRegisteredUnitID()  by NightMare (27/06/2009)
/******************************************************************************/
#if !defined(AFX_DYNOBJMANAGER_H__10CA361D_EFE8_11D1_830E_00104B2CA38F__INCLUDED_)
#define AFX_DYNOBJMANAGER_H__10CA361D_EFE8_11D1_830E_00104B2CA38F__INCLUDED_

#if _MSC_VER >= 1000
	#pragma once
#endif // _MSC_VER >= 1000

#include "_item.h"
#include "ObjectStructure.h"

/******************************************************************************/
typedef ObjectStructure *( *DYNOBJCALLBACK)( void );

/******************************************************************************/
class __declspec( dllexport ) DynObjManager  
/******************************************************************************/
{
public:
	static void RegisterObjectStructureCallback( WORD wStructureID, DYNOBJCALLBACK lpCallback );

	static ObjectStructure *GetRegisteredUnit( WORD wStructureID, WORD wReferenceID, LPCTSTR lpszUnitName );
   
   static int GetNbrRegisteredUnit();
   static ObjectStructure *GetRegisteredUnitIndex(int iIdx);
   static ObjectStructure *GetRegisteredUnitID(WORD wID);

	static void PostInitDestroy( void );
	static void Destroy( void );
private:
	struct DYNLIST{
		DYNOBJCALLBACK lpCallback;
		WORD wStructureID;		
	};

	static TemplateList< DYNLIST >			tlDynObjects;
	static TemplateList< ObjectStructure >	tlRegisteredUnits;

};

#endif // !defined(AFX_DYNOBJMANAGER_H__10CA361D_EFE8_11D1_830E_00104B2CA38F__INCLUDED_)
