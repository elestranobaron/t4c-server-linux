/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#ifndef __Assign__
#define __Assign__

#include "Instruction.h"
#include "NPC_Editor.h"
#include "../WDAFile.h"
#include <list>

/******************************************************************************/
namespace NPC_Editor
/******************************************************************************/
{
	class Assign	: public Instruction
	{
	public:
		Assign( std::string name, std::string helpText, InstructionIds id );
		~Assign();
	    std::string GetValue()
		{
			return value; 
		};
	    void SetValue( const std::string &val )
		{
			value = val; 
		};
	    std::string GetVar()
		{
			return var; 
		};
	    void SetVar( const std::string &thevar )
		{
			var = thevar; 
		};
	    void LoadImp( WDAFile &file );
		virtual void GetSubInstructions( std::list< Instruction * > &subInstructions )
		{};

	protected:
		Instruction *Clone();

	private:
		void UpdateName();
		std::string var;
		std::string value;
	};
}
#endif // __Command__