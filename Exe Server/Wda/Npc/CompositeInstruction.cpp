/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "CompositeInstruction.h"
#include "InstructionFactory.h"

using namespace std;

/******************************************************************************/
namespace NPC_Editor
/******************************************************************************/
{
	/******************************************************************************/
	CompositeInstruction::CompositeInstruction( std::string name, std::string helpText, InstructionIds id ) :
		Instruction( name, helpText, id )
	/******************************************************************************/
	{
	}
	/******************************************************************************/
	CompositeInstruction::~CompositeInstruction()
	/******************************************************************************/
	{
		// Destroy all instructions
		list< Instruction * >::iterator i;
		for( i = subInstructions.begin(); i != subInstructions.end(); i++ )
		{
			if (*i != NULL)
			{
				delete *i;
				*i = NULL;
			}
		}
	}
	/******************************************************************************/
	bool CompositeInstruction::MoveInstruction(Instruction *ins, Instruction *relativeIns, InsertionRelation relation)
	/******************************************************************************/
	{
		return false;
	}
	/******************************************************************************/
	void CompositeInstruction::DeleteInstruction(Instruction *ins)
	/******************************************************************************/
	{
		// Try to find the instruction
		list< Instruction * >::iterator i;
		for( i = subInstructions.begin(); i != subInstructions.end(); i++ )
		{
			if( *i == ins )
			{
				i = subInstructions.erase( i );
			}
		}
	}
	/******************************************************************************/
	// Adds an instruction. This function will check for a relation between
	// the given relative instruction and its sub instructions, then yield
	// the control flow to them if it hasn't found any.
	bool CompositeInstruction::AddInstruction(Instruction *ins, Instruction *relativeIns, InsertionRelation relation, bool &stopSearch)
	/******************************************************************************/
	{
		// Cannot have a NULL relative instruction.
		ATLASSERT( ins != NULL );

		if( ins == NULL )
		{
			_LOG_DEBUG
				LOG_DEBUG_LVL1,
				"CompositeInstruction::AddInstruction -> Cannot add a NULL instruction."
				LOG_
				return false;
		}

		// If adding an instruction as child of this one.
		if( relation == asChild && ( relativeIns == this || relativeIns == NULL ) )
		{
			// Add it to the instruction list
			subInstructions.push_back( ins );
			return true;
		}

		// Check all instruction for their potential relationship.
		list< Instruction * >::iterator i;
		for( i = subInstructions.begin(); i != subInstructions.end(); i++ )
		{        
			// If forwarding the instruction worked.
			if( (*i)->AddInstruction( ins, relativeIns, relation, stopSearch ) )
			{
				return true;
			}
			// If the forwarding requires that the search stops (a relation was
			// found but adding was refused).
			if( stopSearch )
			{
				return false;
			}

			// Check for the next relation.
			if( relation == asNext && (*i) == relativeIns )
			{
				i++; // insert after.
				// Add the instruction right after this one.
				subInstructions.insert( i, ins );
				return true;
			}
			else if( relation == asPrevious && (*i) == relativeIns )
			{
				subInstructions.insert( i, ins );
				return true;
			}
		}

		// No instruction was found.
		_LOG_DEBUG
			LOG_DEBUG_LVL1,
			"CompositeInstruction::AddInstruction -> The relative instruction was never found." 
		LOG_
		return false;
	}
	/******************************************************************************/
	void CompositeInstruction::Load(WDAFile &file)
	/******************************************************************************/
	{
		// Load the instruction part.
		Instruction::Load( file );

		DWORD version = 0;
		DWORD size = 0, i;

		file.Read( version );
		file.Read( size );
		for( i = 0; i != size; i++ )
		{
			DWORD theId;
			file.Read( theId );

			Instruction *ins = InstructionFactory::GetInstance()->CreateInstruction( (InstructionIds)theId );
			if( ins == NULL )
			{
				throw "Program Error";
			}

			ins->LoadImp( file );
			ins->Load( file );

			ins->UpdateName();

			subInstructions.push_back( ins );
		}
	}
	/******************************************************************************/
	// Returns all sub-instructions under this instruction.
	void CompositeInstruction::GetSubInstructions(std::list< Instruction * > &subIns) // The container that is to receive the instructions.
	/******************************************************************************/
	{
		std::list< Instruction * >::iterator i;
		for( i = subInstructions.begin(); i != subInstructions.end(); i++ )
		{
			subIns.push_back( *i );
		}
	}
	/******************************************************************************/
	// Copies the sub instructions from the given copy
	void CompositeInstruction::Copy(Instruction *theCopy)
	/******************************************************************************/
	{
		// Copy the normal instruction data
		Instruction::Copy( theCopy );

		// Clear the previous sub-instructions.
		std::list< Instruction * >::iterator i;
		for( i = subInstructions.begin(); i != subInstructions.end(); i++ )
		{
			if (*i != NULL)
			{
				delete *i;
				*i = NULL;
			}
		}
		subInstructions.clear();

		list< Instruction * > copySubInstructions;
		theCopy->GetSubInstructions( copySubInstructions );

		// Clone all subinstructions.
		for( i = copySubInstructions.begin(); i != copySubInstructions.end(); i++ )
		{
			subInstructions.push_back( (*i)->Clone() );
		}
	}
} // NPC_Editor

