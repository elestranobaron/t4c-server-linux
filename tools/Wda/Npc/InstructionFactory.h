/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#ifndef __InstructionFactory__
#define __InstructionFactory__

#include "Instruction.h"
#include "NPC_Editor.h"
#include <list>

/******************************************************************************/
namespace NPC_Editor
/******************************************************************************/
{
	class InstructionFactory
	{
	public:
		~InstructionFactory();
		Instruction *CreateInstruction( InstructionIds id );
		void GetInstructionList( std::list< Instruction * > &instructionList ) const;
		static InstructionFactory *GetInstance();
	
	private:
		InstructionFactory();
	};
}

#endif // __InstructionFactory__