#ifndef LOCK_H_CLASS_DEFINITION
#define LOCK_H_CLASS_DEFINITION

#define LOCK_FUNC()     void Lock( void )
#define UNLOCK_FUNC()   void Unlock( void )
#define PICKLOCK_FUNC() BOOL PickLock( void )

#include "StandardTypes.h"
#include "stackhlp.h"
#include <string>
#include <mutex>
#include <thread>

class CLock;
class CDebugLockManager{
public:
    virtual void Locking( DWORD callerAddr, CLock *lockPtr, DWORD &newLockEntry ) = 0;
    virtual void GotLock( DWORD &newLockEntry, DWORD &previousLockEntry ) = 0;
    virtual void Picklock( DWORD callerAddr, CLock *lockPtr, DWORD &lockEntry ) = 0;
    virtual void Unlocking( DWORD &lockEntry ) = 0;
    virtual bool DumpLockData( std::string fileName ) = 0;

    enum{ EmptyEntry = 0xFFFFFFFF };

    static __declspec( dllexport ) CDebugLockManager *GetInstance();
};
//////////////////////////////////////////////////////////////////////////////////////////
// CLock class definition.
class CLock  
{
public:
    CLock(){ 
        lockEntry = CDebugLockManager::EmptyEntry;
    };
    virtual ~CLock(){
    };

    //////////////////////////////////////////////////////////////////////////////////////////
    virtual void Lock( ){
        DWORD callerAddr;
        GET_CALLER_ADDR( callerAddr );

        // Holds the new lock entry table. This information is thread specific
        // until the lock is aquired.
        DWORD newLockEntry;

        // Register that we are waiting for the lock to be released.
        CDebugLockManager::GetInstance()->Locking( callerAddr, this, newLockEntry );

        csThreadLock.lock();

        // Register that we are inside the lock.
        CDebugLockManager::GetInstance()->GotLock( newLockEntry, lockEntry );
    };
    //////////////////////////////////////////////////////////////////////////////////////////    
    virtual void Unlock( ){
        // Log that this lock was released.
        CDebugLockManager::GetInstance()->Unlocking( lockEntry );

        csThreadLock.unlock();
    }
    
    virtual int PickLock( ){
        if( csThreadLock.try_lock() ){
            DWORD callerAddr;
            GET_CALLER_ADDR( callerAddr );

            // Log picklog
            CDebugLockManager::GetInstance()->Picklock( callerAddr, this, lockEntry );
            return true;
        }
        return false;
    }

private:
    // Entry of lock in the lock table.
    DWORD lockEntry;

#if defined(__linux__)
    /* create_world_unit() tient deja WorldMap::Lock avant deposit_unit() — std::mutex non recursif = stall 46. */
    std::recursive_mutex csThreadLock;
#else
    std::mutex csThreadLock;
#endif

};

//////////////////////////////////////////////////////////////////////////////////////////
// Auto lock structure. Locks lock on creation, and unlocks it on destruction.
// Client is responsible for preserving the lock's integrity.
class CAutoLock{
public:
    CAutoLock( CLock *lpNewLock ) : lpLock( lpNewLock ){
        lpLock->Lock();
    }
    ~CAutoLock(){
        lpLock->Unlock();
    }
private:
    CLock *lpLock;
};



//////////////////////////////////////////////////////////////////////////////////////////
// Helper functions, allows safely locking multiple locks.
inline void MultiLock( CLock *cLockL, CLock *cLockR ){
    cLockL->Lock();
    while( !cLockR->PickLock() ){
        cLockL->Unlock();
        std::this_thread::yield();
        cLockL->Lock();
    }
}
//////////////////////////////////////////////////////////////////////////////////////////
// Allows lockuing 3 locks.
inline void MultiLock3( CLock *cLock1, CLock *cLock2, CLock *cLock3 ){
    bool boFailed = true;
    while( boFailed ){
        cLock1->Lock();
        if( cLock2->PickLock() ){
            if( cLock3->PickLock() ){
                boFailed = false;
            }else{
                cLock1->Unlock();
                cLock2->Unlock();
                boFailed = true;
            }
        }else{
            cLock1->Unlock();
            boFailed = true;
        }
    }
}

#endif//#define LOCK_H_CLASS_DEFINITION
