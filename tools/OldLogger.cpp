#ifdef _WIN32
#include "stdafx.h"
#endif

#include "OldLogger.h"
#include "TimeUtils.h"

#include <cstdarg>
#include <cstdio>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <queue>
#include <stack>
#include <string>
#include <thread>
#include <condition_variable>
#include <chrono>

#ifdef _WIN32
#pragma warning(disable : 4786)
#endif

using namespace std;

class LogItemQueue;
class LogItem {
public:
    void SetLogger(CLogger* lpNewLogger) { lpLogger = lpNewLogger; }
    void SetText(string bsNewText) { bsText = bsNewText; }

    CLogger* GetLogger(void) { return lpLogger; }
    string& GetText(void) { return bsText; }

    static LogItem* GetLogItem(void);
    inline void FreeLogItem(void);
    inline void Push(void);
    static LogItem* Pop(void);

private:
    typedef stack<LogItem*> ItemPool;
    typedef LogItemQueue ItemQueue;

    LogItem() : lpLogger(NULL), bsText("") {}

    static ItemPool cItemPool;
    static std::mutex cPoolLock;
    static ItemQueue cItemQueue;

    CLogger* lpLogger;
    string bsText;
};

class LogItemQueue {
public:
    LogItem* pop(void) {
        std::unique_lock<std::mutex> lock(queueMutex);
        if (qQueue.empty()) {
            queueCv.wait_for(lock, std::chrono::seconds(10), [this]() { return !qQueue.empty(); });
        }
        if (qQueue.empty()) {
            return NULL;
        }
        LogItem* lpLogItem = qQueue.front();
        qQueue.pop();
        return lpLogItem;
    }

    void push(LogItem* lpItem) {
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            qQueue.push(lpItem);
        }
        queueCv.notify_one();
    }

private:
    std::mutex queueMutex;
    std::condition_variable queueCv;
    queue<LogItem*> qQueue;
};

CLogger::CLogger(void)
    : bsLogFile((std::filesystem::path("Logs") / "server.log").string()),
      wSetLogLevels(LOG_DEBUG_LIGHT) {}

CLogger::~CLogger(void) {}

LogItem::ItemPool LogItem::cItemPool;
LogItem::ItemQueue LogItem::cItemQueue;
std::mutex LogItem::cPoolLock;

CLogger::AutoAsyncLogThreadStart::AutoAsyncLogThreadStart(void) {
#ifdef _WIN32
    _beginthread(AsyncLogThread, 0, NULL);
#else
    std::thread([]() { AsyncLogThread(NULL); }).detach();
#endif
}

void CLogger::SetLogLevels(WORD wNewLogLevels) {
    Lock();
    wSetLogLevels = wNewLogLevels;
    Unlock();
}

void CLogger::SetLogFile(string bsNewFile) {
    Lock();
    if (!bsNewFile.empty()) {
        bsLogFile = bsNewFile;
    }
    Unlock();
}

WORD CLogger::GetLogLevels(void) {
    return wSetLogLevels;
}

namespace {
inline const char* LogLvl2Str(WORD wLogLvl) {
    switch (wLogLvl) {
        case LOG_CRIT_ERRORS: return "CRITICAL";
        case LOG_GEN_ERRORS: return "Error";
        case LOG_DEBUG_LVL1:
        case LOG_DEBUG_LVL2: return "Debug";
        case LOG_DEBUG_LVL3: return "Debug3";
        case LOG_DEBUG_LVL4: return "Debug4";
        case LOG_MEMORY: return "Memory";
        case LOG_WARNING: return "Warning";
        case LOG_ALWAYS: return "Info";
        case LOG_MISC_1: return "Misc1";
        case LOG_SYSOP: return "Admin";
        case LOG_DEBUG_HIGH: return "HeavyDbg";
        default: return "Log";
    }
}

inline string FormatLogString(WORD wLogLevels, const char* szText) {
    string bsString;

    if (wLogLevels != LOG_DEBUG_LVL2) {
        const std::time_t now = std::time(NULL);
        std::tm tmLocal{};
#ifdef _WIN32
        localtime_s(&tmLocal, &now);
#else
        localtime_r(&now, &tmLocal);
#endif
        static DWORD startTick = GetMonotonicTickCountMs();
        const DWORD elapsed = GetMonotonicTickCountMs() - startTick;

        char ts[256];
        std::snprintf(
            ts,
            sizeof(ts),
            "\r\n(%s),%d/%d/%d,%d:%02d:%02d,+%ums,",
            LogLvl2Str(wLogLevels),
            tmLocal.tm_mon + 1,
            tmLocal.tm_mday,
            tmLocal.tm_year + 1900,
            tmLocal.tm_hour,
            tmLocal.tm_min,
            tmLocal.tm_sec,
            elapsed);
        bsString = ts;
    } else {
        bsString = "\r\n\t";
    }

    bsString += szText;
    return bsString;
}
} // namespace

void CLogger::AsyncLog(WORD wLogLevels, const char* szText, ...) {
    if (wLogLevels & wSetLogLevels || wLogLevels == LOG_ALWAYS) {
        char lpBuffer[4096];
        va_list argp;
        va_start(argp, szText);
        vsnprintf(lpBuffer, sizeof(lpBuffer), szText, argp);
        string bsLoggedString = FormatLogString(wLogLevels, lpBuffer);
        va_end(argp);

        LogItem* lpLogItem = LogItem::GetLogItem();
        lpLogItem->SetLogger(this);
        lpLogItem->SetText(bsLoggedString);

        static AutoAsyncLogThreadStart cAutoStart;
        lpLogItem->Push();
    }
}

void CLogger::SyncLog(WORD wLogLevels, const char* szText, ...) {
    if (wLogLevels & wSetLogLevels) {
        char lpBuffer[4096];
        va_list argp;
        va_start(argp, szText);
        vsnprintf(lpBuffer, sizeof(lpBuffer), szText, argp);
        string bsLoggedString = FormatLogString(wLogLevels, lpBuffer);
        va_end(argp);
        WriteLog(bsLoggedString.c_str());
    }
}

void CLogger::AsyncLogThread(void* lpData) {
    (void)lpData;
    while (1) {
        LogItem* lpLogItem = LogItem::Pop();
        if (lpLogItem != NULL) {
            lpLogItem->GetLogger()->WriteLog(lpLogItem->GetText().c_str());
            lpLogItem->FreeLogItem();
        }
    }
}

void CLogger::WriteLog(const char* szString) {
    string bsFile;
    {
        Lock();
        bsFile = bsLogFile;
        Unlock();
    }

    const std::filesystem::path logPath = bsFile.empty() ? (std::filesystem::path("Logs") / "server.log")
                                                         : std::filesystem::path(bsFile);
    std::error_code ec;
    if (logPath.has_parent_path()) {
        std::filesystem::create_directories(logPath.parent_path(), ec);
    }

    std::ofstream out(logPath, std::ios::app | std::ios::binary);
    if (!out.is_open()) {
        return;
    }
    out << szString;
}

void CLogger::Lock() {
    critSection.lock();
}

void CLogger::Unlock() {
    critSection.unlock();
}

LogItem* LogItem::GetLogItem(void) {
    std::lock_guard<std::mutex> lock(cPoolLock);
    if (cItemPool.empty()) {
        for (int i = 0; i < 10; i++) {
            cItemPool.push(new LogItem());
        }
    }
    LogItem* lpLogItem = cItemPool.top();
    cItemPool.pop();
    return lpLogItem;
}

void LogItem::FreeLogItem(void) {
    std::lock_guard<std::mutex> lock(cPoolLock);
    cItemPool.push(this);
}

void LogItem::Push(void) {
    cItemQueue.push(this);
}

LogItem* LogItem::Pop(void) {
    return cItemQueue.pop();
}
