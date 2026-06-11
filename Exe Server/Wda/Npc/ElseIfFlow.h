/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#ifndef __ElseIfFlow__
#define __ElseIfFlow__

#include "Condition.h"
#include "../WDAFile.h"

/******************************************************************************/
namespace NPC_Editor
/******************************************************************************/
{
	class IfFlow;
	class ElseIfFlow	: public Condition
	{
	public:
		friend IfFlow;
		ElseIfFlow();
		~ElseIfFlow();
		void LoadImp( WDAFile &file );
		virtual void SetCondition(const std::string &thecondition);
	
	protected:
		Instruction *Clone();
	
	private:
		void UpdateName();
	};
}

#endif // __ElseIfFlow__