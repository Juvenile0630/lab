#include "DtDrTcpServerSession.h"
#include "DtDrLog.h"
#include "DtDrBuffer.h"
#include <string.h>
#include <unistd.h>
#include "DtDrPacketAnalyzer.h"

#define BUFSIZE 1024
#define BUFFER_MAX_LENGTH 4096

DtDrTcpServerSession::DtDrTcpServerSession() :
    m_socket(0),
    m_Info(),
    m_pCallback(NULL)
{
    VER("DtDrTcpServerSession::%s", __FUNCTION__);
}

DtDrTcpServerSession::~DtDrTcpServerSession()
{
    VER("DtDrTcpServerSession::%s", __FUNCTION__);
}

BOOL DtDrTcpServerSession::Initialize()
{
    VER("DtDrTcpServerSession::%s", __FUNCTION__);
    return TRUE;
}

void DtDrTcpServerSession::SetCallback( DtDrTcpServerSessionCallback* pCallback )
{
    VER("DtDrTcpServerSession::%s", __FUNCTION__);
    m_pCallback = pCallback;
}

void DtDrTcpServerSession::Stop()
{
    VER("DtDrTcpServerSession::%s", __FUNCTION__);
    m_running = FALSE;
    if (m_socket != 0) {
        close(m_socket);
    }
}

void DtDrTcpServerSession::SetClient( int socket, sockaddr_in clientInfo )
{
    VER("DtDrTcpServerSession::%s", __FUNCTION__);
    m_socket = socket;
    m_Info = clientInfo;
}

void* DtDrTcpServerSession::Run( void* pArg )
{
    VER("DtDrTcpServerSession::%s(%p) start", __FUNCTION__, pArg);

    if (m_socket == 0) {
        ERR("socket uninitialized: m_socket = %d", m_socket);
        return NULL;
    }

    int recvLength = 0;
    char recvBuffer[BUFSIZE];
    char tmpBuffer[BUFSIZE];

    struct timeval timeout;
    fd_set fds;
    fd_set fdsb;
    FD_ZERO(&fdsb);
    FD_SET(m_socket, &fdsb);

    DtDrBuffer *pBuffer = NULL;
    int counter = 0;
    DtDrPacketAnalyzer* pPacketAnalyzer = new DtDrPacketAnalyzer();
    while (m_running) {
        usleep(1 * 1000);
        timeout.tv_sec = 3;
        timeout.tv_usec = 0;
        memcpy(&fds, &fdsb, sizeof(fd_set));
        if (select(m_socket + 1, &fds, NULL, NULL, &timeout) <= 0) {
            continue;
        }

        if (FD_ISSET(m_socket, &fds) == 0) {
            continue;
        }
        memset(recvBuffer, 0, BUFSIZE);

        recvLength = recv(m_socket, recvBuffer, BUFSIZE, 0);
        if (recvLength < 0) {
            if (recvLength == EAGAIN || recvLength == EWOULDBLOCK) {
                ERR("recv timeout error.");
            } else if (recvLength == EBADF) {
                ERR("recv bad socket descriptor.");
            } else if (recvLength == ECONNREFUSED) {
                ERR("recv remote disconnect.");
            } else if (recvLength == EFAULT) {
                ERR("recv buffer irregal address.");
            } else if (recvLength == EINTR) {
                ERR("recv interupt signal.");
            } else if (recvLength == EINVAL) {
                ERR("recv irregal argument.");
            } else if (recvLength == ENOMEM) {
                ERR("recv no memory.");
            } else if (recvLength == ENOTCONN) {
                ERR("recv not connection.");
            } else if (recvLength == ENOTSOCK) {
                ERR("recv not socket.");
            } else {
                ERR("recv unknown error.:%d", recvLength);
            }
        } else if (recvLength == 0) {
            INFO("connection closed by foreign host");
            sendForeignHost(this);
            m_running = FALSE;
            break;
        }
        int length = 0;
        if (pBuffer != NULL && pBuffer->GetLength() > BUFFER_MAX_LENGTH) {
            ERR("delete unknown buffer:%d", pBuffer->GetLength());
            delete pBuffer;
            pBuffer = NULL;
        }
        int remainLength = recvLength;
        char* pBuffer = recvBuffer;
        int readedLength = 0;
        while (remainLength > 0) {
            if (pPacketAnalyzer->SetBuffer(pBuffer, remainLength, &readedLength)) {
                sendRecvBuffer(pPacketAnalyzer->GetPacketParser());
                pPacketAnalyzer->Initialize();
            }
            remainLength -= readedLength;
            pBuffer += readedLength;
        }
#if 0
        for (int i = 0;i < recvLength;i++) {
            if (recvBuffer[i] == ';') {
                counter++;
                if (pBuffer == NULL) {
                    pBuffer = new DtDrBuffer();
                }
                pBuffer->Add(tmpBuffer, length);
                length = 0;
                sendRecvBuffer(pBuffer);
                delete pBuffer;
                pBuffer = NULL;

            } else {
                tmpBuffer[length++] = recvBuffer[i];
            }
        }
        if (length > 0) {
            if (pBuffer == NULL) {
                pBuffer = new DtDrBuffer();
            }
            pBuffer->Add(tmpBuffer, length);
            length = 0;
        }
#endif
    }
    delete pPacketAnalyzer;
    return NULL;
}

void DtDrTcpServerSession::sendForeignHost( void* pBuffer )
{
    VER("DtDrTcpServerSession::%s(%p)", __FUNCTION__, pBuffer);
    if (m_pCallback == NULL) { return; }
    DtDrTcpServerSessionCallbackMessageData data;
    data.m_message = DTDR_TCP_SERVER_SESSION_FOREIGN_HOST;
    data.m_pBuffer = pBuffer;
    m_pCallback->SessionCallback(data);
}

void DtDrTcpServerSession::sendRecvBuffer( void* pBuffer )
{
    VER("DtDrTcpServerSession::%s(%p)", __FUNCTION__, pBuffer);
    if (m_pCallback == NULL) { return; }
    DtDrTcpServerSessionCallbackMessageData data;
    data.m_message = DTDR_TCP_SERVER_SESSION_RECV_BUFFER;
    data.m_pBuffer = pBuffer;
    m_pCallback->SessionCallback(data);
}


