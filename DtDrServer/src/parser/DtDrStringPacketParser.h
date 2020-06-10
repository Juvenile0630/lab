#ifndef __DTDR_STRING_PACKET_PARSER_H_WAS_INCLUDED__
#define __DTDR_STRING_PACKET_PARSER_H_WAS_INCLUDED__

#include "DtDrPacketProtocolParser.h"
#include "DtDrTypes.h"
#include "DtDrBuffer.h"

class DtDrStringPacketParser : public DtDrPacketProtocolParser
{
public:
    DtDrStringPacketParser();
    virtual ~DtDrStringPacketParser();
    BOOL SetBuffer( char* pBuffer, int length, int* pReadedLength );
    BOOL IsValidPacket(){ return !m_hasError; };
    BOOL GetPacketStream( DtDrPacket* pResult );
private:
    BOOL parse();
    BOOL m_bComplete;
    DtDrPacket* m_pPacketData;
};

#endif // __DTDR_STRING_PACKET_PARSER_H_WAS_INCLUDED__
