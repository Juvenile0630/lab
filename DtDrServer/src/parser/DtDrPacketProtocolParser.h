#ifndef __DTDR_PACKET_PROTOCOL_PARSER_H_WAS_INCLUDED__
#define __DTDR_PACKET_PROTOCOL_PARSER_H_WAS_INCLUDED__

#include "DtDrTypes.h"
#include "DtDrBuffer.h"

struct DtDrPacket {
    char prefix[8];     // (8)
    char device[32];    // (32)
    unsigned short ch1; // (5)
    unsigned short ch2; // (5)
    unsigned short ch3; // (5)
    unsigned short ch4; // (5) 65535
    unsigned long ecg;  // (10) 4294967295
    double pri;         // (15)
    double heartrate;   // (15)
    double latitude;    // (15)
    double longitude;   // (15)
    double gyro1;       // (15)
    double gyro2;       // (15)
    double gyro3;       // (15)
    double axel1;       // (15)
    double axel2;       // (15)
    double axel3;       // (15)
    double mgnt1;       // (15)
    double mgnt2;       // (15)
    double mgnt3;       // (15)
    char mdate[32];     // (32):(26)YYYY.MM.DD HH:ii:ss.mmmuuu
    char udate[32];     // (32):(26)YYYY.MM.DD HH:ii:ss.mmmuuu
    char connection[8]; // (8)
    char buffer[1024];  // 8+32+5+5+5+5+10+15+15+15+15+15+15+15+15+15+15+15+15+15+32+32+8+21
};

class DtDrPacketProtocolParser
{
public:
    DtDrPacketProtocolParser();
    virtual ~DtDrPacketProtocolParser();
    virtual BOOL SetBuffer( char* pBuffer, int length, int* pReadedLength ){ return TRUE; };
    virtual BOOL IsValidPacket(){ return m_hasError; };
    virtual BOOL GetPacketStream( DtDrPacket* pResult ){ return TRUE; };
protected:
    DtDrBuffer* m_pBuffer;
    BOOL m_hasError;
};



#endif // __DTDR_PACKET_PROTOCOL_PARSER_H_WAS_INCLUDED__
