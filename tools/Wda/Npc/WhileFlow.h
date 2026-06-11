/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#ifndef __WhileFlow__
#define __WhileFlow__

#include "../WDAFile.h"
#include "Condition.h"

/******************************************************************************/
namespace NPC_Editor
/******************************************************************************/
{
	class WhileFlow	: public Condition
	{
	public:
		WhileFlow();
		~WhileFlow();      
		void LoadImp( WDAFile &file );
		virtual void SetCondition(const std::string &thecondition);
	
	protected:
		Instruction *Clone();
	
	private:
		void UpdateName();
	};
}

#endif // __WhileFlow__