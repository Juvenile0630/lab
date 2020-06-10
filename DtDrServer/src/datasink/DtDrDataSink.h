#ifndef __DTDR_DATA_SINK_H_WAS_INCLUDED__
#define __DTDR_DATA_SINK_H_WAS_INCLUDED__

#include "DtDrThread.h"
#include "DtDrLock.h"
#include "DtDrList.h"
#include "DtDrBuffer.h"
#include "DtDrPacketProtocolParser.h"

class DtDrDataSink : public DtDrThread
{
public:
    DtDrDataSink();
    virtual ~DtDrDataSink();
    virtual BOOL SetPacket( DtDrPacketProtocolParser* pPacket ){ return TRUE; };
    virtual BOOL SetBuffer( char* pBuffer, int length, BOOL bRegister = TRUE );
    virtual void* Run( void* pArg );
    virtual BOOL Initialize();

protected:
    DtDrList<DtDrPacketProtocolParser*>* m_pPacketArray;
    DtDrList<DtDrBuffer*>* m_pBufferArray;
    DtDrMutex m_mutex;
};

#endif // __DTDR_DATA_SINK_H_WAS_INCLUDED__

