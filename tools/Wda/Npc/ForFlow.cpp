/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "ForFlow.h"

using namespace std;

/******************************************************************************/
namespace
{
    string AnGetName()
	{
        return GetAppString( IDS_INST_FOR );
    }
    string AnGetHelp()
	{
        return GetAppString( IDS_INST_FOR_HELP );
    }
};
/******************************************************************************/
namespace NPC_Editor
/******************************************************************************/
{
	/******************************************************************************/
	ForFlow::ForFlow()	: ControlFlow( AnGetName(), AnGetHelp(), InsFor )
	/******************************************************************************/
	{
	}
	/******************************************************************************/
	ForFlow::~ForFlow()
	/******************************************************************************/
	{
	}
	/******************************************************************************/
	void ForFlow::LoadImp(WDAFile &file)
	/******************************************************************************/
	{
		file.Read( startValue );
		file.Read( endValue );
		file.Read( assignedVar );	
	}
	/******************************************************************************/
	// Clone a FOR
	Instruction *ForFlow::Clone( void )
	/******************************************************************************/
	{
		ForFlow *theFor = new ForFlow;

		theFor->Copy( this );
		theFor->startValue = this->startValue;
		theFor->endValue = this->endValue;
		theFor->assignedVar = this->assignedVar;

		return theFor;
	}
	/******************************************************************************/
	// Update the name.
	void ForFlow::UpdateName( void )
	/******************************************************************************/
	{
		char buf[ 4096 ];
		sprintf_s( 
			buf,
			4096,
			GetAppString( IDS_FOR_DISPLAY ).c_str(), 
			assignedVar.c_str(),
			startValue.c_str(),
			endValue.c_str()
			);

		SetName( buf );
	}
} // NPC_Editor

