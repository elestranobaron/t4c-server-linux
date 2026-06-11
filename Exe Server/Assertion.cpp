/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
// Contract pattern implementation code.
//
// Defines the assertion handler implementation code.
// This part is hidden from the interface for easy update of assertion handling.

#include "stdafx.h"
#include "Assertion.h"

using namespace std;

/******************************************************************************/
// This assertion implementation throws a  exception
class AssertionThrow : public AssertionImp
/******************************************************************************/
{
    virtual void HandlePrecondition()
    {
        throw;// new logic_error( "Precondition error" );
    };

    virtual void HandlePostcondition(){
        throw;// new logic_error( "Postcondition error" );
    };
};

/******************************************************************************/
AssertionImp *AssertionImp::GetDefaultImp()
/******************************************************************************/
{
    static AssertionThrow inst;

    return &inst;
}



