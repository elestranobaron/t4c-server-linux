#ifdef _WIN32
#include "stdafx.h"
#endif
#include "Lock.h"
#include "RegKeyHandler.h"
#include "Timer.h"
#include <mutex>

#ifdef _WIN32
#pragma warning(disable:4786 )
#endif

namespace{
    enum LockState{
        TryLocked = 1, Locked = 2, Unlocked = 3, PickLocked = 4
    };

    //////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////    
    class CDebugLockManagerTracer : public CDebugLockManager{
    public:
        CDebugLockManagerTracer(){
            /* Pas de RegKeyHandler ici : appele depuis le 1er CLock::Lock (init statique). */
            LOCK_TABLE_SIZE = 10000;

            curPos = 0;
            totalLockCount = 0;
            lockTable = new LockData[ LOCK_TABLE_SIZE ];

            concurrentLocks = 0;
            maxConcurrentLocks = 0;            
        }
        ~CDebugLockManagerTracer(){
            delete lockTable;
        }
        void Lock(){
            critSection.lock();
        }
        void Unlock(){
            critSection.unlock();
        }

        class LockData{
        public:
            LockData() : lockState( Unlocked ){};

            DWORD callerAddr;
            DWORD manAddr;
            DWORD threadId;
            DWORD previousEntry;
            CLock *lockPtr;
            DWORD lockEntry;
            DWORD timeStamp;
            DWORD waitTime;
            LockState lockState;
        };

        // Simply logs a lock call into the journal.
        DWORD AddEntry( DWORD callerAddr, DWORD manAddr, LockState state, CLock *lockPtr ){
            Lock();
            totalLockCount++;

            /* Table pleine ou fuite TryLocked : eviter boucle infinie + curPos hors bornes. */
            DWORD scanned = 0;
            DWORD slot = curPos;
            for( ; scanned < LOCK_TABLE_SIZE; ++scanned ){
                ++slot;
                if( slot >= LOCK_TABLE_SIZE ){
                    slot = 0;
                }
                if( lockTable[ slot ].lockState == Unlocked ){
                    curPos = slot;
                    break;
                }
            }
            if( scanned >= LOCK_TABLE_SIZE ){
                Unlock();
                return CDebugLockManager::EmptyEntry;
            }

            // Fill-in the entry data.
            lockTable[ curPos ].callerAddr = callerAddr;
            lockTable[ curPos ].manAddr = manAddr;
            lockTable[ curPos ].threadId = GetCurrentThreadId();
            lockTable[ curPos ].lockState = state;
            lockTable[ curPos ].lockPtr = lockPtr;
            lockTable[ curPos ].timeStamp = GetRunTime();            
            DWORD ret = curPos;

            Unlock();
            return ret;
        }

        ////////////////////////////////////////////////////////////////////
        virtual void Locking( DWORD callerAddr, CLock *lockPtr, DWORD &newLockEntry ){
            // When Locking, CLockData must be kept being the lock's Id
            // in order for other threads to add entries.
            DWORD manAddr;
            GET_CALLER_ADDR( manAddr );
            newLockEntry = AddEntry( callerAddr, manAddr, TryLocked, lockPtr );
            if( newLockEntry == CDebugLockManager::EmptyEntry ){
                return;
            }

            // Stat info to check the maximum number of entries ever used at one time.
            InterlockedIncrement( reinterpret_cast< long * >( &concurrentLocks ) );
            if( concurrentLocks > maxConcurrentLocks ){
                maxConcurrentLocks = concurrentLocks;
            }
        };
        ////////////////////////////////////////////////////////////////////
        virtual void GotLock( DWORD &newLockEntry, DWORD &previousLockEntry ){
            if( newLockEntry == CDebugLockManager::EmptyEntry ||
                newLockEntry >= LOCK_TABLE_SIZE ){
                previousLockEntry = newLockEntry;
                return;
            }
            Lock();
            lockTable[ newLockEntry ].previousEntry = previousLockEntry;
            lockTable[ newLockEntry ].lockState = Locked;

            const DWORD currentTime = GetRunTime();
            lockTable[ newLockEntry ].waitTime =
                currentTime - lockTable[ newLockEntry ].timeStamp;
            lockTable[ newLockEntry ].timeStamp = currentTime;

            previousLockEntry = newLockEntry;
            Unlock();
        };
        ////////////////////////////////////////////////////////////////////
        virtual void Unlocking( DWORD &lockEntry ){

            if( lockEntry == EmptyEntry ){
                return;
            }
            if( lockEntry >= LOCK_TABLE_SIZE ){
                lockEntry = EmptyEntry;
                return;
            }
            Lock();
            const DWORD oldLockEntry = lockEntry;
            lockEntry = lockTable[ oldLockEntry ].previousEntry;
            lockTable[ oldLockEntry ].lockState = Unlocked;
            Unlock();

            InterlockedDecrement( reinterpret_cast< long * >( &concurrentLocks ) );
        };
        ////////////////////////////////////////////////////////////////////
        virtual void Picklock( DWORD callerAddr, CLock *lockPtr, DWORD &lockEntry ){
            // Directly modify the lockEntry since lock is already acquired.
            DWORD manAddr;
            GET_CALLER_ADDR( manAddr );
            DWORD newLockEntry = AddEntry( callerAddr, manAddr, PickLocked, lockPtr );
            lockTable[ newLockEntry ].previousEntry = lockEntry;
            lockEntry = newLockEntry;
        };

        ////////////////////////////////////////////////////////////////////
        // This function checks the buffer and plays back all the lock data
        // into a CDebugLockManagerImpl implementation. It then delegates
        // the file data dumping to it.
        virtual bool DumpLockData( std::string fileName ){
            Lock();

            // Get the ending pos.
            DWORD startPos;
            if( curPos >= LOCK_TABLE_SIZE ){
                startPos = 0;
            }else{
                startPos = curPos + 1;
            }

            FILE *f = fopen( fileName.c_str(), "wb" );
            if( f == NULL ){
                return false;
            }

            SYSTEMTIME sysTime; 
	        GetLocalTime(&sysTime);
            
            fprintf( f, "-----" );
            fprintf( f, "\r\nDeadlock report %02u/%02u/%04u %02u:%02u:%02u",
                sysTime.wMonth,
                sysTime.wDay,
                sysTime.wYear, 
                sysTime.wHour, 
                sysTime.wMinute,
                sysTime.wSecond
            );
            fprintf( f, "\r\n" );

            // Read the lock table for unresolved entries.
            for(;;){
                LockData &lockData = lockTable[ startPos ];
                
                // Check the event.
                switch( lockData.lockState ){
                case TryLocked:
                    fprintf( f, "\r\nFound unresolved TryLock:" );
                    fprintf( f, "\r\n At %08ums: lockPtr=0x%08x, callerAddr=0x%08x, manAddr=0x%08x, threadId=%03u",
                        lockData.timeStamp, lockData.lockPtr, lockData.callerAddr, lockData.manAddr, lockData.threadId 
                    );

                    break;
                case Locked:
                    fprintf( f, "\r\nFound unresolved Lock:" );
                    fprintf( f, "\r\n At %08ums: lockPtr=0x%08x, callerAddr=0x%08x, manAddr=0x%08x, threadId=%03u, waited %ums.",
                        lockData.timeStamp, lockData.lockPtr, lockData.callerAddr, lockData.manAddr, lockData.threadId, lockData.waitTime 
                    );
                    break;
                case PickLocked:
                    fprintf( f, "\r\nFound unresolved PickLock:" );
                    fprintf( f, "\r\n At %08ums: lockPtr=0x%08x, callerAddr=0x%08x, manAddr=0x%08x, threadId=%03u",
                        lockData.timeStamp, lockData.lockPtr, lockData.callerAddr, lockData.manAddr, lockData.threadId 
                    );
                    break;
                }

                // If buffer was totally checked.
                if( startPos == curPos ){
                    break;                    
                }

                // Move pointer.
                startPos++;
                if( startPos >= LOCK_TABLE_SIZE ){
                    startPos = 0;
                }
            }
            fprintf( f, "\r\n\r\n" );

            fclose( f );
            
            critSection.unlock();

            // File logging never fails.
            return true;
        }
    private:
        // Internal lock.
        std::mutex critSection;
        
        // Buffer variables.
        DWORD curPos;
        DWORD LOCK_TABLE_SIZE;
        LockData *lockTable;
        
        // Stat variables.
        DWORD totalLockCount;
        long  concurrentLocks;
        long  maxConcurrentLocks;

    };
};

// Does absolutly nothing (disabled lock manager).
class CEmptyDebugLockManager : public CDebugLockManager{
    virtual void Locking( DWORD callerAddr, CLock *lockPtr, DWORD &newLockEntry ){};
    virtual void GotLock( DWORD &newLockEntry, DWORD &previousLockEntry ){};
    virtual void Picklock( DWORD callerAddr, CLock *lockPtr, DWORD &lockEntry ){};
    virtual void Unlocking( DWORD &lockEntry ){};
    virtual bool DumpLockData( std::string fileName ){ return false; };
};


// Returns the implementation to use.
CDebugLockManager *GetCurrentImplementation(){
#ifndef _WIN32
    /* Linux : pas de GET_CALLER_ADDR ; traceur non thread-safe sous charge UDP. */
    static CEmptyDebugLockManager instance;
    return &instance;
#else
    RegKeyHandler regKey;

    regKey.Open( HKEY_LOCAL_MACHINE, "Software\\Vircom\\The 4th Coming Server\\Logging" );

    const DWORD imp = regKey.GetProfileInt( "DeadlockLogging", 1 );

    if( imp != 0 ){
        static CDebugLockManagerTracer instance;
        return &instance;
    }
    static CEmptyDebugLockManager instance;
    return &instance;
#endif
}

// Returns the debug lock manager instance.
CDebugLockManager *CDebugLockManager::GetInstance(){
    static CDebugLockManager *instance = GetCurrentImplementation();
    return instance;
}
