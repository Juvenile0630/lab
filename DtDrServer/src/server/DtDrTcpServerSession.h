#ifndef __DTDR_TCP_SERVER_SESSION_H_WAS_INCLUDED__
#define __DTDR_TCP_SERVER_SESSION_H_WAS_INCLUDED__

#include "DtDrTypes.h"
#include "DtDrThread.h"
#include <sys/socket.h> //socket(), bind(), accept(), listen()
#include <arpa/inet.h> // struct sockaddr_in, struct sockaddr, inet_ntoa()

enum DtDrTcpServerSessionCallbackMessage {
    DTDR_TCP_SERVER_SESSION_FOREIGN_HOST,   // connection close from client
    DTDR_TCP_SERVER_SESSION_RECV_BUFFER,    // buffer receive
};

struct DtDrTcpServerSessionCallbackMessageData {
    DtDrTcpServerSessionCallbackMessage m_message;
    void* m_pBuffer;
};

class DtDrTcpServerSessionCallback
{
public:
    DtDrTcpServerSessionCallback(){};
    virtual ~DtDrTcpServerSessionCallback(){};
    virtual void SessionCallback( DtDrTcpServerSessionCallbackMessageData data ) = 0;
};

class DtDrTcpServerSession : public DtDrThread
{
public:
    DtDrTcpServerSession();
    virtual ~DtDrTcpServerSession();
    BOOL Initialize();
    void SetCallback( DtDrTcpServerSessionCallback* pCallback );
    void SetClient( int socket, sockaddr_in clientInfo );
    void Stop();
    void* Run( void* pArg );

private:
    void sendForeignHost( void* pBuffer );
    void sendRecvBuffer( void* pBuffer );
    int m_socket;
    sockaddr_in m_Info;
    DtDrTcpServerSessionCallback* m_pCallback;
};

#endif // __DTDR_TCP_SERVER_SESSION_H_WAS_INCLUDED__
