/*
Module : BASE64.H
Purpose: Defines the interface for a simple base64 decoding class
Created: PJN / 22-04-1999
History: None

Copyright (c) 1999 by PJ Naughter.  
All rights reserved.

*/


/////////////////////////////// Defines ///////////////////////////////////////

#ifndef __BASE64_H__
#define __BASE64_H__


/////////////////////////////// Classes ///////////////////////////////////////

class CBase64Decoder
{
public:
//Methods
	int Decode(const CString& sInput, CString& sOutput);

protected:
	void WriteBits(UINT nBits, int nNumBts, LPTSTR szOutput, int& lp );

	int m_nBitsRemaining;
	ULONG m_lBitStorage;
	LPCTSTR m_szInput;
	static CString m_sBase64Alphabet;
};

#endif //__BASE64_H__
