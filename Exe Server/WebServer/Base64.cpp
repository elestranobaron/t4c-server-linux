/*
Module : BASE64.CPP
Purpose: Implementation for a simple base64 decoding class
Created: PJN / 22-04-1999
History: None                    

Copyright (c) 1999 by PJ Naughter.  
All rights reserved.

*/

//////////////// Includes ////////////////////////////////////////////

#include "stdafx.h"
#include "Base64.h"


//////////////// Statics / Macros ////////////////////////////////////

CString CBase64Decoder::m_sBase64Alphabet = 
  _T("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/");

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////// Implementation //////////////////////////////////////

int CBase64Decoder::Decode(const CString& sInput, CString& sOutput)
{
   int i=0;
   m_nBitsRemaining = 0;

   sOutput.Empty();  
   if (sInput.GetLength() == 0)
      return 0;

   //Build Decode Table
   int nDecode[256];
   for (i=0; i<256; i++) 
      nDecode[i] = -2; // Illegal digit
   for (i=0; i<64; i++)
   {
      nDecode[m_sBase64Alphabet[i]] = i;
      nDecode[m_sBase64Alphabet[i]| 0x80] = i; // Ignore 8th bit
      nDecode['='] = -1; 
      nDecode['='| 0x80] = -1; // Ignore MIME padding char
   }

   // Decode the Input
   i=0;
   char* szOutput = sOutput.GetBuffer(sInput.GetLength());
   for (int p=0; p<sInput.GetLength(); p++)
   {
      int c = sInput[p];
      int nDigit = nDecode[c & 0x7F];
      if (nDigit < -1) 
      {
         sOutput.ReleaseBuffer();  
         return 0;
      }
      else if (nDigit >= 0) 
         // i (index into output) is incremented by write_bits()
         WriteBits(nDigit & 0x3F, 6, szOutput, i);
   }	
   szOutput[i] = _T('\0');
   sOutput.ReleaseBuffer();

   return i;
}

void CBase64Decoder::WriteBits(UINT nBits, int nNumBits, LPTSTR szOutput, int& i)
{
	UINT nScratch;
	m_lBitStorage = (m_lBitStorage << nNumBits) | nBits;
	m_nBitsRemaining += nNumBits;
	while (m_nBitsRemaining > 7) 
	{
		nScratch = m_lBitStorage >> (m_nBitsRemaining - 8);
		szOutput[i++] = (char) (nScratch & 0xFF);
		m_nBitsRemaining -= 8;
	}
}

