/******************************************************************************
Modify for vs2008 (30/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "TFCPacket.h"
#include <iterator>
#include <functional>

#ifdef _AFXDLL
	#ifdef _DEBUG
		#undef THIS_FILE
		static char THIS_FILE[]=__FILE__;
		#define new DEBUG_NEW
	#endif 
#endif 

#define GROW_BY					5
#define HEADER_SIZE				( KEY_SIZE + CHECKSUM_SIZE )

/******************************************************************************/
TFCPacketException::TFCPacketException(UINT cause)
/******************************************************************************/
{
	m_cause = cause;
}
/******************************************************************************/
TFCPacket::TFCPacket( void )
/******************************************************************************/
{
	// Reserve 64 bytes default size.
	vBuffer.reserve( 64 );

	// Create the space for the initial header.
	int i;
	for( i = 0; i < HEADER_SIZE; i++ )
	{
		vBuffer.push_back( 0 );
	}

	nPos = 0;
	packetSeedID = 0;
}
/******************************************************************************/
 TFCPacket::~TFCPacket( void )
/******************************************************************************/
{
}
/******************************************************************************/
// Destroys a packet. Allows reusing a packet object.
void TFCPacket::Destroy( void )
/******************************************************************************/
{
	BYTE bHeader[ HEADER_SIZE ] = { 0, 0, 0, 0 };

	// Destroy the vector, but not its header.
	vBuffer.erase( vBuffer.begin() + HEADER_SIZE, vBuffer.end() );

	nPos = 0;
}
/******************************************************************************/
// Seeks the position of in the packet.
void TFCPacket::Seek(std::int32_t where, char how)
/******************************************************************************/
{
	switch(how)
	{
		case 0: nPos = where; break;
		case 1: nPos += where; break;
	}
}
/******************************************************************************/
TFCPacket &TFCPacket::operator << (short value)
/******************************************************************************/
{
	vBuffer.push_back( HIBYTE(value) );
	vBuffer.push_back( LOBYTE(value) );

	return *this;
}
/******************************************************************************/
TFCPacket &TFCPacket::operator << (char value)
/******************************************************************************/
{
    vBuffer.push_back( (BYTE)value );

    return(*this);
}
/******************************************************************************/
TFCPacket &TFCPacket::operator << (long value)
/******************************************************************************/
{
	vBuffer.push_back( HIBYTE(HIWORD(value)) );
	vBuffer.push_back( LOBYTE(HIWORD(value)) );
	vBuffer.push_back( HIBYTE(LOWORD(value)) );
	vBuffer.push_back( LOBYTE(LOWORD(value)) );

	return(*this);
}
/******************************************************************************/
TFCPacket & TFCPacket:: operator << (const char * lpszString)
/******************************************************************************/
{
	int nStrLen = strlen(lpszString);

	// Stored string length.
	vBuffer.push_back( HIBYTE( nStrLen ) );
	vBuffer.push_back( LOBYTE( nStrLen ) );

	// Copy string into vector.
	copy( lpszString, lpszString + nStrLen, back_inserter( vBuffer ) );

	return *this;
}
/******************************************************************************/
TFCPacket &TFCPacket::operator << ( const string &csString )
/******************************************************************************/
{
	return operator<< ( csString.c_str() );
}

#ifdef _AFXDLL
/******************************************************************************/
TFCPacket & TFCPacket:: operator << (CString &csString)
/******************************************************************************/
{
	int nStrLen = csString.GetLength();

	// Stored string length.
	vBuffer.push_back( HIBYTE( nStrLen ) );
	vBuffer.push_back( LOBYTE( nStrLen ) );

	LPBYTE lpStringBuffer = (LPBYTE)csString.GetBuffer( 0 );

	// Copy string buffer into vector
	copy( lpStringBuffer, lpStringBuffer + nStrLen, back_inserter( vBuffer ) );

	csString.ReleaseBuffer( nStrLen );

	return *this;
}
#endif

#ifndef _WIN32
/******************************************************************************/
TFCPacket & TFCPacket::operator << (const CString &csString)
/******************************************************************************/
{
	const int nStrLen = static_cast<int>(csString.GetLength());

	vBuffer.push_back( HIBYTE( nStrLen ) );
	vBuffer.push_back( LOBYTE( nStrLen ) );

	const char *lpStringBuffer = csString.c_str();
	if (nStrLen > 0 && lpStringBuffer) {
		copy( lpStringBuffer, lpStringBuffer + nStrLen, back_inserter( vBuffer ) );
	}

	return *this;
}
/******************************************************************************/
CString TFCPacket::GetDebugPacketString( void )
/******************************************************************************/
{
	CString csTemp;
	CString csFinal;

	for (unsigned int i = 0; i < vBuffer.size() - HEADER_SIZE; i++) {
		csTemp.Format( "%u ", vBuffer[ HEADER_SIZE + i ] );
		csFinal += csTemp;
	}

	return csFinal;
}
#endif
/******************************************************************************/
void TFCPacket::Get(long *i)
/******************************************************************************/
{
	*i = 0;
	if( HEADER_SIZE + nPos + 4 <= vBuffer.size() )
	{
		*i =  vBuffer[ HEADER_SIZE + nPos++ ] << 24;
		*i += vBuffer[ HEADER_SIZE + nPos++ ] << 16;
		*i += vBuffer[ HEADER_SIZE + nPos++ ] << 8;
		*i += vBuffer[ HEADER_SIZE + nPos++ ];
	}
}
/******************************************************************************/
void TFCPacket::Get(short *i)
/******************************************************************************/
{
    *i = 0;
	if( HEADER_SIZE + nPos + sizeof( short ) <= vBuffer.size() )
	{
		*i =  vBuffer[ HEADER_SIZE + nPos++ ] << 8;
		*i += vBuffer[ HEADER_SIZE + nPos++ ];
	}
}
/******************************************************************************/
void TFCPacket::Get(char *i)
/******************************************************************************/
{
	*i = 0;
	if( HEADER_SIZE + nPos + sizeof( char ) <= vBuffer.size() )
	{
		*i = vBuffer[ HEADER_SIZE + nPos++ ];
	}
}
/******************************************************************************/
void TFCPacket::Get(unsigned long *i)
/******************************************************************************/
{
	std::uint32_t v = 0;
	Get( &v );
	*i = static_cast<unsigned long>( v );
}
/******************************************************************************/
void TFCPacket::Get(std::uint32_t *i)
/******************************************************************************/
{
	*i = 0;
	if( HEADER_SIZE + nPos + 4 <= vBuffer.size() )
	{
		*i = ( static_cast<std::uint32_t>( vBuffer[ HEADER_SIZE + nPos++ ] ) << 24U ) |
		     ( static_cast<std::uint32_t>( vBuffer[ HEADER_SIZE + nPos++ ] ) << 16U ) |
		     ( static_cast<std::uint32_t>( vBuffer[ HEADER_SIZE + nPos++ ] ) << 8U ) |
		     static_cast<std::uint32_t>( vBuffer[ HEADER_SIZE + nPos++ ] );
	}
}
/******************************************************************************/
void TFCPacket::Get(unsigned short *i)
/******************************************************************************/
{
	*i = 0;
	if( HEADER_SIZE + nPos + sizeof( short ) <= vBuffer.size() )
	{
		*i =  vBuffer[ HEADER_SIZE + nPos++ ] << 8;
		*i += vBuffer[ HEADER_SIZE + nPos++ ];
	}
}
/******************************************************************************/
void TFCPacket::Get(unsigned char *i)
/******************************************************************************/
{
	*i = 0;
	if( HEADER_SIZE + nPos + sizeof( char ) <= vBuffer.size() )
	{
		*i = vBuffer[ HEADER_SIZE + nPos++ ];
	}
}
/******************************************************************************/
void TFCPacket::Get( string &str )
{
	char buf[ 1024 ];

	WORD strLen = 0;

	Get( (short *)&strLen );

	if (strLen>1024) 
	{
		strLen = 1024; //BLBLBL Juste au cas o�, tronquage de toute chaine soit disant plus longue que le buffer.
	}

	if( HEADER_SIZE + nPos + sizeof( char ) * strLen <= vBuffer.size() )
	{
		int i;
		for( i = 0; i < strLen; i++ )
		{
			Get( (char *)&buf[ i ] );
		}
		buf[ i ] = 0;
		str = buf;
	}
} 
#ifdef _AFXDLL
/******************************************************************************/
// Returns the packet content inside a user-viewable string.
CString TFCPacket::GetDebugPacketString( void )
/******************************************************************************/
{
	CString csTemp;
	CString csFinal;

	unsigned int i;

	// Scroll through packet
	for( i = 0; i < vBuffer.size() - HEADER_SIZE; i++ )
	{
		csTemp.Format( "%u ", vBuffer[ HEADER_SIZE + i ] );
		csFinal += csTemp;
	}

	return csFinal;
}
#endif
/******************************************************************************/
// Sets the buffer for the packet.
BOOL TFCPacket::SetBuffer( LPBYTE lpNewBuffer, int nBufferSize)
/******************************************************************************/
{
	// If given packet buffer is big enough to hold wanted information.
	if( nBufferSize < HEADER_SIZE + sizeof( RQ_SIZE ) )
	{
		return FALSE;
	}
	// Destroy previous packet.
	vBuffer.erase( vBuffer.begin(), vBuffer.end() );

	copy( lpNewBuffer, lpNewBuffer + nBufferSize, back_inserter( vBuffer ) );

    nPos = 0;

    return TRUE;
}
/******************************************************************************/
// Returns the packet's buffer and its size.
void TFCPacket::GetBuffer(LPBYTE &lpNewBuffer, int &nBufferSize)
/******************************************************************************/
{
    lpNewBuffer = &vBuffer.front();
    nBufferSize = vBuffer.size();
}
/******************************************************************************/
// Returns the packet's seed ID
unsigned int TFCPacket::GetPacketSeedID(void)
/******************************************************************************/
{
	return packetSeedID;
}
/******************************************************************************/
// Sets the packet's seed ID
void TFCPacket::SetPacketSeedID( unsigned int newPacketSeedID)
/******************************************************************************/
{
	packetSeedID = newPacketSeedID;
}
/******************************************************************************/
// Returns the packet type.
RQ_SIZE TFCPacket::GetPacketID( void )
/******************************************************************************/
{
	// If there is at least a packetID.
	if( vBuffer.size() - HEADER_SIZE >= sizeof( RQ_SIZE ) )
	{
		int nOldPos = nPos;

		// Go to the beginning of the packet.
		nPos = 0;
		RQ_SIZE rqPacketID = 0;

		// Fetch the packetID.
		Get( (RQ_SIZE *)&rqPacketID );

		// Restore old position.
		nPos = nOldPos;

		return rqPacketID;
	}

	return 0;
}