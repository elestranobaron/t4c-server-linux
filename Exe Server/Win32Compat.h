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
#include <cstdarg>
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

inline DWORD GetTickCount() {
    using clock = std::chrono::steady_clock;
    static const auto start = clock::now();
    return static_cast<DWORD>(
        std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - start).count());
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

#include <netinet/in.h>
#define NM_IPV4_ADDR(inaddr) ((inaddr).s_addr)

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
    if (h == INVALID_HANDLE_VALUE || h == NULL) {
        return TRUE;
    }
    const std::uintptr_t hp = reinterpret_cast<std::uintptr_t>(h);
    if (hp <= 0x100) {
        return TRUE;
    }
    FILE *fp = reinterpret_cast<FILE *>(h);
    if (fp) {
        return std::fclose(fp) == 0 ? TRUE : FALSE;
    }
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

// NOTE: a Win32 CRITICAL_SECTION is recursive (re-entrant by the owning
// thread). Several code paths rely on that (e.g. a thread holding the player
// list writer lock then re-reading the list via EnterReader during broadcast/
// cleanup). Emulating it with a non-recursive std::mutex caused a self-deadlock
// that froze the UDP receive thread. Use a recursive_mutex to match Win32.
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

#ifndef fopen_s
inline int fopen_s(FILE **fp, const char *filename, const char *mode) {
    if (!fp) {
        return EINVAL;
    }
    *fp = std::fopen(filename, mode);
    return (*fp != nullptr) ? 0 : errno;
}
#endif
#ifndef LOBYTE
#define LOBYTE(w) ((BYTE)((w) & 0xff))
#endif
#ifndef HIBYTE
#define HIBYTE(w) ((BYTE)(((WORD)(w) >> 8) & 0xff))
#endif
#ifndef LOWORD
#define LOWORD(l) ((WORD)((DWORD)(l) & 0xffff))
#endif
#ifndef HIWORD
#define HIWORD(l) ((WORD)(((DWORD)(l) >> 16) & 0xffff))
#endif
#ifndef GENERIC_READ
#define GENERIC_READ 0x80000000u
#endif
#ifndef FILE_SHARE_READ
#define FILE_SHARE_READ 0x00000001u
#endif
#ifndef OPEN_EXISTING
#define OPEN_EXISTING 3u
#endif
inline HANDLE CreateFile(LPCTSTR lpFileName, DWORD, DWORD, void *, DWORD, DWORD, HANDLE) {
    if (!lpFileName) {
        return INVALID_HANDLE_VALUE;
    }
    FILE *fp = std::fopen(lpFileName, "rb");
    return fp ? reinterpret_cast<HANDLE>(fp) : INVALID_HANDLE_VALUE;
}
inline DWORD GetFileSize(HANDLE hFile, DWORD *) {
    FILE *fp = reinterpret_cast<FILE *>(hFile);
    if (!fp) {
        return 0;
    }
    std::fseek(fp, 0, SEEK_END);
    const long sz = std::ftell(fp);
    std::fseek(fp, 0, SEEK_SET);
    return sz >= 0 ? static_cast<DWORD>(sz) : 0;
}
inline BOOL ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nBytes, DWORD *lpRead, void *) {
    FILE *fp = reinterpret_cast<FILE *>(hFile);
    if (!fp || !lpBuffer) {
        return FALSE;
    }
    const size_t n = std::fread(lpBuffer, 1, nBytes, fp);
    if (lpRead) {
        *lpRead = static_cast<DWORD>(n);
    }
    return n > 0 ? TRUE : FALSE;
}
#ifndef _MAX_PATH
#define _MAX_PATH 260
#endif
#ifndef MAX_PATH
#define MAX_PATH _MAX_PATH
#endif
#ifndef CP_ACP
#define CP_ACP 0
#endif
inline int WideCharToMultiByte(unsigned int, unsigned long, const wchar_t *ws, int, char *s, int cb, const char *, const char *) {
    if (!ws || !s || cb <= 0) {
        return 0;
    }
    int i = 0;
    for (; ws[i] && i + 1 < cb; ++i) {
        s[i] = static_cast<char>(ws[i] & 0xFF);
    }
    s[i] = '\0';
    return i;
}
inline int _vscprintf(const char *fmt, va_list ap) {
    if (!fmt) {
        return 0;
    }
    va_list copy;
    va_copy(copy, ap);
    const int n = std::vsnprintf(nullptr, 0, fmt, copy);
    va_end(copy);
    return n < 0 ? 0 : n;
}
#ifndef vsprintf_s
#define vsprintf_s(buf, sz, fmt, ap) std::vsnprintf((buf), static_cast<size_t>(sz), (fmt), (ap))
#endif
#ifndef strtok_s
inline char *strtok_s(char *str, const char *delim, char **ctx) {
    return ::strtok_r(str, delim, ctx);
}
#endif
#ifndef __noop
#define __noop nullptr
#endif
#ifndef _i64toa_s
#define _i64toa_s(val, buf, sz, radix) \
    do { \
        if ((buf) && (sz) > 0) { \
            if ((radix) == 10) { \
                std::snprintf((buf), static_cast<size_t>(sz), "%lld", static_cast<long long>(val)); \
            } else { \
                (buf)[0] = '\0'; \
            } \
        } \
    } while (0)
#endif
#ifndef fprintf_s
#define fprintf_s(stream, ...) std::fprintf((stream), __VA_ARGS__)
#endif

#include <cstdarg>

#ifndef sscanf_s
#define sscanf_s sscanf
#endif
#ifndef _strnicmp
#define _strnicmp strncasecmp
#endif

inline int sprintf_s(char *buffer, std::size_t sizeOfBuffer, const char *format, ...) {
    va_list args;
    va_start(args, format);
    const int result = std::vsnprintf(buffer, sizeOfBuffer, format, args);
    va_end(args);
    return result;
}

inline int sprintf_s(char *buffer, const char *format, ...) {
    va_list args;
    va_start(args, format);
    const int result = std::vsnprintf(buffer, 4096, format, args);
    va_end(args);
    return result;
}

inline int ctime_s(char *buffer, std::size_t sizeOfBuffer, const std::time_t *timer) {
    if (!buffer || sizeOfBuffer == 0) {
        return EINVAL;
    }
    char *const text = std::ctime(const_cast<std::time_t *>(timer));
    if (!text) {
        buffer[0] = '\0';
        return EINVAL;
    }
    std::strncpy(buffer, text, sizeOfBuffer - 1);
    buffer[sizeOfBuffer - 1] = '\0';
    return 0;
}

inline BOOL GetProcessAffinityMask(HANDLE, DWORD *processMask, DWORD *systemMask) {
    if (processMask) {
        *processMask = 0xFFFFFFFFu;
    }
    if (systemMask) {
        *systemMask = 0xFFFFFFFFu;
    }
    return TRUE;
}

#ifndef EVENTLOG_ERROR_TYPE
#define EVENTLOG_ERROR_TYPE 1
#define EVENTLOG_WARNING_TYPE 2
#define EVENTLOG_INFORMATION_TYPE 4
#endif
#ifndef vsprintf_s
#define vsprintf_s(buf, sz, fmt, ap) std::vsnprintf((buf), static_cast<size_t>(sz), (fmt), (ap))
#endif
#ifndef _vsntprintf_s
#define _vsntprintf_s(buf, sizeOfBuffer, count, fmt, ap) \
    std::vsnprintf((buf), static_cast<size_t>((count) < (sizeOfBuffer) ? (count) : (sizeOfBuffer)), (fmt), (ap))
#endif
#ifndef strcat_s
#define strcat_s(dst, sz, src) \
    do { \
        if ((dst) && (sz) > 0 && (src)) { \
            const size_t _used = std::strlen(dst); \
            const size_t _cap = static_cast<size_t>(sz); \
            if (_used + 1 < _cap) { \
                std::strncat((dst), (src), _cap - _used - 1); \
            } \
        } \
    } while (0)
#endif

#ifndef MOVEFILE_REPLACE_EXISTING
#define MOVEFILE_REPLACE_EXISTING 0x00000001u
#endif
#ifndef PROCESS_SET_INFORMATION
#define PROCESS_SET_INFORMATION 0x0200u
#endif
#ifndef SEE_MASK_DOENVSUBST
#define SEE_MASK_DOENVSUBST 0x00000200u
#endif
#ifndef SEE_MASK_NOCLOSEPROCESS
#define SEE_MASK_NOCLOSEPROCESS 0x00000040u
#endif
#ifndef SW_SHOWNORMAL
#define SW_SHOWNORMAL 1
#endif
#ifndef SE_ERR_DDEFAIL
#define SE_ERR_DDEFAIL 32
#endif

typedef void *HWND;

typedef struct _SHELLEXECUTEINFOA {
    DWORD cbSize;
    unsigned long fMask;
    HWND hwnd;
    LPCSTR lpVerb;
    LPCSTR lpFile;
    LPCSTR lpParameters;
    LPCSTR lpDirectory;
    int nShow;
    HINSTANCE hInstApp;
    void *lpIDList;
    LPCSTR lpClass;
    void *hkeyClass;
    DWORD dwHotKey;
    HANDLE hMonitor;
    HANDLE hProcess;
} SHELLEXECUTEINFO, *LPSHELLEXECUTEINFO;

inline BOOL ShellExecuteEx(SHELLEXECUTEINFO *pExecInfo) {
    if (!pExecInfo || !pExecInfo->lpFile) {
        return FALSE;
    }
    std::string cmd = "sh \"";
    cmd += pExecInfo->lpFile;
    cmd += "\"";
    pExecInfo->hProcess = NULL;
    const int rc = std::system(cmd.c_str());
    pExecInfo->hInstApp = (rc == 0) ? reinterpret_cast<HINSTANCE>(33) : reinterpret_cast<HINSTANCE>(SE_ERR_DDEFAIL);
    return rc == 0 ? TRUE : FALSE;
}

inline BOOL DeleteFile(LPCTSTR path) {
    return (path && std::remove(path) == 0) ? TRUE : FALSE;
}

inline BOOL MoveFileEx(LPCTSTR src, LPCTSTR dst, DWORD flags) {
    if (!src || !dst) {
        return FALSE;
    }
    if (flags & MOVEFILE_REPLACE_EXISTING) {
        std::remove(dst);
    }
    return std::rename(src, dst) == 0 ? TRUE : FALSE;
}

inline HANDLE OpenProcess(DWORD, BOOL, DWORD) {
    return reinterpret_cast<HANDLE>(std::uintptr_t{2});
}

inline BOOL SetProcessAffinityMask(HANDLE hProcess, DWORD mask) {
    (void)hProcess;
    (void)mask;
    return TRUE;
}

#include <cassert>
#ifndef ASSERT
#define ASSERT(expr) assert(expr)
#endif
#ifndef ATLASSERT
#define ATLASSERT(expr) ASSERT(expr)
#endif

#ifndef ERROR_INVALID_PARAMETER
#define ERROR_INVALID_PARAMETER 87u
#endif
#ifndef ERROR_INSUFFICIENT_BUFFER
#define ERROR_INSUFFICIENT_BUFFER 122u
#endif

inline void SetLastError(DWORD err) {
    errno = static_cast<int>(err);
}

#endif /* !_WIN32 */

#endif /* T4C_WIN32_COMPAT_H */
