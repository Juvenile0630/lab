#ifndef __DTDR_PACKET_PARSER_H_WAS_INCLUDED__
#define __DTDR_PACKET_PARSER_H_WAS_INCLUDED__

#include "DtDrBuffer.h"
#include "DtDrList.h"
#include "DtDrDataSink.h"
#include "DtDrLock.h"

class DtDrPacketAnalyzer
{
public:
    DtDrPacketAnalyzer();
    virtual ~DtDrPacketAnalyzer();
    BOOL Initialize();
    BOOL SetBuffer(char* pBuffer, int length, int* pReadedLength);
    DtDrPacketProtocolParser* GetPacketParser(){ return m_pProtocolParser; };

private:
    DtDrMutex m_mutex;
    DtDrDataSink* m_pStringDataSink;
    DtDrPacketProtocolParser* m_pProtocolParser;
    DtDrBuffer* m_pBuffer;
};

#endif // __DTDR_PACKET_PARSER_H_WAS_INCLUDED__

