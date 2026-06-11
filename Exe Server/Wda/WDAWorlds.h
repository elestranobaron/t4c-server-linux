/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#ifndef AFX_WDAWORLDS_H__0F034DCF_D33D_11D2_84AD_00E02922FA40__INCLUDED_
#define AFX_WDAWORLDS_H__0F034DCF_D33D_11D2_84AD_00E02922FA40__INCLUDED_

#if _MSC_VER >= 1000
	#pragma once
#endif

#include "WDATable.h"

/******************************************************************************/
class WDAWorlds : public WDATable  
/******************************************************************************/
{
public:
	// Construction
    WDAWorlds( vir::Logger &cTraceLogger );
	virtual ~WDAWorlds();

    // Structures.
    struct WorldData
	{
        WorldData() : 
            m_ReadOnly(false),
            wWorldID(0),
            wWorldSizeX(0),
            wWorldSizeY(0),
            lpbDataW(NULL),
            lpbDataWOri(NULL)
        {}        

        bool                m_ReadOnly;
        WORD                wWorldID;        
        std::string         csWorldName;
        WORD                wWorldSizeX;
        WORD                wWorldSizeY;
        LPBYTE              lpbDataW;
        LPBYTE              lpbDataWOri;
    };
    // Creates from a wdaFile.
    virtual void CreateFrom( WDAFile &wdaFile, bool createReadOnly );
    // Returns the worlds.
    virtual std::vector< WorldData > &GetWorlds( void );

private:
    std::vector< WorldData > vWorlds;
};

#endif
