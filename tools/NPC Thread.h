/******************************************************************************
Modify for vs2008 (06/05/2009)
/******************************************************************************/
#ifndef NPCMAIN_H_INCLUDE
#define NPCMAIN_H_INCLUDE

/******************************************************************************/
class NPCMain
/******************************************************************************/
{
public:
    static NPCMain &GetInstance();

    
    unsigned int GetThreadId(){ return nNPCThreadId; };
    Creatures *GetMainNPC( LPCTSTR npcName, DWORD order, WORD wLang );
    DWORD CountNPC( LPCTSTR npcName, WORD wLang );

    void Create();
    void KillAll();
    void KillAllID(DWORD dwID);
private:
    NPCMain();
    void NPCThreadFunc( void );

    static unsigned int CALLBACK NPCThread( void *pParam );

    HANDLE hNPCThread;
    unsigned int    nNPCThreadId;

    bool boNPCThreadDone;
};


#endif