#include "DtDrDataSink.h"
#include "DtDrLog.h"
#include <unistd.h>

DtDrDataSink::DtDrDataSink() :
    m_pPacketArray(new DtDrList<DtDrPacketProtocolParser*>()),
    m_pBufferArray(new DtDrList<DtDrBuffer*>()),
    m_mutex()
{
    VER("DtDrDataSink::%s()", __FUNCTION__);
}

DtDrDataSink::~DtDrDataSink()
{
    VER("DtDrDataSink::%s()", __FUNCTION__);
    m_running = FALSE;
    Stop();
    while (!m_pPacketArray->IsEmpty()) {
        DtDrPacketProtocolParser* pPacket = m_pPacketArray->RemoveFront();
        delete pPacket;
    }
    while (!m_pBufferArray->IsEmpty()) {
        DtDrBuffer* pBuffer = m_pBufferArray->RemoveFront();
        delete pBuffer;
    }
    delete m_pPacketArray;
    m_pPacketArray = NULL;
    delete m_pBufferArray;
    m_pBufferArray = NULL;
}

BOOL DtDrDataSink::SetBuffer( char* pData, int length, BOOL bRegister )
{
    DtDrLock lock(&m_mutex);
    VER("DtDrDataSink::%s(%p, %d)", __FUNCTION__, pData, length);
    if (pData == NULL || length <= 0) {
        return FALSE;
    }
    DtDrBuffer* pBuffer = new DtDrBuffer();
    pBuffer->Add(pData, length);
    m_pBufferArray->PushBack(pBuffer);
    return TRUE;
}

BOOL DtDrDataSink::Initialize()
{
    VER("DtDrDataSink::%s()", __FUNCTION__);
    return TRUE;
}

void* DtDrDataSink::Run( void* pArg )
{
    VER("DtDrDataSink::%s()", __FUNCTION__);
    return NULL;
}

