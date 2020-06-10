#include "DtDrPacketParser.h"
#include "DtDrLog.h"
#include <unistd.h>
#include <string.h>
#include "DtDrMySqlManager.h"

#define DTDR_PROTOCOL_DATA_HEADER_LENGTH    5
#define DTDR_PROTOCOL_DATA_HEADER_STRING    "DtDs:"
#define DTDR_PROTOCOL_DATA_HEADER_BINARY    "DtDb:"

DtDrPacketParser::DtDrPacketParser() :
    m_mutex()
{
    VER("DtDrPacketParser::%s()", __FUNCTION__);
    m_pStringDataSink = new DtDrMySqlManager();
    if (m_pStringDataSink == NULL) {
        ERR("DtDrPacketParser::%s() m_pStringDataSink is NULL", __FUNCTION__);
        return;
    }
    m_pStringDataSink->Initialize();
    m_pStringDataSink->Start();
}

DtDrPacketParser::~DtDrPacketParser()
{
    VER("DtDrPacketParser::%s()", __FUNCTION__);
    delete m_pStringDataSink;
}

void DtDrPacketParser::SetBuffer( DtDrBuffer* pBuffer )
{
    DtDrLock lock(&m_mutex);
    VER("DtDrPacketParser::%s(%p)", __FUNCTION__, pBuffer);

    if (memcmp(pBuffer->GetBuffer(), DTDR_PROTOCOL_DATA_HEADER_STRING, DTDR_PROTOCOL_DATA_HEADER_LENGTH) == 0) {
        pBuffer->Add("\0", 1);

        int pos = DTDR_PROTOCOL_DATA_HEADER_LENGTH;
        char* pPos = strstr(pBuffer->GetBuffer(), ")");
        if (pPos != NULL) {
            pos = pPos - pBuffer->GetBuffer();
            pos += 1;
        }
        m_pStringDataSink->SetBuffer(pBuffer->GetBuffer() + pos, pBuffer->GetLength() - pos, pos == DTDR_PROTOCOL_DATA_HEADER_LENGTH);
    } else if (memcmp(pBuffer->GetBuffer(), DTDR_PROTOCOL_DATA_HEADER_BINARY, DTDR_PROTOCOL_DATA_HEADER_LENGTH) == 0){ 
    } else {
        ERR("Unknown Buffer:%d", pBuffer->GetLength());
    }


}

