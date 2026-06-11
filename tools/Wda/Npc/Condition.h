/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#ifndef __Condition__
#define __Condition__

#include "ControlFlow.h"

/******************************************************************************/
namespace NPC_Editor
/******************************************************************************/
{
	class Condition	: public ControlFlow
	{
	public:
		Condition::Condition( std::string name, std::string helpText, InstructionIds id ) : ControlFlow( name, helpText, id )
		{}
		virtual std::string GetCondition() const { 
			return condition;
		} 
		virtual void SetCondition(const std::string &thecondition)
		{
			condition = thecondition; 
		}
		virtual void Copy( Instruction *theCopy )
		{
			this->condition = static_cast< Condition * >( theCopy )->condition;
			ControlFlow::Copy( theCopy );
		}
		void LoadImp( WDAFile &file ) = 0
		{
			file.Read( condition );
		}
		void UpdateName( std::string condName )
		{
			if( condition.size() == 0 )
			{
				SetName( condName );        
				return;
			}
			std::string newName = condName;
			newName += "( ";
			newName += condition;
			newName += " )";
			SetName( newName );
		}
	
	protected:
	private:
		std::string condition;
	};            
}

#endif // __ControlFlow__