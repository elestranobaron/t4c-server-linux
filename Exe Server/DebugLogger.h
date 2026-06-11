#if !defined(AFX_DEBUGLOGGER_H__8D89B71E_024E_11D3_84F6_00E02922FA40__INCLUDED_)
#define AFX_DEBUGLOGGER_H__8D89B71E_024E_11D3_84F6_00E02922FA40__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <string>
#include <deque>
#include "Lock.h"
#include "Portability.h"

#ifdef _WIN32
#pragma warning( disable : 4786 )
#endif

class DebugLogger : public CLock
{
public:	
	~DebugLogger();

    static DebugLogger &GetInstance();

    void LogString( std::string csString, bool boLogTime = false );

    void Flush( void );

    void AdjustMaxLogSize( unsigned long dwNewSize );

private:
    unsigned long GetStepBackSize( void );

    unsigned long dwMaxLogSize;

    DebugLogger();

    std::deque< String > qStrings;
};
  
#endif
