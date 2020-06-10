#ifndef __DTDR_MANAGER_H_WAS_INCLUDED__
#define __DTDR_MANAGER_H_WAS_INCLUDED__

#include "DtDrTypes.h"
#include "DtDrThread.h"
#include "DtDrDebugCommand.h"
#include "DtDrTcpServerManager.h"
#include "DtDrTimer.h"
#include "DtDrDataSink.h"

class DtDrManager : public DtDrDebugCommandCallback, public DtDrThread, public DtDrTcpServerManagerResponse, public DtDrTimerCallback
{
public:
    DtDrManager();
    virtual ~DtDrManager();

    BOOL DebugCommand( char* pCommand );
    void* Run( void* pArg );

    BOOL Callback( DtDrTcpServerManagerMessage, void* pBuffer );
    int TimerCallback(int time);
private:
    class DtDrManagerMessage : public DtDrMessage
    {
    public:
        enum {
            DT_DR_MANAGER_MESSAGE_CLOSE_SESSION,
            DT_DR_MANAGER_MESSAGE_EXIT,
            DT_DR_MANAGER_MESSAGE_FLUSH,
        };
    };
    DtDrDataSink* m_pDataSink;
    DtDrMessageQueue* m_pMessageQueue;
    DtDrTimer* m_pTimer;
};

#endif

