#ifndef T4C_WIN32_COMPAT_H
#define T4C_WIN32_COMPAT_H

#ifndef _WIN32

#include "StandardTypes.h"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <chrono>
#include <thread>
#include <pthread.h>
#include <cstdlib>
#include <cerrno>
#include <cstdio>
#include <filesystem>
#include <unistd.h>
#include <queue>
#include <mutex>
#include <condition_variable>

#ifndef _snprintf
#define _snprintf std::snprintf
#endif

typedef void *HANDLE;
typedef struct tagPOINT {
    LONG x;
    LONG y;
} POINT;
typedef unsigned int UINT;
typedef void *LPOVERLAPPED;
#ifndef CALLBACK
#define CALLBACK
#endif

#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE ((HANDLE)(intptr_t)-1)
#endif

inline DWORD GetCurrentThreadId() {
    return static_cast<DWORD>(reinterpret_cast<std::uintptr_t>(pthread_self()));
}

inline void Sleep(DWORD ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

#ifndef INFINITE
#define INFINITE 0xFFFFFFFFu
#endif
#ifndef WAIT_OBJECT_0
#define WAIT_OBJECT_0 0u
#endif
#ifndef WAIT_TIMEOUT
#define WAIT_TIMEOUT 258u
#endif
#ifndef THREAD_PRIORITY_HIGHEST
#define THREAD_PRIORITY_HIGHEST 2
#endif
#ifndef THREAD_PRIORITY_ABOVE_NORMAL
#define THREAD_PRIORITY_ABOVE_NORMAL 1
#endif
#ifndef THREAD_PRIORITY_BELOW_NORMAL
#define THREAD_PRIORITY_BELOW_NORMAL (-1)
#endif
#ifndef PAGE_READWRITE
#define PAGE_READWRITE 0
#endif
#ifndef FILE_MAP_WRITE
#define FILE_MAP_WRITE 0
#endif
#ifndef CONTEXT_FULL
#define CONTEXT_FULL 0
#endif
#ifndef WSAECONNABORTED
#define WSAECONNABORTED ECONNABORTED
#endif
#ifndef WSAEHOSTUNREACH
#define WSAEHOSTUNREACH EHOSTUNREACH
#endif
#ifndef WSAECONNRESET
#define WSAECONNRESET ECONNRESET
#endif
#ifndef WSAEADDRNOTAVAIL
#define WSAEADDRNOTAVAIL EADDRNOTAVAIL
#endif
#ifndef WSAEAFNOSUPPORT
#define WSAEAFNOSUPPORT EAFNOSUPPORT
#endif
#ifndef WSAETIMEDOUT
#define WSAETIMEDOUT ETIMEDOUT
#endif
#ifndef WSAENETUNREACH
#define WSAENETUNREACH ENETUNREACH
#endif

#define _T(x) (x)
typedef char TCHAR;
typedef TCHAR *LPTSTR;
typedef const TCHAR *LPCTSTR;

inline HANDLE GetCurrentThread() {
    return reinterpret_cast<HANDLE>(std::uintptr_t{2});
}

inline void *GetModuleHandle(LPCSTR lpModuleName) {
    (void)lpModuleName;
    return nullptr;
}

inline DWORD GetModuleFileName(void *hModule, TCHAR *path, DWORD sizeChars) {
    (void)hModule;
    if (!path || sizeChars == 0) {
        return 0;
    }
    const ssize_t n = ::readlink("/proc/self/exe", path, static_cast<size_t>(sizeChars - 1));
    if (n <= 0) {
        path[0] = 0;
        return 0;
    }
    path[static_cast<size_t>(n)] = 0;
    return static_cast<DWORD>(n);
}

typedef char CHAR;
typedef void *HKEY;
#define HKEY_LOCAL_MACHINE ((HKEY)(std::uintptr_t)0x80000002)

typedef struct _SYSTEMTIME {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
} SYSTEMTIME;

typedef struct _EXCEPTION_RECORD {
    DWORD ExceptionCode;
    DWORD ExceptionFlags;
    struct _EXCEPTION_RECORD *ExceptionRecord;
    void *ExceptionAddress;
    DWORD NumberParameters;
    std::uintptr_t ExceptionInformation[15];
} EXCEPTION_RECORD;

typedef struct _CONTEXT {
    DWORD ContextFlags;
    std::uintptr_t Eip;
} CONTEXT;

typedef struct _EXCEPTION_POINTERS {
    EXCEPTION_RECORD *ExceptionRecord;
    CONTEXT *ContextRecord;
} EXCEPTION_POINTERS;
typedef EXCEPTION_POINTERS *LPEXCEPTION_POINTERS;

typedef CONTEXT *LPCONTEXT;
#ifndef EXCEPTION_CONTINUE_SEARCH
#define EXCEPTION_CONTINUE_SEARCH 0
#endif
#ifndef EXCEPTION_CONTINUE_EXECUTION
#define EXCEPTION_CONTINUE_EXECUTION (-1)
#endif
#ifndef EXCEPTION_EXECUTE_HANDLER
#define EXCEPTION_EXECUTE_HANDLER 1
#endif
typedef LONG (*LPTOP_LEVEL_EXCEPTION_FILTER)(EXCEPTION_POINTERS *);

#ifndef MB_OK
#define MB_OK 0
#endif

inline int MessageBox(void *hwnd, LPCTSTR text, LPCTSTR caption, UINT type) {
    (void)hwnd;
    (void)caption;
    (void)type;
    if (text) {
        std::fputs(text, stderr);
    }
    std::fputc('\n', stderr);
    return 0;
}

inline BOOL CreateDirectory(LPCTSTR path, void *) {
    if (!path || !*path) {
        return FALSE;
    }
    std::error_code ec;
    std::filesystem::create_directories(path, ec);
    return ec ? FALSE : TRUE;
}

inline void GetLocalTime(SYSTEMTIME *st) {
    if (!st) {
        return;
    }
    std::memset(st, 0, sizeof(*st));
    std::time_t t = std::time(nullptr);
    std::tm *loc = std::localtime(&t);
    if (!loc) {
        return;
    }
    st->wYear = static_cast<WORD>(loc->tm_year + 1900);
    st->wMonth = static_cast<WORD>(loc->tm_mon + 1);
    st->wDayOfWeek = static_cast<WORD>(loc->tm_wday);
    st->wDay = static_cast<WORD>(loc->tm_mday);
    st->wHour = static_cast<WORD>(loc->tm_hour);
    st->wMinute = static_cast<WORD>(loc->tm_min);
    st->wSecond = static_cast<WORD>(loc->tm_sec);
}

inline void ZeroMemory(void *p, std::size_t n) {
    std::memset(p, 0, n);
}

inline void OutputDebugString(const char *lpOutputString) {
    if (lpOutputString && *lpOutputString) {
        std::fputs(lpOutputString, stderr);
    }
}

inline DWORD GetLastError(void) {
    return static_cast<DWORD>(errno);
}

inline BOOL MessageBeep(UINT uType) {
    (void)uType;
    return TRUE;
}

inline HANDLE CreateEvent(void *, BOOL manualReset, BOOL initialState, LPCSTR) {
    (void)manualReset;
    (void)initialState;
    return reinterpret_cast<HANDLE>(std::uintptr_t{1});
}

inline BOOL CloseHandle(HANDLE h) {
    (void)h;
    return TRUE;
}

inline DWORD WaitForSingleObject(HANDLE h, DWORD ms) {
    (void)h;
    (void)ms;
    return WAIT_OBJECT_0;
}

inline BOOL SetEvent(HANDLE h) {
    (void)h;
    return TRUE;
}

inline BOOL ResetEvent(HANDLE h) {
    (void)h;
    return TRUE;
}

struct CRITICAL_SECTION {
    std::recursive_mutex mtx;
};
typedef CRITICAL_SECTION *LPCRITICAL_SECTION;

inline void InitializeCriticalSection(CRITICAL_SECTION *cs) {
    (void)cs;
}
inline void DeleteCriticalSection(CRITICAL_SECTION *cs) {
    (void)cs;
}
inline void EnterCriticalSection(CRITICAL_SECTION *cs) {
    if (cs) {
        cs->mtx.lock();
    }
}
inline void LeaveCriticalSection(CRITICAL_SECTION *cs) {
    if (cs) {
        cs->mtx.unlock();
    }
}

#ifndef CREATE_SUSPENDED
#define CREATE_SUSPENDED 0x00000004
#endif
/*
inline HANDLE CreateIoCompletionPort(HANDLE fileHandle, HANDLE existingCompletionPort, DWORD completionKey, DWORD numberOfConcurrentThreads) {
    (void)fileHandle;
    (void)existingCompletionPort;
    (void)completionKey;
    (void)numberOfConcurrentThreads;
    return reinterpret_cast<HANDLE>(std::uintptr_t{1});
}

inline BOOL PostQueuedCompletionStatus(HANDLE completionPort, DWORD numberOfBytesTransferred, std::uintptr_t completionKey, LPOVERLAPPED overlapped) {
    (void)completionPort;
    (void)numberOfBytesTransferred;
    (void)completionKey;
    (void)overlapped;
    return TRUE;
}

inline BOOL GetQueuedCompletionStatus(HANDLE completionPort, DWORD *numberOfBytesTransferred, std::uintptr_t *completionKey, LPOVERLAPPED *overlapped, DWORD milliseconds) {
    (void)completionPort;
    (void)overlapped;
    (void)milliseconds;
    if (numberOfBytesTransferred) {
        *numberOfBytesTransferred = 1;
    }
    if (completionKey) {
        *completionKey = 0;
    }
    return TRUE;
}

inline BOOL GetQueuedCompletionStatus(HANDLE completionPort, DWORD *numberOfBytesTransferred, DWORD *completionKey, LPOVERLAPPED *overlapped, DWORD milliseconds) {
    std::uintptr_t key = 0;
    const BOOL result = GetQueuedCompletionStatus(completionPort, numberOfBytesTransferred, &key, overlapped, milliseconds);
    if (completionKey) {
        *completionKey = static_cast<DWORD>(key);
    }
    return result;
}
*/

struct IoCompletionPort {
    std::queue<std::pair<std::uintptr_t, DWORD>> items;
    std::mutex mtx;
    std::condition_variable cv;
};

inline HANDLE CreateIoCompletionPort(HANDLE fileHandle, HANDLE existingCompletionPort, DWORD completionKey, DWORD numberOfConcurrentThreads) {
    (void)fileHandle; (void)completionKey; (void)numberOfConcurrentThreads;
    if (existingCompletionPort && existingCompletionPort != INVALID_HANDLE_VALUE)
        return existingCompletionPort;
    return reinterpret_cast<HANDLE>(new IoCompletionPort());
}

inline BOOL PostQueuedCompletionStatus(HANDLE completionPort, DWORD numberOfBytesTransferred, std::uintptr_t completionKey, LPOVERLAPPED overlapped) {
    (void)overlapped; (void)numberOfBytesTransferred;
    auto *port = reinterpret_cast<IoCompletionPort*>(completionPort);
    if (!port) return FALSE;
    std::lock_guard<std::mutex> lock(port->mtx);
    port->items.push({completionKey, numberOfBytesTransferred});
    port->cv.notify_one();
    return TRUE;
}

inline BOOL GetQueuedCompletionStatus(HANDLE completionPort, DWORD *numberOfBytesTransferred, std::uintptr_t *completionKey, LPOVERLAPPED *overlapped, DWORD milliseconds) {
    (void)overlapped;
    auto *port = reinterpret_cast<IoCompletionPort*>(completionPort);
    if (!port) return FALSE;
    std::unique_lock<std::mutex> lock(port->mtx);
    if (milliseconds == INFINITE) {
        port->cv.wait(lock, [port]{ return !port->items.empty(); });
    } else {
        if (!port->cv.wait_for(lock, std::chrono::milliseconds(milliseconds),
                               [port]{ return !port->items.empty(); }))
            return FALSE;
    }
    auto [key, bytes] = port->items.front();
    port->items.pop();
    if (completionKey) *completionKey = key;
    if (numberOfBytesTransferred) *numberOfBytesTransferred = bytes;
    return TRUE;
}

inline BOOL GetQueuedCompletionStatus(HANDLE completionPort, DWORD *numberOfBytesTransferred, DWORD *completionKey, LPOVERLAPPED *overlapped, DWORD milliseconds) {
    std::uintptr_t key = 0;
    const BOOL result = GetQueuedCompletionStatus(completionPort, numberOfBytesTransferred, &key, overlapped, milliseconds);
    if (completionKey) *completionKey = static_cast<DWORD>(key);
    return result;
}

inline DWORD timeGetTime() {
    using clock = std::chrono::steady_clock;
    static const auto start = clock::now();
    return static_cast<DWORD>(
        std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - start).count());
}

inline unsigned long _beginthread(void (*start_address)(void *), unsigned stack_size, void *arglist) {
    (void)stack_size;
    if (start_address) {
        std::thread(start_address, arglist).detach();
    }
    return 1UL;
}

inline unsigned long _beginthreadex(void *security, unsigned stack_size, unsigned int (*start_address)(void *), void *arglist, unsigned initflag, unsigned *thrdaddr) {
    (void)security;
    (void)stack_size;
    (void)initflag;
    if (thrdaddr) {
        *thrdaddr = 1;
    }
    if (start_address) {
        std::thread(start_address, arglist).detach();
    }
    return 1UL;
}

inline DWORD ResumeThread(HANDLE hThread) {
    (void)hThread;
    return 0;
}

inline DWORD SuspendThread(HANDLE hThread) {
    (void)hThread;
    return 0;
}

inline BOOL SetThreadPriority(HANDLE hThread, int nPriority) {
    (void)hThread;
    (void)nPriority;
    return TRUE;
}

inline BOOL TerminateThread(HANDLE hThread, DWORD dwExitCode) {
    (void)hThread;
    (void)dwExitCode;
    return TRUE;
}

inline BOOL CancelIo(HANDLE hFile) {
    (void)hFile;
    return TRUE;
}

inline HANDLE CreateMutex(void *mutexAttributes, BOOL initialOwner, LPCSTR name) {
    (void)mutexAttributes;
    (void)initialOwner;
    (void)name;
    return reinterpret_cast<HANDLE>(std::uintptr_t{1});
}

inline HANDLE CreateFileMapping(HANDLE fileHandle, void *attributes, DWORD protect, DWORD maxSizeHigh, DWORD maxSizeLow, LPCSTR name) {
    (void)fileHandle;
    (void)attributes;
    (void)protect;
    (void)maxSizeHigh;
    (void)maxSizeLow;
    (void)name;
    return reinterpret_cast<HANDLE>(std::uintptr_t{1});
}

inline void *MapViewOfFile(HANDLE fileMappingObject, DWORD desiredAccess, DWORD fileOffsetHigh, DWORD fileOffsetLow, std::size_t numberOfBytesToMap) {
    (void)fileMappingObject;
    (void)desiredAccess;
    (void)fileOffsetHigh;
    (void)fileOffsetLow;
    return std::malloc(numberOfBytesToMap ? numberOfBytesToMap : 1u);
}

inline BOOL UnmapViewOfFile(const void *baseAddress) {
    std::free(const_cast<void *>(baseAddress));
    return TRUE;
}

inline BOOL ReleaseMutex(HANDLE hMutex) {
    (void)hMutex;
    return TRUE;
}

inline DWORD GetCurrentProcessId() {
    return 1;
}

inline HANDLE GetCurrentProcess() {
    return reinterpret_cast<HANDLE>(std::uintptr_t{3});
}

inline BOOL TerminateProcess(HANDLE hProcess, UINT uExitCode) {
    (void)hProcess;
    std::exit(static_cast<int>(uExitCode));
    return TRUE;
}

inline BOOL GetExitCodeThread(HANDLE hThread, DWORD *lpExitCode) {
    (void)hThread;
    if (lpExitCode) {
        *lpExitCode = 0;
    }
    return TRUE;
}

inline BOOL GetThreadContext(HANDLE hThread, CONTEXT *context) {
    (void)hThread;
    if (context) {
        context->ContextFlags = 0;
    }
    return TRUE;
}

inline void CoInitialize(void *reserved) {
    (void)reserved;
}

inline void CoUninitialize() {}

#include <dlfcn.h>
#ifndef HINSTANCE
typedef void *HINSTANCE;
#endif
inline BOOL FreeLibrary(HINSTANCE hLibModule) {
    return dlclose(hLibModule) == 0 ? TRUE : FALSE;
}

inline HINSTANCE LoadLibrary(LPCSTR lpLibFileName) {
    if (lpLibFileName == nullptr) {
        return nullptr;
    }
    return dlopen(lpLibFileName, RTLD_NOW);
}

#endif /* !_WIN32 */

#endif /* T4C_WIN32_COMPAT_H */
