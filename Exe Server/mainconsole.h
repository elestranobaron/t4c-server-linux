/******************************************************************************
Modify for vs2008 (03/05/2009)
/******************************************************************************/
#if !defined(AFX_MAINCONSOLE_H__056C8101_0631_11D3_84FA_00E02922FA40__INCLUDED_)
#define AFX_MAINCONSOLE_H__056C8101_0631_11D3_84FA_00E02922FA40__INCLUDED_

#if _MSC_VER >= 1000
	#pragma once
#endif // _MSC_VER >= 1000

#include <string>

/******************************************************************************/
class MainConsole  
/******************************************************************************/
{
public:	
	virtual ~MainConsole();
    static MainConsole &GetInstance();
    void TakeControl( void );
    void Terminate( void );

private:
    MainConsole();
    static unsigned int PipeThread( void *lpInstance );
};

#ifdef _WIN32

/******************************************************************************/
class AsyncNamedPipeServer
/******************************************************************************/
{
public:
	AsyncNamedPipeServer( std::string bsThePipeName );
	~AsyncNamedPipeServer();

    enum PipeStatus
	{
        StatusRead,
        StatusWrite,
        StatusConnect,
        StatusIoPending
    };

    bool Create( void );
    void Destroy( void );
    void Reset( void )
	{
        Disconnect();
        Destroy();
        Create();
    }

    PipeStatus GetPendingIo( void );

    bool Read      ( void );
    void Write     ( void );
    void Connect   ( void );
    void Disconnect( void );

    void   SetMessage( std::string bsMessage );
    std::string GetMessage( void );

private:

    std::string      bsPipeName;
    char        lpBuffer[ 4096 ];
    HANDLE      hPipe;
    OVERLAPPED  overLapped;
    PipeStatus  pendingPipeStatus;    
};

#else

class AsyncNamedPipeServer {
public:
	explicit AsyncNamedPipeServer(std::string bsThePipeName) { (void)bsThePipeName; }
	~AsyncNamedPipeServer() {}

	enum PipeStatus {
		StatusRead,
		StatusWrite,
		StatusConnect,
		StatusIoPending
	};

	bool Create(void) { return false; }
	void Destroy(void) {}
	void Reset(void) {}
	PipeStatus GetPendingIo(void) { return StatusIoPending; }
	bool Read(void) { return false; }
	void Write(void) {}
	void Connect(void) {}
	void Disconnect(void) {}
	void SetMessage(std::string bsMessage) { (void)bsMessage; }
	std::string GetMessage(void) { return std::string(); }
};

#endif /* _WIN32 */

#endif // !defined(AFX_MAINCONSOLE_H__056C8101_0631_11D3_84FA_00E02922FA40__INCLUDED_)
