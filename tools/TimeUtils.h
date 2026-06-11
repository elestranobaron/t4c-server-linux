#ifndef T4C_TIME_UTILS_H
#define T4C_TIME_UTILS_H

#include "StandardTypes.h"
#include <chrono>
#include <cstdint>

inline DWORD GetMonotonicTickCountMs() {
    return static_cast<DWORD>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()
        ).count()
    );
}

#endif // T4C_TIME_UTILS_H
