#ifndef __DTDR_TCP_SERVER_MANAGER_H_WAS_INCLUDED__
#define __DTDR_TCP_SERVER_MANAGER_H_WAS_INCLUDED__

#include "DtDrTypes.h"
#include "DtDrThread.h"
#include "DtDrList.h"
#include "DtDrTcpServerSession.h"
#include "DtDrBuffer.h"
#include "DtDrMessageQueue.h"

class DtDrTcpServerManagerResponse : public DtDrMessage
{
public:
    DtDrTcpServerManagerResponse(){};
    virtual ~DtDrTcpServerManagerResponse(){};
    enum DtDrTcpServerManagerMessage {
        DTDR_TCP_SERVER_MANAGER_MESSAGE_CLOSE_SESSION,
        DTDR_TCP_SERVER_MANAGER_MESSAGE_RECV_BUFFER,
    };
    virtual BOOL Callback( DtDrTcpServerManagerMessage message, void* pBuffer ) = 0;
};

class DtDrTcpServerManager : public DtDrThread, public DtDrTcpServerSessionCallback
{
public:
    DtDrTcpServerManager();
    virtual ~DtDrTcpServerManager();
    BOOL Initialize( unsigned short port ); // port is 49152-65535
    void SessionCallback( DtDrTcpServerSessionCallbackMessageData data );
    void SetCallback( DtDrTcpServerManagerResponse* pCallback );
    void CloseSession( DtDrTcpServerSession* pSession );
    void* Run( void* pArg );

private:
    DtDrList<DtDrTcpServerSession*>* m_pSessions;
    int m_socket;
    unsigned short m_port;
    DtDrTcpServerManagerResponse* m_pResponse;
    DtDrMessageQueue* m_pMessageQueue;
};

#endif // __DTDR_TCP_SERVER_MANAGER_H_WAS_INCLUDED__
