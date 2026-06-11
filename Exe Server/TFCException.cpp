#include "StdAfx.h"
#include "TFCException.h"
#include <cstring>

TFCException::TFCException()
{
	excp = NULL;
}

TFCException::~TFCException()
{
	if( excp )
	{
		delete excp;
	}
}

void TFCException::SetException( LPEXCEPTION_POINTERS new_excp )
{
	excp = new EXCEPTION_POINTERS;
	memcpy( excp, new_excp, sizeof( EXCEPTION_POINTERS ) );
}

LPEXCEPTION_POINTERS TFCException::GetException()
{
	return excp;
}

void TFCException::SetTFCException( DWORD ExceptionID, LPVOID address )
{
	excp = new EXCEPTION_POINTERS;
	memset( excp, 0, sizeof( EXCEPTION_POINTERS ) );

	excp->ExceptionRecord = new EXCEPTION_RECORD;
	excp->ExceptionRecord->ExceptionCode = ExceptionID;
	excp->ExceptionRecord->ExceptionFlags = 0;
	excp->ExceptionRecord->ExceptionRecord = NULL;
	excp->ExceptionRecord->ExceptionAddress = address;
	excp->ExceptionRecord->NumberParameters = 0;
}
