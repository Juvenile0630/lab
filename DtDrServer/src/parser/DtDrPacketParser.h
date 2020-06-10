#ifndef __DTDR_PACKET_PARSER_H_WAS_INCLUDED__
#define __DTDR_PACKET_PARSER_H_WAS_INCLUDED__

#include "DtDrBuffer.h"
#include "DtDrList.h"
#include "DtDrDataSink.h"
#include "DtDrLock.h"

struct DtDrProcotol {
    char prefix[8];
    char device[32];
    unsigned short ch1;
    unsigned short ch2;
    unsigned short ch3;
    unsigned short ch4;
    double latitude;
    double longitude;
    double gyro1;
    double gyro2;
    double gyro3;
    double axel1;
    double axel2;
    double axel3;
    double mgnt1;
    double mgnt2;
    double mgnt3;
    char mdate[32]; //(26)YYYY.MM.DD HH:ii:ss.mmmuuu
    char udate[32]; //(26)YYYY.MM.DD HH:ii:ss.mmmuuu
    char connection[8];
};

class DtDrPacketParser
{
public:
    DtDrPacketParser();
    virtual ~DtDrPacketParser();
    void SetBuffer( DtDrBuffer* pBuffer );

private:
    DtDrMutex m_mutex;
    DtDrDataSink* m_pStringDataSink;
};

#endif // __DTDR_PACKET_PARSER_H_WAS_INCLUDED__

