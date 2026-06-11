/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#ifndef __RootInstruction__
#define __RootInstruction__

#include "CompositeInstruction.h"
#include "Instruction.h"
#include "NPC_Editor.h"
#include "../WDAFile.h"

/******************************************************************************/
namespace NPC_Editor
/******************************************************************************/
{
	class RootInstruction	: public CompositeInstruction
	{
	public:
		RootInstruction();
		~RootInstruction();   
		bool AddInstruction( Instruction *ins, Instruction *relativeIns, InsertionRelation relation, bool &stopSearch );    
	    void LoadImp( WDAFile &file );

	protected:
		Instruction *Clone(){ return NULL; };

	private:
		void UpdateName(){};
	};
}

#endif // __RootInstruction__