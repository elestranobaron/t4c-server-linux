/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#ifndef __CompositeInstruction__
#define __CompositeInstruction__

#include "Instruction.h"
#include "NPC_Editor.h"
#include "../WDAFile.h"

/******************************************************************************/
namespace NPC_Editor
/******************************************************************************/
{
	class CompositeInstruction	: public Instruction
	{
	public:
		CompositeInstruction( std::string name, std::string helpText, InstructionIds id );
		~CompositeInstruction();   
		bool MoveInstruction( Instruction *ins, Instruction *relativeIns, InsertionRelation relation );
		void DeleteInstruction( Instruction *ins );
		bool AddInstruction( Instruction *ins, Instruction *relativeIns, InsertionRelation relation, bool &stopSearch );        
		virtual void Load( WDAFile &file );    
	    void GetSubInstructions( std::list< Instruction * > &subInstructions );
	    virtual bool AcceptsSubInstructions()
		{
			return true;
		}
		virtual Instruction *Clone() = 0;
		virtual void Copy( Instruction *theCopy );

	protected:
		std::list<Instruction*>	subInstructions; 
	};            
}

#endif // __CompositeInstruction__