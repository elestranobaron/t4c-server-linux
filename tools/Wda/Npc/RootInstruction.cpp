/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "RootInstruction.h"

using namespace std;

/******************************************************************************/
namespace
/******************************************************************************/
{
    string AnGetName()
	{
		return "<root-reserved>";
	}
	string AnGetHelp(){
		return "<root-reserved>";
	}
};
/******************************************************************************/
namespace NPC_Editor
/******************************************************************************/
{
	/******************************************************************************/
	RootInstruction::RootInstruction()	: CompositeInstruction( AnGetName(), AnGetHelp(), InsRoot)
	/******************************************************************************/
	{
	}
	/******************************************************************************/
	RootInstruction::~RootInstruction()
	/******************************************************************************/
	{
	}
	/******************************************************************************/
	// Adds an instruction to the root. First checks if it is added directly 
	// at the root, then forwards it to CompositeInstruction.
	bool RootInstruction::AddInstruction(Instruction *ins, Instruction *relativeIns, InsertionRelation relation, bool &stopSearch)
	/******************************************************************************/
	{
		bool kwCheck = false;

		// If the relative instruction is the root.
		if( relativeIns == this || relativeIns == NULL )
		{
			kwCheck = true;
		}

		// If the relative instruction is part of the root's subinstructions.
		if( relation == asNext || relation == asPrevious )
		{
			list< Instruction * >::iterator i;
			for( i = subInstructions.begin(); i != subInstructions.end(); i++ )
			{
				if( (*i) == relativeIns )
				{
					kwCheck = true;
					break;
				}
			}
		}

		if( kwCheck )
		{
			// Return an error if the instruction isn't a keyword.
			if( ins->GetId() != InsKeyword )
			{
				_LOG_DEBUG
					LOG_DEBUG_LVL1,
					"RootInstruction::AddInstruction -> You cannot add a non-keyword instruction at the root. It must be a keyword."
					LOG_
					stopSearch = true;
				return false;
			}
		}
		return CompositeInstruction::AddInstruction( ins, relativeIns, relation, stopSearch );
	}
	/******************************************************************************/
	void RootInstruction::LoadImp(WDAFile &file)
	/******************************************************************************/
	{	
	}
} // NPC_Editor

