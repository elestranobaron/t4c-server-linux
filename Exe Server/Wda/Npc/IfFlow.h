/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#ifndef __IfFlow__
#define __IfFlow__

#include "../WDAFile.h"
#include "ControlFlow.h"
#include "ElseFlow.h"
#include "ElseIfFlow.h"

/******************************************************************************/
namespace NPC_Editor
/******************************************************************************/
{
	class IfFlow	: public Condition
	{
	public:
		IfFlow();
		~IfFlow();
		void LoadImp( WDAFile &file );
		void GetElseIfs( std::list< Instruction * > &elseIfs );
		ElseFlow* GetElseFlow();
		virtual bool AddInstruction( Instruction *ins, Instruction *relativeIns, InsertionRelation relation, bool &stopSearch );
		virtual void DeleteInstruction( Instruction *ins );
		virtual void SetCondition(const std::string &thecondition);
	
	protected:
		Instruction *Clone();
	
	private:
		void UpdateName();
		std::list<ElseIfFlow*>  elseIfs;
		ElseFlow               *elseFlow; 
	};            
}

#endif // __IfFlow__