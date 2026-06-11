// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__BC8F306A_A74F_11D0_9B9E_444553540000__INCLUDED_)
#define AFX_STDAFX_H__BC8F306A_A74F_11D0_9B9E_444553540000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifdef _WIN32

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#ifndef _WIN32_WINNT        // Allow use of features specific to Windows NT 4 or later.
#define _WIN32_WINNT 0x0400        // Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif 

//#define _AFX_NO_OLE_SUPPORT
//#define _AFX_NO_DB_SUPPORT
//#define _AFX_NO_DAO_SUPPORT
#pragma warning( disable : 4786 )   // Debug string too long.
#pragma warning( disable : 4284 )   // Return type of operator ->

// To disable when debugging app.
#pragma warning( disable : 4244 )   // Conversion from 'type1' to 'type2'
#pragma warning( disable : 4018 )   // Signed/unsigned mismatched.

//#define __ENABLE_LOG
#include <afxdao.h>

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxmt.h>

#pragma comment(lib, "ws2_32.lib")
#include <afxsock.h>	// MFC socket extensions

#ifdef MEM_DEBUG
    #include <smrtheap.hpp>
#endif

#include "CustomBuild.h"

#include <windows.h>
#include <time.h>
#include <ctime>
#include <process.h>
#include <math.h>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <queue>
#include <deque>
#include <stack>
#include <string>
#include <algorithm>
#include "Lock.h"

#include "ExitCode.h"
#include "Timer.h"
#include "CrashRpt.h"
#ifdef __ENABLE_LOG
	extern DEBUG_LOG __LOG;
#endif

#else /* Linux server build */

#include <cstddef>
#include <cstring>
#include <ctime>
#include <cmath>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <queue>
#include <deque>
#include <stack>
#include <string>
#include <algorithm>
#include "Win32Compat.h"
#include "Portability.h"
#include "Lock.h"

#ifndef TRACE
#define TRACE(...) ((void)0)
#endif

#include "ExitCode.h"
#include "Timer.h"
#ifdef __ENABLE_LOG
	extern DEBUG_LOG __LOG;
#endif

#endif /* _WIN32 */

namespace vir{};
using namespace vir;
//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__BC8F306A_A74F_11D0_9B9E_444553540000__INCLUDED_
