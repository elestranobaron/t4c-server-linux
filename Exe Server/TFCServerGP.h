#ifndef __TFCSERVERGP_H
#define __TFCSERVERGP_H

#include "StandardTypes.h"
#include "Portability.h"
#ifndef _WIN32
#ifndef LPVOID
typedef void* LPVOID;
#endif
#endif
#ifdef _WIN32
#include <eh.h>
#endif
#include "TFCException.h"

#ifndef _WIN32
typedef String CString;
class CFile;
#ifndef BOOL
typedef int BOOL;
#endif
#ifndef LPVOID
typedef void* LPVOID;
#endif
#endif

void ExceptionFunction(unsigned int u, EXCEPTION_POINTERS* pExp);
BOOL LockGP( BOOL boTry );

#ifndef DISABLE_GP
#else
#endif

#define LOCK_GP		LockGP( FALSE );
#define TRYLOCK_GP  LockGP( TRUE )

#define REPORT_FUNC_PROTOTYPE   CFile csGP, TFCException *e

typedef void ( *REPORT_FUNC )( REPORT_FUNC_PROTOTYPE );

void ReportGP( TFCException *e, CString csThreadNameID );

void RegisterReportFunction( REPORT_FUNC lpReportFunc );

#endif
