#ifndef CRYPT_H
#define CRYPT_H

#ifdef _WIN32
#include <windows.h>
#endif
#include "Xorkey.h"

namespace TFCCrypt 
{   
	int DecryptS(unsigned char *&pBuffer, int &pBufferSize, unsigned int dwKey = 0xFFFFFFFF); // Decrypts pBuffer and returns true on success and false on failure
	int EncryptS(unsigned char *&pBuffer, int &pBufferSize, unsigned int dwKey = 0xFFFFFFFF); // Encrypts pBuffer and returns the seed used on encryption
	unsigned long DecryptS2(unsigned char *&pBuffer, int &pBufferSize,unsigned int dwKey = 0xFFFFFFFF);
}

#endif // CRYPT_H
