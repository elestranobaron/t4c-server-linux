/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "WDAFile.h"
#include "../Format.h"
#include "../Random.h"

#include <memory>

using namespace std;

// File-scope functions and data.
namespace
{
/******************************************************************************/
	//  Helper function, returns a random byte.
	inline BYTE GetRandomByte(DWORD dwSeed)
/******************************************************************************/
	{
		// Create a static structure that will be initialized the first time the function is called.
		static struct RandomBuffer
		{
			// Statically create the random buffer at initialization time.
			RandomBuffer()
			{
				DWORD i;
				// Initialize the random buffer.
				Random rnd;
				rnd.SetSeed( 23422 );
				for( i = 0; i < BufferSize; i++ )
				{
					pbRandom[ i ] = rnd( 0, 255 );
				}                
			}

			inline BYTE GetRandomByte( DWORD dwSeed )
			{
				return pbRandom[ dwSeed % BufferSize ];
			}

		private:
			enum{ BufferSize = 3418 };
			BYTE pbRandom[ BufferSize ];
		} cRandomBuffer;

		return cRandomBuffer.GetRandomByte( dwSeed );
	}
}; // namespace
/******************************************************************************/
WDAFileException::WDAFileException
(
 std::string csExplain, // The textual explanation of the exception.
 ErrorType error        // The error code of the exception
) : errorType( error ), csExplanation( csExplain )
/******************************************************************************/
{}
/******************************************************************************/
//  Returns the error type
WDAFileException::ErrorType WDAFileException::GetError( void )
/******************************************************************************/
{
    return errorType;
}
/******************************************************************************/
// Returns the textual explanation of the exception
std::string &WDAFileException::GetExplanation( void )
/******************************************************************************/
{
    return csExplanation;
}
/******************************************************************************/
/******************************************************************************/
WDAFile::WDAFile()
/******************************************************************************/
{
    fFile = NULL;
}
/******************************************************************************/
WDAFile::~WDAFile()
/******************************************************************************/
{
    Close();    
}
/******************************************************************************/
//  Creates a WDAFile, truncates any existing one.
bool WDAFile::Create(string csPath) // The patch where the WDA file should be created
/******************************************************************************/
{
    fopen_s(&fFile, csPath.c_str(), "wb+" );
    return fFile != NULL;
}
/******************************************************************************/
//  Opens a file for reading and writing.
bool WDAFile::Open(string csPath) // The path of the file.
/******************************************************************************/
{
    fopen_s(&fFile, csPath.c_str(), "rb+" );
    if( fFile == NULL )
	{
        return false;
    }
    
    fseek( fFile, 0, SEEK_SET );

    return true;
}
/******************************************************************************/
//  Closes the file. This function is automatically called on destruction.
void WDAFile::Close( void )
/******************************************************************************/
{
    if( fFile != NULL )
	{
        fclose( fFile );
        fFile = NULL;
    }
}
/******************************************************************************/
// Encrypts a byte and returns the encrypted value
BYTE WDAFile::Encrypt
(
 const BYTE bByte, DWORD dwSeed)
/******************************************************************************/
{
    return bByte ^ GetRandomByte( dwSeed );
}
/******************************************************************************/
//  Decrypts the byte and returns the decrypted value
BYTE WDAFile::Decrypt(const BYTE bByte, DWORD dwSeed)
/******************************************************************************/
{
    return bByte ^ GetRandomByte( dwSeed );
}
/******************************************************************************/
//  Writes a buffer to the file
void WDAFile::Write
(
 const void *lpBuffer, // The buffer to write.
 DWORD dwSize    // The size of the buffer to write.
)
/******************************************************************************/
{
    // Buffer is NULL ? You cannot pass NULL to this function.
    ASSERT( lpBuffer != NULL );

    const BYTE *pbBuffer = reinterpret_cast< const BYTE * >( lpBuffer );
    
    // Write the buffer to the file.
    DWORD i;
    DWORD dwOffset = ftell( fFile );
    // We need to do a character-by-character write for stream encryption.
    for( i = 0; i < dwSize; i++ )
	{
        // If fputc fails
        if( fputc( Encrypt( pbBuffer[ i ], i + dwOffset ), fFile ) == EOF )
		{

            // Writing failed!
            ASSERT( false );

            // Throw a WriteError exception.
            static WDAFileException ex( "Error writting to file", WDAFileException::WriteError );
            throw( ex );
        }
    }
}
/******************************************************************************/
//  Writes a DWORD to the file.
void WDAFile::Write(DWORD dwDword) // The DWORD to write.
/******************************************************************************/
{
    // Write the DWORD's buffer.
    Write( &dwDword, sizeof( DWORD ) );
}
/******************************************************************************/
//  Writes a WORD to the file.
void WDAFile::Write(WORD wWord) // The WORD to write.
/******************************************************************************/
{
    // Write the WORD's buffer.
    Write( &wWord, sizeof( WORD ) );
}
/******************************************************************************/
//  Writes a string to the file.
void WDAFile::Write(const std::string &csString) // Writes a string to the file.
/******************************************************************************/
{
    // Write the string's size.    
    Write( static_cast< DWORD >( csString.size() ) );
    
    // Write the string.
    Write( csString.data(), csString.size() );
}
/******************************************************************************/
void WDAFile::Write(BYTE bByte) // Writes a byte to the file.
/******************************************************************************/
{
    // Write the byte
    Write( &bByte, sizeof( BYTE ) );
}
/******************************************************************************/
//  Writes a boolean to the file.
void WDAFile::Write(bool boBool) // The boolean to write.
/******************************************************************************/
{
    // Copy the bool into a BYTE.
    BYTE bByte = boBool ? 1 : 0;

    // Write that byte. Can only be 1 or 0.
    Write( &bByte, sizeof( BYTE ) );
}
/******************************************************************************/
// Writes to a signed int.
void WDAFile::Write(signed int nInt) // The int.
/******************************************************************************/
{
    Write( &nInt, sizeof( signed int ) );
}
/******************************************************************************/
// Writes a long to the wdaFile
void WDAFile::Write(signed long lLong) // The long
/******************************************************************************/
{
    Write( &lLong, sizeof( signed long ) );
}
/******************************************************************************/
// Writes a char to the wdaFile
void WDAFile::Write(signed char cChar) // The signed char.
/******************************************************************************/
{
    Write( &cChar, sizeof( signed char ) );
}
/******************************************************************************/
//  Write a short.
void WDAFile::Write(signed short sShort) // The short.
/******************************************************************************/
{
    Write( &sShort, sizeof( signed short ) );
}
/******************************************************************************/
//  Writes a double to a wdaFile
void WDAFile::Write(double dblDouble) // The wdaFile
/******************************************************************************/
{
    Write( &dblDouble, sizeof( double ) );
}
/******************************************************************************/
//  Reads from a buffer.
void WDAFile::Read 
(
 void *lpBuffer, // The provided buffer, must at least be dwSize bytes long!
 DWORD dwSize,    // The size of the buffer.
 const char *szExceptionText // Text for the end-of-file exception.
)
/******************************************************************************/
{
    BYTE *pbBuffer = static_cast< BYTE * >( lpBuffer );
    
    DWORD i;
    int ch;
    DWORD dwOffset = ftell( fFile );

    // Do a character-by-character read for stream decryption.
    for( i = 0; i < dwSize; i++ )
	{
        ch = fgetc( fFile );
        
        if( ch == EOF )
		{
            TFormat cFormat;

            static WDAFileException ex( 
                cFormat(
                    "End-of-File reached reading from %s.",
                    szExceptionText
                ), 
                WDAFileException::EndOfFile
            );
            throw( ex );
        }

        // Decrypt character.
        ch = Decrypt( ch, i + dwOffset );

        // Append character to buffer.
        pbBuffer[ i ] = static_cast< BYTE >( ch );
    }
}
/******************************************************************************/
// Reads a DWORD from the wdaFile
void WDAFile::Read(DWORD &dwDword) // The DWORD in which to put the value.
/******************************************************************************/
{
    Read( &dwDword, sizeof( DWORD ), "a DWORD" );
}
/******************************************************************************/
// Reads a WORD from the WDAFile
void WDAFile::Read(WORD  &wWord) // The WORD in which to put the value.
/******************************************************************************/
{
    Read( &wWord, sizeof( WORD ), "a WORD" );
}
/******************************************************************************/
// Reads a string from the WDAFile
void WDAFile::Read(std::string &csString) // The string.
/******************************************************************************/
{
    DWORD dwStringSize = 0xABCD;

    // Read the string size.
    Read( dwStringSize );

    // The string size wasn't fetched!
    ASSERT( dwStringSize != 0xABCD );
    
    // Create a new buffer to hold the string.
    std::unique_ptr<char[]> apBuffer(new char[dwStringSize + 1]);

    // Read the string from the file.
    Read( apBuffer.get(), dwStringSize, "a string" );

    // NULL terminate the string.
    apBuffer.get()[ dwStringSize ] = 0;

    // Assign this buffer to the string.
    csString = const_cast< const char * >( apBuffer.get() );
}
/******************************************************************************/
// Reads a byte from the wdaFile
void WDAFile::Read(BYTE &bByte) // The byte to put the result in.
/******************************************************************************/
{
    Read( &bByte, sizeof( bByte ), "a BYTE" );
}
/******************************************************************************/
// Reads a bool from the wdaFile.
void WDAFile::Read(bool &boBool) // The bool to put the result in.
/******************************************************************************/
{
    // Grab a byte to fetch the result from the WDAFile
    BYTE bByte;

    Read( &bByte, sizeof( BYTE ), "a bool" );

    // A bool can only be 0 or 1! The file must be corrupted or the database was not read properly.
    ASSERT( bByte == 0 || bByte == 1 );

    boBool = bByte == 1 ? true : false;
}
/******************************************************************************/
//  Reads a signed int,
void WDAFile::Read (signed int &nInt) // The container signed int
/******************************************************************************/
{
    Read( &nInt, sizeof( signed int ), "an integer" );
}
/******************************************************************************/
//  Reads from a signed long
void WDAFile::Read(signed long &lLong) // The long
/******************************************************************************/
{
    Read( &lLong, sizeof( signed long ), "a long" );
}
/******************************************************************************/
//  Reads from a signed char.
void WDAFile::Read(signed char &cChar) // The signed char.
/******************************************************************************/
{
    Read( &cChar, sizeof( signed char ), "a char" );
}
/******************************************************************************/
//  Reads the short.
void WDAFile::Read(signed short &sShort) // The short.
/******************************************************************************/
{
    Read( &sShort, sizeof( signed short ), "a short" );
}
/******************************************************************************/
//  Read a double.
void WDAFile::Read(double &dblDouble) // The double
/******************************************************************************/
{
    Read( &dblDouble, sizeof( double ), "a double" );
}
/******************************************************************************/
void WDAFile::Seek( long pos )
/******************************************************************************/
{
    if( fFile != NULL ){
        fseek( fFile, pos, SEEK_SET );
    }
}