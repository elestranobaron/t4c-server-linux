#if !defined(AFX_TFCPACKET_H__CCD2D7A2_B380_11D0_9B9E_444553540000__INCLUDED_)
#define AFX_TFCPACKET_H__CCD2D7A2_B380_11D0_9B9E_444553540000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "StandardTypes.h"
#include <cstdint>

#ifdef _WIN32
#include <windows.h>
#else
#include "Win32Compat.h"
#include "Portability.h"
#endif
#include <vector>
#include <string>

using namespace std;

class TFCPacketException{
public:
	TFCPacketException(UINT error);
	UINT m_cause;
};

typedef short RQ_SIZE;

#define KEY_SIZE        2
#define CHECKSUM_SIZE   2

typedef WORD KEY;
typedef WORD CHECKSUM;

#ifdef _WIN32
class __declspec(dllexport) TFCPacket 
#else
class TFCPacket 
#endif
{
public:
	TFCPacket();
	virtual ~TFCPacket();

	void Create(unsigned int length); // Creates a packet
	void Destroy();                   // Destroys a packet
	void Seek(std::int32_t where, char how);    // Seeks within a packet for << and >>

	void Get(short *);				  // Same as >>
	void Get(char *);				  // Gets a "number" from the packet	
	void Get(std::int32_t *);
	void Get(long *);
	void Get(unsigned short *);
	void Get(unsigned char *);
	void Get(std::uint32_t *);
	void Get(unsigned long *);
    void Get( string &str );
	// Mestoph : VÈrification de la taille des strings avant de lire le contenu du data
	bool CheckLen(WORD usLen);

	void EncryptPacket( void );
	BOOL DecryptPacket( unsigned int seedNumber = 0 );

	TFCPacket & operator << (std::int32_t);   // Insertion operators
	TFCPacket & operator << (long);
	TFCPacket & operator << (short);
	TFCPacket & operator << (char);
    TFCPacket & operator << (const char *);
    TFCPacket & operator << (const string & );
#if defined(_AFXDLL) || !defined(_WIN32)
    TFCPacket & operator << (const CString &);
    CString GetDebugPacketString( void );
#endif

    BOOL SetBuffer( LPBYTE lpBuffer, int nBufferSize );
    void GetBuffer( LPBYTE &lpBuffer, int &nBufferSize );
	unsigned int GetPacketSeedID(void);
	void		 SetPacketSeedID(unsigned int newPacketSeedID);

    RQ_SIZE GetPacketID( void );
private:
    
    vector< BYTE > vBuffer;

    unsigned int     nPos;
	unsigned int packetSeedID;

//  int     nLen;

//  BOOL    boUserBuffer;
};



#endif // !defined(AFX_TFCPACKET_H__CCD2D7A2_B380_11D0_9B9E_444553540000__INCLUDED_)
