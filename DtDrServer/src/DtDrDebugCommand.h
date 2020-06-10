#ifndef __DTDR_DEBUG_COMMAND_H_WAS_INCLUDED__
#define __DTDR_DEBUG_COMMAND_H_WAS_INCLUDED__

#include "DtDrThread.h"
#include "DtDrTypes.h"
#include "DtDrLog.h"

class DtDrDebugCommandCallback
{
public:
    DtDrDebugCommandCallback(){};
    virtual ~DtDrDebugCommandCallback(){};
    virtual BOOL DebugCommand( char* pCommand ) = 0;
};

class DtDrDebugCommand : public DtDrThread
{
public:
    DtDrDebugCommand();
    virtual ~DtDrDebugCommand();
    static DtDrDebugCommand* GetInstance();

    void* Run( void* pArg );

private:
    static DtDrDebugCommand* m_pThis;

};

#endif //__DTDR_DEBUG_COMMAND_H_WAS_INCLUDED__
