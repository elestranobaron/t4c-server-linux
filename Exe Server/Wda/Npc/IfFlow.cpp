/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "IfFlow.h"
#include "InstructionFactory.h"

using namespace std;

/******************************************************************************/
namespace
{
    string AnGetName()
	{
        return GetAppString( IDS_INST_IF );
    }
    string AnGetHelp()
	{
        return GetAppString( IDS_INST_IF_HELP );
    }    
};
/******************************************************************************/
namespace NPC_Editor
/******************************************************************************/
{
	/******************************************************************************/
	IfFlow::IfFlow() : Condition( AnGetName(), AnGetHelp(), InsIf ), elseFlow( NULL )
	/******************************************************************************/
	{
	}
	/******************************************************************************/
	IfFlow::~IfFlow()
	/******************************************************************************/
	{
		if( elseFlow != NULL )
		{
			delete elseFlow;
			elseFlow = NULL;
		}
		list< ElseIfFlow * >::iterator i;
		for( i = elseIfs.begin(); i != elseIfs.end(); i++ )
		{
			if (*i != NULL)
			{
				delete *i;
				*i = NULL;
			}
		}
	}
	/******************************************************************************/
	void IfFlow::LoadImp(WDAFile &file)
	/******************************************************************************/
	{
		Condition::LoadImp( file );

		bool isElseFlow = false;
		file.Read( isElseFlow );

		if( isElseFlow )
		{
			Instruction *ins = InstructionFactory::GetInstance()->CreateInstruction( InsElse );
			if( ins == NULL )
			{
				throw "Program Error";
			}
			ins->LoadImp( file );
			ins->Load( file );

			ins->UpdateName();

			elseFlow = static_cast< ElseFlow * >( ins );
		}

		DWORD size = 0, i;

		file.Read( size );
		for( i = 0; i != size; i++ )
		{
			Instruction *ins = InstructionFactory::GetInstance()->CreateInstruction( InsElseIf );
			if( ins == NULL ){
				throw "Program Error";
			}
			ins->LoadImp( file );
			ins->Load( file );

			ins->UpdateName();

			elseIfs.push_back( static_cast< ElseIfFlow * >( ins ) );
		}
	}
	/******************************************************************************/
	// Checks the ElseIF and Else.
	bool IfFlow::AddInstruction(
		Instruction *ins,              // The instruction to add
		Instruction *relativeIns,      // The relative instruction.
		InsertionRelation relation,    // The relation.
		bool &stopSearch               // 
	)
	/******************************************************************************/
	{
		// If this is a Else or ElseIF to add to this 
		if( ins->GetId() == InsElse && relativeIns == this )
		{
			stopSearch = true;
			if( elseFlow != NULL )
			{
				_LOG_DEBUG
					LOG_DEBUG_LVL1,
					"IfFlow::AddInstruction -> There can only be one ELSE instruction per IF."
					LOG_
					return false;
			}
			// Add the instruction to the elseFlow.
			elseFlow = static_cast< ElseFlow * >( ins );
			return true;
		}
		else if( ins->GetId() == InsElseIf && relativeIns == this )
		{
			stopSearch = true;        
			// Add the instruction to the elseIfFlow.
			elseIfs.push_back( static_cast< ElseIfFlow * >( ins ) );
			return true;
		}
		else
		{
			if( elseFlow != NULL )
			{
				if( elseFlow->AddInstruction( ins, relativeIns, relation, stopSearch ) )
				{
					return true;
				}
				if( stopSearch )
				{
					return false;
				}
			}
			// Check all elseIfs for their potential relationship.
			list< ElseIfFlow * >::iterator i;
			for( i = elseIfs.begin(); i != elseIfs.end(); i++ )
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
			}

			return CompositeInstruction::AddInstruction( ins, relativeIns, relation, stopSearch );
		}
	}
	/******************************************************************************/
	// Deletes an instruction.
	void IfFlow::DeleteInstruction(Instruction *ins) // The instruction to delete.
	/******************************************************************************/
	{
		if( ins == elseFlow )
		{
			elseFlow = NULL;
		}

		list< ElseIfFlow * >::iterator i;
		for( i = elseIfs.begin(); i != elseIfs.end(); i++ )
		{
			if( *i == ins ){
				i = elseIfs.erase( i );
			}
		}

		CompositeInstruction::DeleteInstruction( ins );
	}
	/******************************************************************************/
	// Returns the list of ElseIfs.
	void IfFlow::GetElseIfs(std::list< Instruction * > &ielseIfs) // The list of ElseIfs.
	/******************************************************************************/
	{
		list< ElseIfFlow * >::iterator i;
		for( i = elseIfs.begin(); i != elseIfs.end(); i++ )
		{
			ielseIfs.push_back( *i );
		}
	}
	/******************************************************************************/
	// Returns the else flow. NULL if none.
	ElseFlow* IfFlow::GetElseFlow( void )
	/******************************************************************************/
	{
		return elseFlow;
	}
	/******************************************************************************/
	// Clone IF
	Instruction *IfFlow::Clone( void )
	/******************************************************************************/
	{
		IfFlow *iff = new IfFlow;

		iff->Copy( this );

		// Clone all ElseIfs
		list< ElseIfFlow * >::iterator i;
		for( i = elseIfs.begin(); i != elseIfs.end(); i++ )
		{
			iff->elseIfs.push_back( static_cast< ElseIfFlow * >( (*i)->Clone() ) );
		}

		// Clone the else flow.
		if( elseFlow != NULL )
		{
			iff->elseFlow = static_cast< ElseFlow * >( this->elseFlow->Clone() );
		}

		return iff;
	}
	/******************************************************************************/
	// Update the IF's content.
	void IfFlow::UpdateName( void )
	/******************************************************************************/
	{
		Condition::UpdateName( AnGetName() );
	}
	/******************************************************************************/
	// Sets the condition and updates the name
	void IfFlow::SetCondition(const std::string &thecondition) // The expression.
	/******************************************************************************/
	{
		Condition::SetCondition( thecondition );
		UpdateName();
	}
} // NPC_Editor

