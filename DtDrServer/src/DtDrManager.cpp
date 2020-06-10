#include "DtDrManager.h"
#include <string.h>
#include "DtDrTcpServerManager.h"
#include <unistd.h>
#include "DtDrLock.h"
#include "DtDrMessageQueue.h"
#include "DtDrTcpServerSession.h"
#include "DtDrMySqlManager.h"

#define SERVER_PORT 49152

DtDrManager::DtDrManager() :
    m_pDataSink(new DtDrMySqlManager()),
    m_pMessageQueue(new DtDrMessageQueue()),
    m_pTimer(new DtDrTimer())
{
    VER("DtDrManager::%s()", __FUNCTION__);
    m_pDataSink->Initialize();
}

DtDrManager::~DtDrManager()
{
    VER("DtDrManager::%s()", __FUNCTION__);
    delete m_pDataSink;
    delete m_pMessageQueue;
    delete m_pTimer;
}

BOOL DtDrManager::DebugCommand( char* pCommand )
{
    INFO("DtDrManager::%s(%s)", __FUNCTION__, pCommand);

    if (strcmp(pCommand, "exit") == 0) {
        DtDrMessage msg;
        msg.m_message = DtDrManagerMessage::DT_DR_MANAGER_MESSAGE_EXIT;
        msg.m_pData = NULL;
        m_pMessageQueue->Send(&msg);
        Stop();
        return FALSE;
    }
    if (strstr(pCommand, "set ") == pCommand) {
        char* pBuffer = pCommand;
        pBuffer = strtok(pBuffer, " ");
        pBuffer = strtok(NULL, " "); // for "set"
        while (pBuffer) {
            VER("set command 1:\"%s\"", pBuffer);
            if (strcmp(pBuffer, "loglevel") == 0) {
                pBuffer = strtok(NULL, " ");
                VER("set command 2:%s", pBuffer);
                if (pBuffer != NULL) {
                    char* pEnd = NULL;
                    int level = strtol(pBuffer, &pEnd, 10);
                    CLOGLEVEL(static_cast<DtDrLogLevel>(level));
                }
            }
            pBuffer = strtok(NULL, ",");
        }
    }
    return TRUE;
}

void* DtDrManager::Run(void* pArg)
{
    INFO("DtDrManager::%s(%p)", __FUNCTION__, pArg);
    DtDrTcpServerManager* pServerManager = new DtDrTcpServerManager();

    if (pServerManager->Initialize(SERVER_PORT) == FALSE) {
        ERR("DtDrManager::%s() DtDrTcpServerManager initialize error.", __FUNCTION__);
        return NULL;
    }
    m_pTimer->SetTimerCallback(this);
    m_pTimer->Start();
    pServerManager->SetCallback(this);
    pServerManager->Start();

    m_pDataSink->Start();
    DtDrManagerMessage msg;
    while (m_running) {
        usleep(1 * 1000);
        if (m_pMessageQueue->Recv(&msg) >= 0) {
            switch (msg.m_message) {
                case DtDrManagerMessage::DT_DR_MANAGER_MESSAGE_EXIT:
                    INFO("DtDrManager::%s() message:%d", __FUNCTION__, msg.m_message);
                    m_running = false;
                    break;
                case DtDrManagerMessage::DT_DR_MANAGER_MESSAGE_CLOSE_SESSION:
                    INFO("DtDrManager::%s() message:%d", __FUNCTION__, msg.m_message);
                    pServerManager->CloseSession(reinterpret_cast<DtDrTcpServerSession*>(msg.m_pData));
                    break;
                case DtDrManagerMessage::DT_DR_MANAGER_MESSAGE_FLUSH:
                    FLUSH();
                    break;
                default:
                    ERR("DtDrManager::%s() Unknown message:%d", __FUNCTION__, msg.m_message);
                    break;
            }
        }
    }
    pServerManager->Stop();
    pServerManager->Join();
    delete pServerManager;
    m_pTimer->Stop();
    m_pTimer->Join();
    delete m_pTimer;
    m_pTimer = NULL;
    m_pDataSink->Stop();
    m_pDataSink->Join();
    delete m_pDataSink;
    m_pDataSink = NULL;

    return NULL;
}

BOOL DtDrManager::Callback( DtDrTcpServerManagerMessage message, void* pBuffer )
{
    VER("DtDrManager::%s(%p)", __FUNCTION__, pBuffer);
    if (pBuffer == NULL) {
        ERR("DtDrManager::%s(%p):pBufer is NULL.", __FUNCTION__, pBuffer);
        return FALSE;
    }

    switch (message) {
        case DtDrTcpServerManagerMessage::DTDR_TCP_SERVER_MANAGER_MESSAGE_CLOSE_SESSION: {
            DtDrManagerMessage msg;
            msg.m_message = DtDrManagerMessage::DT_DR_MANAGER_MESSAGE_CLOSE_SESSION;
            msg.m_pData = pBuffer;
            m_pMessageQueue->Send(&msg);
            break;
        }
        case DtDrTcpServerManagerMessage::DTDR_TCP_SERVER_MANAGER_MESSAGE_RECV_BUFFER:
            if (m_pDataSink == NULL) {
                ERR("DtDrManager::%s(%p):Packet Parser is NULL.", __FUNCTION__, pBuffer);
                return FALSE;
            }
//            m_pDataSink->SetBuffer(pBuffer);
            m_pDataSink->SetPacket(reinterpret_cast<DtDrPacketProtocolParser*>(pBuffer));
            break;
        default:
            ERR("DtDrManager::%s() Unknown Message:%d", __FUNCTION__, message);
            break;
    }

    return TRUE;
}

int DtDrManager::TimerCallback(int time)
{
    VER("DtDrManager::%s()", __FUNCTION__);
    DtDrManagerMessage msg;
    msg.m_message = DtDrManagerMessage::DT_DR_MANAGER_MESSAGE_FLUSH;
    msg.m_pData = NULL;
    m_pMessageQueue->Send(&msg);
}


