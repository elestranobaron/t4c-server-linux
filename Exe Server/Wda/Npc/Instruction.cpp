/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "Instruction.h"

using namespace std;

/******************************************************************************/
namespace NPC_Editor
/******************************************************************************/
{	
	/******************************************************************************/
	Instruction::Instruction( std::string iname, std::string ihelpText, InstructionIds iid ) :
		name( iname ), helpText( ihelpText ), id( iid )
	/******************************************************************************/
	{
	}
	/******************************************************************************/
	Instruction::~Instruction()
	/******************************************************************************/
	{
	}
	/******************************************************************************/
	bool Instruction::AddInstruction(Instruction *ins, Instruction *relativeIns, InsertionRelation relation, bool &stopSearch)
	/******************************************************************************/
	{
		// If the instruction is related to this one.
		if( relativeIns == this && relation == asChild )
		{
			stopSearch = true;
			_LOG_DEBUG
				LOG_DEBUG_LVL1,
				"Instruction::AddInstruction -> You cannot add a child instruction to this type of instruction."
				LOG_
		}

		return false;	
	}
	/******************************************************************************/
	bool Instruction::MoveInstruction(Instruction *ins, Instruction *relativeIns, InsertionRelation relation)
	/******************************************************************************/
	{
		return false;	
	}
	/******************************************************************************/
	void Instruction::Load(WDAFile &file)
	/******************************************************************************/
	{
		DWORD version;

		file.Read( version );
	}
	/******************************************************************************/
	// Default to returning nothing.
	void Instruction::GetSubInstructions(std::list< Instruction * > &subInstructions)
	/******************************************************************************/
	{
		return;    
	}
	/******************************************************************************/
	// Copies an instruction according to the copy.
	void Instruction::Copy(Instruction *theCopy)
	/******************************************************************************/
	{
		name = theCopy->name;
		helpText = theCopy->helpText;
		id = theCopy->id;
	}
} // NPC_Editor

