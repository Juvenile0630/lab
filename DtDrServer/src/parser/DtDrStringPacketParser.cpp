#include "DtDrStringPacketParser.h"
#include "DtDrLog.h"

DtDrStringPacketParser::DtDrStringPacketParser() :
    m_bComplete(FALSE),
    m_pPacketData(new DtDrPacket())
{
}

DtDrStringPacketParser::~DtDrStringPacketParser()
{
    delete m_pPacketData;
}

BOOL DtDrStringPacketParser::SetBuffer( char* pBuffer, int length, int* pReadedLength )
{
    DtDrBuffer* pString = new DtDrBuffer();
    pString->Add(pBuffer, length);
    pString->Add("\0", 1);
    VER("DtDrStringPacketParser::%s(%p, %p, \"%s\")", __FUNCTION__, pBuffer, pReadedLength, pString->GetBuffer());
    delete pString;
    if (m_bComplete) { return m_bComplete; }
    *pReadedLength = length;
    for (int i = 0;i < length;i++) {
        if (*(pBuffer + i) == ';') {
            *pReadedLength = i;
            m_bComplete = TRUE;
            break;
        } else if (*(pBuffer + i) < 0x20 || *(pBuffer + i) > 0x7E) {
            ERR("DtDrStringPacketParser::%s() Unknown Packet data[%d]:0x%02X", __FUNCTION__, i, *(pBuffer + i));
            m_hasError = TRUE;
        }
    }
    m_pBuffer->Add(pBuffer, *pReadedLength);
    if (m_bComplete) {
        m_pBuffer->Add("\0", 1);
        if (parse() == FALSE) {
            m_hasError = TRUE;
        }
        *pReadedLength += 1;
    }
    VER("return(%d, %d):%d", m_pBuffer->GetLength(), *pReadedLength, m_bComplete);
    return m_bComplete;
}

BOOL DtDrStringPacketParser::GetPacketStream( DtDrPacket* pResult )
{
    VER("DtDrStringPacketParser::%s()", __FUNCTION__);
    memcpy(pResult, m_pPacketData, sizeof(DtDrPacket));
    return TRUE;
}

BOOL DtDrStringPacketParser::parse()
{
    char* pSavePtr = NULL;
    char* pBuffer = NULL;
    char pAnalyze[m_pBuffer->GetLength()];
    memcpy(pAnalyze, m_pBuffer->GetBuffer(), m_pBuffer->GetLength());
    pBuffer = strtok_r(pAnalyze, ",", &pSavePtr);
    if (pBuffer != NULL) {
        BOOL bPre = TRUE;
        int cntDev = 0;
        memset(m_pPacketData->prefix, 0, sizeof(m_pPacketData->prefix));
        memset(m_pPacketData->device, 0, sizeof(m_pPacketData->device));
        int length = strlen(pBuffer);
        for (int i = 0;i < length;i++) {
            if (*pBuffer == ':') {
                bPre = FALSE;
                pBuffer++;
            } else if (i < sizeof(m_pPacketData->prefix) && bPre == TRUE && *pBuffer != ':') {
                m_pPacketData->prefix[i] = *pBuffer++;
            } else if (cntDev < sizeof(m_pPacketData->device) && bPre == FALSE ) {
                m_pPacketData->device[cntDev++] = *pBuffer++;
            }
        }
    } else {
        ERR("DtDrStringPacketParser::%s():%d pBuffer is NULL", __FUNCTION__, __LINE__);
        return FALSE;
    }


    pBuffer = strtok_r(NULL, ",", &pSavePtr);
    if (pBuffer != NULL) {
        char* pEndBuffer = NULL;
        m_pPacketData->ch1 = static_cast<unsigned short>(strtol(pBuffer, &pEndBuffer, 10));
    } else {
        ERR("DtDrStringPacketParser::%s():%d pBuffer is NULL.(%d):%s", __FUNCTION__, __LINE__, m_pBuffer->GetLength(), m_pBuffer->GetBuffer());
        return FALSE;
    }

    pBuffer = strtok_r(NULL, ",", &pSavePtr);
    if (pBuffer != NULL) {
        char* pEndBuffer = NULL;
        m_pPacketData->ch2 = static_cast<unsigned short>(strtol(pBuffer, &pEndBuffer, 10));
    } else {
        ERR("DtDrStringPacketParser::%s():%d pBuffer is NULL.(%d):%s", __FUNCTION__, __LINE__, m_pBuffer->GetLength(), m_pBuffer->GetBuffer());
        return FALSE;
    }

    pBuffer = strtok_r(NULL, ",", &pSavePtr);
    if (pBuffer != NULL) {
        char* pEndBuffer = NULL;
        m_pPacketData->ch3 = static_cast<unsigned short>(strtol(pBuffer, &pEndBuffer, 10));
    } else {
        ERR("DtDrStringPacketParser::%s():%d pBuffer is NULL.(%d):%s", __FUNCTION__, __LINE__, m_pBuffer->GetLength(), m_pBuffer->GetBuffer());
        return FALSE;
    }

    pBuffer = strtok_r(NULL, ",", &pSavePtr);
    if (pBuffer != NULL) {
        char* pEndBuffer = NULL;
        m_pPacketData->ch4 = static_cast<unsigned short>(strtol(pBuffer, &pEndBuffer, 10));
    } else {
        ERR("DtDrStringPacketParser::%s():%d pBuffer is NULL.(%d):%s", __FUNCTION__, __LINE__, m_pBuffer->GetLength(), m_pBuffer->GetBuffer());
        return FALSE;
    }

    pBuffer = strtok_r(NULL, ",", &pSavePtr);
    if (pBuffer != NULL) {
        char* pEndBuffer = NULL;
        m_pPacketData->ecg = static_cast<unsigned short>(strtol(pBuffer, &pEndBuffer, 10));
    } else {
        ERR("DtDrStringPacketParser::%s():%d pBuffer is NULL.(%d):%s", __FUNCTION__, __LINE__, m_pBuffer->GetLength(), m_pBuffer->GetBuffer());
        return FALSE;
    }

    pBuffer = strtok_r(NULL, ",", &pSavePtr);
    if (pBuffer != NULL) {
        char* pEndBuffer = NULL;
        m_pPacketData->pri = strtod(pBuffer, &pEndBuffer);
    } else {
        ERR("DtDrStringPacketParser::%s():%d pBuffer is NULL.(%d):%s", __FUNCTION__, __LINE__, m_pBuffer->GetLength(), m_pBuffer->GetBuffer());
        return FALSE;
    }

    pBuffer = strtok_r(NULL, ",", &pSavePtr);
    if (pBuffer != NULL) {
        char* pEndBuffer = NULL;
        m_pPacketData->heartrate = strtod(pBuffer, &pEndBuffer);
    } else {
        ERR("DtDrStringPacketParser::%s():%d pBuffer is NULL.(%d):%s", __FUNCTION__, __LINE__, m_pBuffer->GetLength(), m_pBuffer->GetBuffer());
        return FALSE;
    }

    pBuffer = strtok_r(NULL, ",", &pSavePtr);
    if (pBuffer != NULL) {
        char* pEndBuffer = NULL;
        m_pPacketData->latitude = strtod(pBuffer, &pEndBuffer);
    } else {
        ERR("DtDrStringPacketParser::%s():%d pBuffer is NULL.(%d):%s", __FUNCTION__, __LINE__, m_pBuffer->GetLength(), m_pBuffer->GetBuffer());
        return FALSE;
    }

    pBuffer = strtok_r(NULL, ",", &pSavePtr);
    if (pBuffer != NULL) {
        char* pEndBuffer = NULL;
        m_pPacketData->longitude = strtod(pBuffer, &pEndBuffer);
    } else {
        ERR("DtDrStringPacketParser::%s():%d pBuffer is NULL.(%d):%s", __FUNCTION__, __LINE__, m_pBuffer->GetLength(), m_pBuffer->GetBuffer());
        return FALSE;
    }

    pBuffer = strtok_r(NULL, ",", &pSavePtr);
    if (pBuffer != NULL) {
        char* pEndBuffer = NULL;
        m_pPacketData->gyro1 = strtod(pBuffer, &pEndBuffer);
    } else {
        ERR("DtDrStringPacketParser::%s():%d pBuffer is NULL.(%d):%s", __FUNCTION__, __LINE__, m_pBuffer->GetLength(), m_pBuffer->GetBuffer());
        return FALSE;
    }

    pBuffer = strtok_r(NULL, ",", &pSavePtr);
    if (pBuffer != NULL) {
        char* pEndBuffer = NULL;
        m_pPacketData->gyro2 = strtod(pBuffer, &pEndBuffer);
    } else {
        ERR("DtDrStringPacketParser::%s():%d pBuffer is NULL.(%d):%s", __FUNCTION__, __LINE__, m_pBuffer->GetLength(), m_pBuffer->GetBuffer());
        return FALSE;
    }

    pBuffer = strtok_r(NULL, ",", &pSavePtr);
    if (pBuffer != NULL) {
        char* pEndBuffer = NULL;
        m_pPacketData->gyro3 = strtod(pBuffer, &pEndBuffer);
    } else {
        ERR("DtDrStringPacketParser::%s():%d pBuffer is NULL.(%d):%s", __FUNCTION__, __LINE__, m_pBuffer->GetLength(), m_pBuffer->GetBuffer());
        return FALSE;
    }

    pBuffer = strtok_r(NULL, ",", &pSavePtr);
    if (pBuffer != NULL) {
        char* pEndBuffer = NULL;
        m_pPacketData->axel1 = strtod(pBuffer, &pEndBuffer);
    } else {
        ERR("DtDrStringPacketParser::%s():%d pBuffer is NULL.(%d):%s", __FUNCTION__, __LINE__, m_pBuffer->GetLength(), m_pBuffer->GetBuffer());
        return FALSE;
    }

    pBuffer = strtok_r(NULL, ",", &pSavePtr);
    if (pBuffer != NULL) {
        char* pEndBuffer = NULL;
        m_pPacketData->axel2 = strtod(pBuffer, &pEndBuffer);
    } else {
        ERR("DtDrStringPacketParser::%s():%d pBuffer is NULL.(%d):%s", __FUNCTION__, __LINE__, m_pBuffer->GetLength(), m_pBuffer->GetBuffer());
        return FALSE;
    }

    pBuffer = strtok_r(NULL, ",", &pSavePtr);
    if (pBuffer != NULL) {
        char* pEndBuffer = NULL;
        m_pPacketData->axel3 = strtod(pBuffer, &pEndBuffer);
    } else {
        ERR("DtDrStringPacketParser::%s():%d pBuffer is NULL.(%d):%s", __FUNCTION__, __LINE__, m_pBuffer->GetLength(), m_pBuffer->GetBuffer());
        return FALSE;
    }

    pBuffer = strtok_r(NULL, ",", &pSavePtr);
    if (pBuffer != NULL) {
        char* pEndBuffer = NULL;
        m_pPacketData->mgnt1 = strtod(pBuffer, &pEndBuffer);
    } else {
        ERR("DtDrStringPacketParser::%s():%d pBuffer is NULL.(%d):%s", __FUNCTION__, __LINE__, m_pBuffer->GetLength(), m_pBuffer->GetBuffer());
        return FALSE;
    }

    pBuffer = strtok_r(NULL, ",", &pSavePtr);
    if (pBuffer != NULL) {
        char* pEndBuffer = NULL;
        m_pPacketData->mgnt2 = strtod(pBuffer, &pEndBuffer);
    } else {
        ERR("DtDrStringPacketParser::%s():%d pBuffer is NULL.(%d):%s", __FUNCTION__, __LINE__, m_pBuffer->GetLength(), m_pBuffer->GetBuffer());
        return FALSE;
    }

    pBuffer = strtok_r(NULL, ",", &pSavePtr);
    if (pBuffer != NULL) {
        char* pEndBuffer = NULL;
        m_pPacketData->axel3 = strtod(pBuffer, &pEndBuffer);
    } else {
        ERR("DtDrStringPacketParser::%s():%d pBuffer is NULL.(%d):%s", __FUNCTION__, __LINE__, m_pBuffer->GetLength(), m_pBuffer->GetBuffer());
        return FALSE;
    }

    pBuffer = strtok_r(NULL, ",", &pSavePtr);
    if (pBuffer != NULL) {
        memset(m_pPacketData->mdate, 0, sizeof(m_pPacketData->mdate));
        strncpy(m_pPacketData->mdate, pBuffer, sizeof(m_pPacketData->mdate) - 1);
    } else {
        ERR("DtDrStringPacketParser::%s():%d pBuffer is NULL.(%d):%s", __FUNCTION__, __LINE__, m_pBuffer->GetLength(), m_pBuffer->GetBuffer());
        return FALSE;
    }

    pBuffer = strtok_r(NULL, ",", &pSavePtr);
    if (pBuffer != NULL) {
        memset(m_pPacketData->udate, 0, sizeof(m_pPacketData->udate));
        strncpy(m_pPacketData->udate, pBuffer, sizeof(m_pPacketData->udate) - 1);
    } else {
        ERR("DtDrStringPacketParser::%s():%d pBuffer is NULL.(%d):%s", __FUNCTION__, __LINE__, m_pBuffer->GetLength(), m_pBuffer->GetBuffer());
        return FALSE;
    }

    pBuffer = strtok_r(NULL, ",", &pSavePtr);
    if (pBuffer != NULL) {
        memset(m_pPacketData->connection, 0, sizeof(m_pPacketData->connection));
        strncpy(m_pPacketData->connection, pBuffer, sizeof(m_pPacketData->connection) - 1);
    } else {
        ERR("DtDrStringPacketParser::%s():%d pBuffer is NULL.(%d):%s", __FUNCTION__, __LINE__, m_pBuffer->GetLength(), m_pBuffer->GetBuffer());
        return FALSE;
    }

    memset(m_pPacketData->buffer, 0, sizeof(m_pPacketData->buffer));
    snprintf(m_pPacketData->buffer, sizeof(m_pPacketData->buffer) - 1,
        "%d,%d,%d,%d,%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%s,%s,%s",
        m_pPacketData->ch1,
        m_pPacketData->ch2,
        m_pPacketData->ch3,
        m_pPacketData->ch4,
        m_pPacketData->ecg,
        m_pPacketData->pri,
        m_pPacketData->heartrate,
        m_pPacketData->latitude,
        m_pPacketData->longitude,
        m_pPacketData->gyro1,
        m_pPacketData->gyro2,
        m_pPacketData->gyro3,
        m_pPacketData->axel1,
        m_pPacketData->axel2,
        m_pPacketData->axel3,
        m_pPacketData->mgnt1,
        m_pPacketData->mgnt2,
        m_pPacketData->mgnt3,
        m_pPacketData->mdate,
        m_pPacketData->udate,
        m_pPacketData->connection
    );
    DET("DtDrStringPacketParser::%s(%s, %s, %s, %p)", __FUNCTION__, m_pPacketData->prefix, m_pPacketData->device, m_pPacketData->buffer, m_pPacketData->buffer);

    return TRUE;
}
