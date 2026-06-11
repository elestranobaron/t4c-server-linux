/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#ifndef __Instruction__
#define __Instruction__

#include "NPC_Editor.h"
#include "../WDAFile.h"
#include <string>
#include <list>

/******************************************************************************/
namespace NPC_Editor
/******************************************************************************/
{
	class InstructionFactory;
	class Instruction
	{
	public:
		friend InstructionFactory;
		Instruction( std::string name, std::string helpText, InstructionIds id );
		~Instruction();
		std::string GetName() const {
			return name;  
		}
		void SetName(const std::string &thename)
		{
			name = thename; 
		}
		std::string GetHelpText() const 
		{
			return helpText; 
		}
		void SetHelpText(const std::string &thehelpText)
		{
			helpText = thehelpText; 
		}
		InstructionIds GetId() const 
		{
			return id; 
		}
		void SetId(const InstructionIds &theid)
		{
			id = theid; 
		}
		virtual bool AddInstruction( Instruction *ins, Instruction *relativeIns, InsertionRelation relation, bool &stopSearch );
		virtual bool MoveInstruction( Instruction *ins, Instruction *relativeIns, InsertionRelation relation );
		virtual void GetSubInstructions( std::list< Instruction * > &subInstructions ) = 0;
		virtual void Load( WDAFile &file );    
		virtual void LoadImp( WDAFile &file ) = 0;
		virtual bool AcceptsSubInstructions()
		{
			return false;
		}
	    virtual void Copy( Instruction *theCopy );
		virtual Instruction *Clone() = 0;
		virtual void UpdateName() = 0;
	
	private:
		std::string name;
		std::string helpText;
		InstructionIds id;
	};
}

#endif // __Instruction__