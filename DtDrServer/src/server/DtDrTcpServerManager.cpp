#include "DtDrTcpServerManager.h"
#include "DtDrLog.h"
#include <sys/socket.h> //socket(), bind(), accept(), listen()
#include <arpa/inet.h> // struct sockaddr_in, struct sockaddr, inet_ntoa()
#include <string.h>
#include <unistd.h>
#include "DtDrBuffer.h"
#include "DtDrErrorNumber.h"

#define QUEUELIMIT 5

DtDrTcpServerManager::DtDrTcpServerManager() :
    m_pSessions(new DtDrList<DtDrTcpServerSession*>()),
    m_socket(0),
    m_port(0),
    m_pResponse(NULL)
{
    VER("DtDrTcpServerManager::%s()", __FUNCTION__);
}

DtDrTcpServerManager::~DtDrTcpServerManager()
{
    VER("DtDrTcpServerManager::%s()", __FUNCTION__);
    while (!m_pSessions->IsEmpty()) {
        DtDrTcpServerSession* pSession = m_pSessions->RemoveFront();
        if (pSession == NULL) {
            continue;
        }
        pSession->Stop();
        pSession->Join();
        delete pSession;
    }
    delete m_pSessions;
}

BOOL DtDrTcpServerManager::Initialize( unsigned short port )
{
    VER("DtDrTcpServerManager::%s()", __FUNCTION__);
    if (port < 49152) {
        ERR("port(%d) was defined IANA.");
        return FALSE;
    }
    m_port = port;
    return TRUE;
}

void DtDrTcpServerManager::SetCallback( DtDrTcpServerManagerResponse* pCallback )
{
    VER("DtDrTcpServerManager::%s()", __FUNCTION__);
    m_pResponse = pCallback;
}

void DtDrTcpServerManager::CloseSession( DtDrTcpServerSession* pSession )
{
    VER("DtDrTcpServerManager::%s()", __FUNCTION__);
    pSession->Stop();
    pSession->Join();
    m_pSessions->Erase(pSession);
    delete pSession;
}

void* DtDrTcpServerManager::Run( void* pArg )
{
    VER("DtDrTcpServerManager::%s(%p)", __FUNCTION__, pArg);
    if ((m_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0 ){
        ERR("socket() failed:%s", ERRSTRING());
        return NULL;
    }

    struct sockaddr_in addr; //server internet socket address
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port        = htons(m_port);

    if (bind(m_socket, (struct sockaddr *) &addr, sizeof(addr) ) < 0 ) {
        ERR("bind() failed:%s", ERRSTRING());
        return NULL;
    }

    if (listen(m_socket, QUEUELIMIT) < 0) {
        ERR("listen() failed:%s", ERRSTRING());
        return NULL;
    }

    struct timeval timeout;
    fd_set fds;
    fd_set fdsb;

    int client = 0;
    struct sockaddr_in clitSockAddr; // client internet socket address
    unsigned int clitLen = sizeof(clitSockAddr);
    FD_ZERO(&fdsb);
    FD_SET(m_socket, &fdsb);
    INFO("wait for access from client.:%d", m_port);
    while (m_running) {
        usleep(1 * 1000);
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        memcpy(&fds, &fdsb, sizeof(fd_set));
        if (select(m_socket + 1, &fds, NULL, NULL, &timeout) <= 0) {
            continue;
        }
        if (FD_ISSET(m_socket, &fds)) {
            if ((client = accept(m_socket, (struct sockaddr *) &clitSockAddr, &clitLen)) < 0) {
                ERR("accept() failed:%s", ERRSTRING());
                continue;
            }
            INFO("connected from %s.\n", inet_ntoa(clitSockAddr.sin_addr));
            DtDrTcpServerSession* pSession = new DtDrTcpServerSession();
            pSession->SetClient(client, clitSockAddr);
            pSession->SetCallback(this);
            pSession->Start(NULL);
            m_pSessions->PushBack(pSession);
        }
    }
    return NULL;
}

void DtDrTcpServerManager::SessionCallback( DtDrTcpServerSessionCallbackMessageData data )
{
    VER("DtDrTcpServerManager::%s()", __FUNCTION__);
    switch (data.m_message) {
        case DTDR_TCP_SERVER_SESSION_FOREIGN_HOST:
            if (m_pResponse != NULL) {
                m_pResponse->Callback(DtDrTcpServerManagerResponse::DTDR_TCP_SERVER_MANAGER_MESSAGE_CLOSE_SESSION, data.m_pBuffer);
            }
            break;
        case DTDR_TCP_SERVER_SESSION_RECV_BUFFER:
            if (m_pResponse != NULL) {
                m_pResponse->Callback(DtDrTcpServerManagerResponse::DTDR_TCP_SERVER_MANAGER_MESSAGE_RECV_BUFFER, data.m_pBuffer);
            }
            break;
        default:
            break;
    }
    return;
}


