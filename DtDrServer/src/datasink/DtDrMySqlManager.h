#ifndef __DTDR_MYSQL_MANAGER_H_WAS_INCLUDED__
#define __DTDR_MYSQL_MANAGER_H_WAS_INCLUDED__

#include "DtDrTypes.h"
#include "DtDrDataSink.h"
#include "DtDrMySqlConnector.h"
#include "DtDrMySqlTableManager.h"
#include "DtDrMySqlTableMasterRecord.h"
#include "DtDrMySqlDataTable.h"
#include "DtDrMessageQueue.h"


class DtDrMySqlManager : public DtDrDataSink
{
public:
    DtDrMySqlManager();
    virtual ~DtDrMySqlManager();
    virtual BOOL SetPacket( DtDrPacketProtocolParser* pPacketProtocolParser );
    virtual BOOL SetBuffer( char* pBuffer, int length, BOOL bRegister );
    virtual void Stop( );
    virtual void* Run( void* pArg );
    BOOL Initialize();

private:
    class DtDrMySqlManagerMessage : public DtDrMessage {
    public:
        DtDrMySqlManagerMessage() {}
        virtual ~DtDrMySqlManagerMessage() {}
        enum {
            SET_PACKET,
            SET_BUFFER,
            CLOSE,
        };
    };
    DtDrMySqlConnector* m_pConnector;
    DtDrMySqlTableManager* m_pTableManager;
    DtDrMessageQueue* m_pMessageQueue;
};


#endif // __DTDR_MYSQL_MANAGER_H_WAS_INCLUDED__