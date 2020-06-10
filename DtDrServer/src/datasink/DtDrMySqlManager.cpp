#include <unistd.h>
#include "DtDrMySqlManager.h"
#include "DtDrClock.h"
#include <string.h>

#define DTDR_DATABASE "dandelion"
#define DTDR_HOSTNAME "localhost"
#define DTDR_USERNAME "richard"
#define DTDR_PASSWORD "lyons2004"

#define DTDR_TABLE_MAX_ROW_LENGTH 100000
#define DTDR_TABLE_MAX_STACK_ROW_LENGTH 100

DtDrMySqlManager::DtDrMySqlManager()
{
    VER("DtDrMySqlManager::%s()", __FUNCTION__);
    m_pConnector = new DtDrMySqlConnector();
    if (m_pConnector == NULL) {
        ERR("DtDrMySqlManager::%s() m_pConnector is NULL", __FUNCTION__);
        return;
    }
    m_pConnector->SetHostName(DTDR_HOSTNAME);
    m_pConnector->SetDatabase(DTDR_DATABASE);
    m_pConnector->SetUserName(DTDR_USERNAME);
    m_pConnector->SetPassword(DTDR_PASSWORD);
    m_pTableManager = new DtDrMySqlTableManager(m_pConnector);
    m_pMessageQueue = new DtDrMessageQueue();

}

DtDrMySqlManager::~DtDrMySqlManager()
{
    VER("DtDrMySqlManager::%s()", __FUNCTION__);

    Stop();
    Join();

    m_pConnector->Disconnect();
    delete m_pMessageQueue;
    delete m_pTableManager;
    delete m_pConnector;
}

BOOL DtDrMySqlManager::Initialize()
{
    VER("DtDrMySqlManager::%s()", __FUNCTION__);
    if (m_pConnector == NULL) {
        ERR("DtDrMySqlManager::%s() m_pConnecctor is NULL", __FUNCTION__);
        return FALSE;
    }
    BOOL bRet = FALSE;
    do {
        bRet = m_pConnector->Connect();
        usleep(1 * 1000 * 1000);
    } while (!bRet);

    m_pTableManager->Initialize();

    DtDrList<DtDrList<DtDrBuffer*>*>* pList = new DtDrList<DtDrList<DtDrBuffer*>*>();
    m_pTableManager->GetUncompletedTableList(pList);
    while (!pList->IsEmpty()) {
        DtDrList<DtDrBuffer*>* pRow = pList->RemoveFront();
        DtDrMySqlTableMasterRecord* pRecord = new DtDrMySqlTableMasterRecord(pRow);
        if (pRecord->GetEnd() != NULL) {
            ERR("pRecord->GetEnd() is not NULL");
            delete pRecord;
            delete pRow;
            continue;
        }
        INFO("TableName:%s", pRecord->GetTable());
        DtDrMySqlDataTable* pDataTable = new DtDrMySqlDataTable(m_pConnector, pRecord->GetTable(), pRecord->GetDevice());

        char buffer[32];
        memset(buffer, 0, sizeof(buffer));
        DtDrTimeStamp time = DtDrClock::GetTime();
        snprintf(buffer, sizeof(buffer) - 1, "%04d-%02d-%02d %02d:%02d:%02d.%06d",
            time.m_date.tm_year + 1900,
            time.m_date.tm_mon + 1,
            time.m_date.tm_mday,
            time.m_date.tm_hour,
            time.m_date.tm_min,
            time.m_date.tm_sec,
            time.m_msec.tv_usec
        );
        m_pTableManager->Update(buffer, pDataTable->GetTableName()->GetBuffer());
        delete pDataTable;
        delete pRecord;
        delete pRow;
    }
    delete pList;

}

BOOL DtDrMySqlManager::SetPacket( DtDrPacketProtocolParser* pPacketProtocol )
{
    DtDrLock lock(&m_mutex);
    VER("DtDrDataSink::%s(%p)", __FUNCTION__, pPacketProtocol);
    if (pPacketProtocol == NULL) {
        ERR("DtDrMySqlManager::%s() pPacket is NULL",__FUNCTION__);
        return FALSE;
    }
    if (pPacketProtocol->IsValidPacket() == FALSE) {
        delete pPacketProtocol;
        return FALSE;
    }
    ERR("pPacketProtocol:%p", pPacketProtocol);
    m_pPacketArray->PushBack(pPacketProtocol);

    DtDrMySqlManagerMessage message;
    message.m_message = DtDrMySqlManagerMessage::SET_PACKET;
    m_pMessageQueue->Send(&message);
    return TRUE;
}

BOOL DtDrMySqlManager::SetBuffer(char* pData, int length, BOOL bRegister)
{
    DtDrLock lock(&m_mutex);
    VER("DtDrDataSink::%s(%p, %d)", __FUNCTION__, pData, length);
    if (pData == NULL || length <= 0) {
        ERR("DtDrMySqlManager::%s() pData is NULL",__FUNCTION__);
        return FALSE;
    }
    DtDrBuffer* pBuffer = new DtDrBuffer();
    pBuffer->Add(pData, length);
    m_pBufferArray->PushBack(pBuffer);

    DtDrMySqlManagerMessage message;
    message.m_message = DtDrMySqlManagerMessage::SET_BUFFER;
    message.m_sequence = bRegister;
    INFO("bRegister:%d",bRegister);
    m_pMessageQueue->Send(&message);
    return TRUE;
}

void DtDrMySqlManager::Stop()
{
    VER("DtDrMySqlManager::%s()", __FUNCTION__);
    m_running = FALSE;
    DtDrMySqlManagerMessage message;
    message.m_message = DtDrMySqlManagerMessage::CLOSE;
    m_pMessageQueue->Send(&message);
}

void* DtDrMySqlManager::Run( void* pArg )
{
    VER("DtDrMySqlManager::%s()", __FUNCTION__);
    DtDrMySqlManagerMessage message;
    while (m_running) {
        m_pMessageQueue->Recv(&message);
        if (message.m_message == DtDrMySqlManagerMessage::CLOSE) {
            break;
        } else if (message.m_message == DtDrMySqlManagerMessage::SET_PACKET) {
            DtDrLock lock(&m_mutex);
            if (m_pPacketArray->IsEmpty()) {
                ERR("m_pPacketArray is Empty");
                continue;
            }
            DtDrPacketProtocolParser* pPacketProtocol = m_pPacketArray->RemoveFront();
            if (pPacketProtocol == NULL) {
                ERR("pPacketProtocol is NULL.");
                continue;
            }
            DtDrPacket* pPacket = new DtDrPacket();
            if (pPacketProtocol->GetPacketStream(pPacket) == FALSE) {
                ERR("pPacketProtocol->GetPacketStream() error");
                delete pPacket;
                continue;
            }
            if (pPacketProtocol->IsValidPacket() == FALSE) {
                continue;
            }
            DtDrBuffer* pDevice = new DtDrBuffer();
            pDevice->Add(pPacket->device);
            pDevice->Add("\0", 1);
            DtDrMySqlDataTable* pDataTable = m_pTableManager->GetDataTable(pDevice);
            if (pDataTable && pDataTable->GetCount() >= DTDR_TABLE_MAX_ROW_LENGTH) {
                pDataTable->ExecuteMultipleInsert();
                char buffer[32];
                memset(buffer, 0, sizeof(buffer));
                DtDrTimeStamp time = DtDrClock::GetTime();
                snprintf(buffer, sizeof(buffer) - 1, "%04d-%02d-%02d %02d:%02d:%02d.%06d",
                    time.m_date.tm_year + 1900,
                    time.m_date.tm_mon + 1,
                    time.m_date.tm_mday,
                    time.m_date.tm_hour,
                    time.m_date.tm_min,
                    time.m_date.tm_sec,
                    time.m_msec.tv_usec
                );
                m_pTableManager->Update(buffer, pDataTable->GetTableName()->GetBuffer());
                m_pTableManager->DeleteDataTable(pDevice);
                pDataTable = NULL;
            }
            if (pDataTable == NULL) {
                DtDrTimeStamp time = DtDrClock::GetTime();
                char tablename[128 + 1];
                memset(tablename, 0, sizeof(tablename));
                snprintf(tablename, sizeof(tablename) - 1, "datatbl_%s_%04d%02d%02d%02d%02d%02d%06d",
                    pPacket->device,
                    time.m_date.tm_year + 1900,
                    time.m_date.tm_mon + 1,
                    time.m_date.tm_mday,
                    time.m_date.tm_hour,
                    time.m_date.tm_min,
                    time.m_date.tm_sec,
                    time.m_msec.tv_usec
                );

                INFO("create new table(%s):%s", tablename, pPacket->device);
                pDataTable = new DtDrMySqlDataTable(m_pConnector, tablename, pPacket->device);
                m_pTableManager->SetDataTable(pDevice, pDataTable);
                int id = m_pTableManager->Insert(tablename, pPacket->device);
            }
//            pDataTable->Insert(pPacket->buffer, pPacket->device);
            pDataTable->StackMultipleInsert(pPacket->buffer, pPacket->device);
            if (pDataTable->GetStackCount() >= DTDR_TABLE_MAX_STACK_ROW_LENGTH) {
                pDataTable->ExecuteMultipleInsert();
            }
            delete pPacket;
            delete pPacketProtocol;
            delete pDevice;
            pDevice = NULL;
            pPacket = NULL;

        } else if (message.m_message == DtDrMySqlManagerMessage::SET_BUFFER) {
            DtDrLock lock(&m_mutex);
            if (m_pBufferArray->IsEmpty()) {
                ERR("DtDrMySqlManager::%s() pBuffer is Empty", __FUNCTION__);
                continue;
            }
            DtDrBuffer* pBuffer = m_pBufferArray->RemoveFront();
            if (pBuffer == NULL) {
                ERR("DtDrMySqlManager::%s() pBuffer is NULL", __FUNCTION__);
                continue;
            }

            char device[32 + 1];
            memset(device, 0, sizeof(device));
            char* pComma = pBuffer->GetBuffer();
            int comma = 0;
            for (comma = 0;comma < 32;comma++) {
                if (pComma[comma] == ',') {
                    break;
                }
                device[comma] = pComma[comma];
            }
            device[comma] = '\0';

            DtDrBuffer* pDevice = new DtDrBuffer();
            pDevice->Add(device, comma);
            pDevice->Add("\0", 1);
            INFO("bRegister:%d",message.m_sequence);
            if (!message.m_sequence) {
                INFO("unregistered data(%s):%s", device, pBuffer->GetBuffer() + comma + 1);
                continue;
            }
            DtDrMySqlDataTable* pDataTable = m_pTableManager->GetDataTable(pDevice);
            if (pDataTable && pDataTable->GetCount() >= DTDR_TABLE_MAX_ROW_LENGTH) {
                char buffer[32];
                memset(buffer, 0, sizeof(buffer));
                DtDrTimeStamp time = DtDrClock::GetTime();
                snprintf(buffer, sizeof(buffer) - 1, "%04d-%02d-%02d %02d:%02d:%02d.%06d",
                    time.m_date.tm_year + 1900,
                    time.m_date.tm_mon + 1,
                    time.m_date.tm_mday,
                    time.m_date.tm_hour,
                    time.m_date.tm_min,
                    time.m_date.tm_sec,
                    time.m_msec.tv_usec
                );
                m_pTableManager->Update(buffer, pDataTable->GetTableName()->GetBuffer());
                m_pTableManager->DeleteDataTable(pDevice);
                pDataTable = NULL;
            }
            if (pDataTable == NULL) {
                DtDrTimeStamp time = DtDrClock::GetTime();
                char tablename[128 + 1];
                memset(tablename, 0, sizeof(tablename));
                snprintf(tablename, sizeof(tablename) - 1, "datatbl_%s_%04d%02d%02d%02d%02d%02d%06d",
                    device,
                    time.m_date.tm_year + 1900,
                    time.m_date.tm_mon + 1,
                    time.m_date.tm_mday,
                    time.m_date.tm_hour,
                    time.m_date.tm_min,
                    time.m_date.tm_sec,
                    time.m_msec.tv_usec
                );

                INFO("create new table(%s):%s", tablename, device);
                pDataTable = new DtDrMySqlDataTable(m_pConnector, tablename, device);
                m_pTableManager->SetDataTable(pDevice, pDataTable);
                int id = m_pTableManager->Insert(tablename, device);
            }

            if (strcmp(pDataTable->GetDeviceName()->GetBuffer(), device) != 0) {
                ERR("insert into %s:%s %s",
                    pDataTable->GetDeviceName()->GetBuffer(),
                    device,
                    pDataTable->GetTableName()->GetBuffer()
                );
            }
            pDataTable->Insert(pBuffer->GetBuffer() + comma + 1, device);
            delete pDevice;
            delete pBuffer;
        }

    }
    return NULL;

}

