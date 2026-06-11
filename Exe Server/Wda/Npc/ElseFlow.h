/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#ifndef __ElseFlow__
#define __ElseFlow__

#include "ControlFlow.h"
#include "../WDAFile.h"

/******************************************************************************/
namespace NPC_Editor
/******************************************************************************/
{
	class IfFlow;
	class ElseFlow	: public ControlFlow
	{
	public:
		friend IfFlow;
		ElseFlow();
		~ElseFlow();
		void LoadImp( WDAFile &file );
	
	protected:
		Instruction *Clone();
	
	private:
		void UpdateName(){};
	};
}
#endif // __ElseFlow__