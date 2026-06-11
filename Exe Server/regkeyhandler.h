#if !defined(AFX_REGKEYHANDLER_H__1B901DD3_FE30_11D0_88FB_00E029058623__INCLUDED_)
#define AFX_REGKEYHANDLER_H__1B901DD3_FE30_11D0_88FB_00E029058623__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef _WIN32
#include "Win32Compat.h"
#include "StandardTypes.h"
#include <string>
#ifndef __declspec
#define __declspec(x)
#endif
#endif

class __declspec(dllexport) RegKeyHandler  
{
public:
	RegKeyHandler();
	virtual ~RegKeyHandler();

	BOOL Open(HKEY key, LPCTSTR subkey);
	BOOL Create(HKEY key, LPCTSTR subkey);
	void Close( void );

	BOOL DeleteValue( LPCTSTR item );
	BOOL DeleteKey(  LPCTSTR subkey );

	LPCTSTR GetProfileString(LPCTSTR item, LPCTSTR default_arg);
	DWORD   GetProfileInt(LPCTSTR item, DWORD default_arg);

	void WriteProfileString(LPCTSTR item, LPCTSTR value);
	void WriteProfileInt(LPCTSTR item, DWORD value);

#ifndef _WIN32
	/** Charge T4CServer.ini une fois (thread-safe). */
	static void EnsureIniLoaded();
	/** Recharge le fichier INI sous mutex (AutoConfig / rafraichissement explicite). */
	static bool ReloadIniFromDisk();
#endif

private:
#ifndef _WIN32
	std::string m_iniSubKey;
#else
	LPCTSTR subkey;
	HKEY    mainkey;
	HKEY    keyhandle;
#endif
	TCHAR	returnstr[1024];
};

#endif // !defined(AFX_REGKEYHANDLER_H__1B901DD3_FE30_11D0_88FB_00E029058623__INCLUDED_)
