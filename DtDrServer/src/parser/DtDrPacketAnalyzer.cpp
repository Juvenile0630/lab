#include "DtDrPacketAnalyzer.h"
#include "DtDrStringPacketParser.h"
#include "DtDrLog.h"
#include <unistd.h>
#include <string.h>

#define DTDR_PROTOCOL_DATA_HEADER_LENGTH    5
#define DTDR_PROTOCOL_DATA_HEADER_STRING    "DtDs:"
#define DTDR_PROTOCOL_DATA_HEADER_BINARY    "DtDb:"

DtDrPacketAnalyzer::DtDrPacketAnalyzer() :
    m_pBuffer(NULL),
    m_mutex(),
    m_pProtocolParser(NULL)
{
    VER("DtDrPacketAnalyzer::%s()", __FUNCTION__);
}

DtDrPacketAnalyzer::~DtDrPacketAnalyzer()
{
    VER("DtDrPacketAnalyzer::%s()", __FUNCTION__);
}

BOOL DtDrPacketAnalyzer::Initialize()
{
    m_pProtocolParser = NULL;
    return TRUE;
}

BOOL DtDrPacketAnalyzer::SetBuffer(char* pBuffer, int length, int* pReadedLength)
{
    VER("DtDrPacketAnalyzer::%s(%p, %d, %p)", __FUNCTION__, pBuffer, length, pReadedLength);
    if (pReadedLength == NULL) {
        ERR("DtDrPacketAnalyzer::%s() pReadedLength is NULL", __FUNCTION__);
        return FALSE;
    }
    int remainLength = 0;
    if (m_pBuffer != NULL) {
        remainLength = m_pBuffer->GetLength();
    }
    DtDrBuffer* pData = new DtDrBuffer();
    if (m_pProtocolParser == NULL) {
        if (length < DTDR_PROTOCOL_DATA_HEADER_LENGTH && m_pBuffer == NULL) {
            DET("length:%d", length);
            m_pBuffer = new DtDrBuffer();
            m_pBuffer->Add(pBuffer, length);
            *pReadedLength = length;
            return FALSE;
        } else if (m_pBuffer != NULL) {
            DET("m_pBuffer is not NULL(%d):%p", __LINE__, m_pBuffer);
            m_pBuffer->Add(pBuffer, length);
            if (m_pBuffer->GetLength() < DTDR_PROTOCOL_DATA_HEADER_LENGTH) {
                *pReadedLength = length;
                delete pData;
                return FALSE;
            }
            pData->Add(m_pBuffer->GetBuffer(), m_pBuffer->GetLength());
            delete m_pBuffer;
            m_pBuffer = NULL;
        } else {
            pData->Add(pBuffer, length);
        }
        if (memcmp(pData->GetBuffer(), DTDR_PROTOCOL_DATA_HEADER_STRING, DTDR_PROTOCOL_DATA_HEADER_LENGTH) == 0) {
            m_pProtocolParser = new DtDrStringPacketParser();
        } else {
            ERR("Unknown packet");
            delete pData;
            *pReadedLength = length;
            return TRUE;
        }
    } else {
        if (m_pBuffer != NULL) {
            DET("m_pBuffer is not NULL(%d):%p", __LINE__, m_pBuffer);
            pData->Add(m_pBuffer->GetBuffer(), m_pBuffer->GetLength());
            pData->Add(pBuffer, length);
            delete m_pBuffer;
            m_pBuffer = NULL;
        } else {
            pData->Add(pBuffer, length);
        }
    }

    BOOL bRet = m_pProtocolParser->SetBuffer(pData->GetBuffer(), pData->GetLength(), pReadedLength);
    delete pData;
    *pReadedLength -= remainLength;

    return bRet;
}

