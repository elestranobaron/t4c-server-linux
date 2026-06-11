#ifndef T4C_STANDARD_TYPES_H
#define T4C_STANDARD_TYPES_H

#include <cstdint>
#include <cstring>

// Fixed-width replacements for legacy Win32 integral aliases.
using BYTE = std::uint8_t;
using WORD = std::uint16_t;
using DWORD = std::uint32_t;
using INT = int;
using LPBOOL = int *;

#ifndef _WIN32
#ifndef BOOL
typedef int BOOL;
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
using LPBYTE = BYTE *;
using LPWORD = WORD *;
using LPINT = int *;
using LPDWORD = DWORD *;
using LPVOID = void *;
using COLORREF = std::uint32_t;
using USHORT = std::uint16_t;
using SOCKET = int;
using LPCSTR = const char *;
using LONG = long;
#define __int64 long long
typedef unsigned long long uhyper;
typedef unsigned __int64 __uint64;
#ifndef strcpy_s
#define strcpy_s(dst, sz, src) \
    do { \
        if ((dst) && (sz) > 0) { \
            std::strncpy((dst), (src), (size_t)(sz) - 1); \
            (dst)[(size_t)(sz) - 1] = '\0'; \
        } \
    } while (0)
#endif
#ifndef _itoa_s
#define _itoa_s(val, buf, sz, radix) \
    do { \
        if ((buf) && (sz) > 0) { \
            if ((radix) == 10) std::snprintf((buf), (size_t)(sz), "%d", (int)(val)); \
            else (buf)[0] = '\0'; \
        } \
    } while (0)
#endif
#ifndef __declspec
#define __declspec(x)
#endif

inline LONG InterlockedIncrement(LONG *value) {
    return ++(*value);
}
inline LONG InterlockedDecrement(LONG *value) {
    return --(*value);
}
#endif

#endif // T4C_STANDARD_TYPES_H
