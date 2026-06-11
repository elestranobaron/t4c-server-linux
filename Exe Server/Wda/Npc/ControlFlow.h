/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#ifndef __ControlFlow__
#define __ControlFlow__

#include "CompositeInstruction.h"

/******************************************************************************/
namespace NPC_Editor
/******************************************************************************/
{
	class ControlFlow	: public CompositeInstruction
	{
	public:
		ControlFlow( std::string name, std::string helpText, InstructionIds id );
		~ControlFlow();
	
	protected:
	private:
	};            
}

#endif // __ControlFlow__