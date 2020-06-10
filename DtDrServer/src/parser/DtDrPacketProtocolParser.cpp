#include "DtDrPacketProtocolParser.h"

DtDrPacketProtocolParser::DtDrPacketProtocolParser() :
    m_pBuffer(new DtDrBuffer()),
    m_hasError(FALSE)
{
}

DtDrPacketProtocolParser::~DtDrPacketProtocolParser()
{
    delete m_pBuffer;
}

