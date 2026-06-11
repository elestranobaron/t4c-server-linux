#ifdef _WIN32

#include "stdafx.h"
#include "TFC Server.h"
#include "TFC_MAIN.h"
#include "ExpFltr.h"
#include "TFCServerGP.h"
#include "RegKeyHandler.h"
#include "ODBCTrace.h"
#include "ThreadMonitor.h"

extern TFC_MAIN *TFCServer;
extern CTFCServerApp theApp;

BOOL LockGP( BOOL boTry ){
    try{
        static CLock cGPlock;
        if( boTry ){
            return cGPlock.PickLock();
        }else{
            cGPlock.Lock();
        }
    }catch(...){
    }
    return FALSE;
}

void ExceptionFunction(unsigned int, EXCEPTION_POINTERS* pExp)
{
	TFCException *excp = new TFCException;
	excp->SetException(pExp);
	throw excp;
}

void ReportGP(TFCException *e, CString csThreadNameID)
{
    (void)e;
    (void)csThreadNameID;
}

void RegisterReportFunction(REPORT_FUNC)
{
}

#else

#include "TFCServerGP.h"
#include "TimeUtils.h"

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <thread>

#define TIMEOUT 300000
#define SERVER_GP_TIMEOUT_KILL 105

namespace {
std::mutex gpMutex;
std::atomic<bool> gpFinished(false);
}

void ExceptionFunction(unsigned int, EXCEPTION_POINTERS*) {
    throw new TFCException();
}

BOOL LockGP(BOOL boTry) {
    if (boTry) {
        return gpMutex.try_lock() ? 1 : 0;
    }
    gpMutex.lock();
    return 1;
}

void ReportGP(TFCException* e, CString) {
    (void)e;
    gpFinished.store(false);

    std::thread reportThread([]() {
        const std::filesystem::path logsDir = std::filesystem::path("Logs");
        const std::filesystem::path gpFilePath = logsDir / "T4CServerGP.out";
        std::error_code ec;
        std::filesystem::create_directories(logsDir, ec);

        std::ofstream out(gpFilePath, std::ios::app | std::ios::binary);
        if (out.is_open()) {
            out << "[Linux] GP report placeholder, tick=" << GetMonotonicTickCountMs() << "\n";
        }
        gpFinished.store(true);
    });

    std::thread timeoutThread([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(TIMEOUT));
        if (!gpFinished.load()) {
            std::exit(SERVER_GP_TIMEOUT_KILL);
        }
    });

    reportThread.join();
    gpFinished.store(true);
    timeoutThread.detach();
}

void RegisterReportFunction(REPORT_FUNC) {
}

#endif
