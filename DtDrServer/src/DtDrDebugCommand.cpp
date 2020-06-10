#include "DtDrDebugCommand.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "DtDrSemaphore.h"
#include "DtDrSharedMemory.h"


#define LINE_LENGTH 10

DtDrDebugCommand* DtDrDebugCommand::m_pThis = NULL;

DtDrDebugCommand* DtDrDebugCommand::GetInstance()
{
    if (m_pThis == NULL) {
        m_pThis = new DtDrDebugCommand();
    }
    return m_pThis;
}

DtDrDebugCommand::DtDrDebugCommand()
{
}

DtDrDebugCommand::~DtDrDebugCommand()
{
}

void* DtDrDebugCommand::Run( void* pArg )
{
    DtDrSemaphore* pSemaphore = new DtDrSemaphore();
    pSemaphore->Open("/SDtDrDebugCommand", TRUE);
    DtDrSharedMemory* pSharedMemory = new DtDrSharedMemory();
    pSharedMemory->Open("/MDtDrDebugCommand", TRUE, TRUE);
    DtDrBuffer* pDebugCommand = new DtDrBuffer();

    DtDrDebugCommandCallback* pCallback = reinterpret_cast<DtDrDebugCommandCallback*>(pArg);
    if (pArg == NULL) {
        return NULL;
    }
    char buffer[LINE_LENGTH];
    char* pBuffer = NULL;
    int length = 0;
    fprintf(stdout, "$ ");
    while (m_running) {
//        usleep(1 * 1000);
        pSemaphore->Wait();
        pDebugCommand->Clear();
        if (pSharedMemory->Read(pDebugCommand) == FALSE) {
            continue;
        }
        INFO("$ %s", pDebugCommand->GetBuffer());
/*
        memset(buffer, 0, sizeof(char) * LINE_LENGTH);
        fgets(buffer, sizeof(char) * LINE_LENGTH, stdin);
        int returnIndex = -1;
        for (int i = 0;i < LINE_LENGTH;i++) {
            if (buffer[i] == '\n') {
                returnIndex = i;
                break;
            }
        }
        int bufferLength = returnIndex;
        if (returnIndex == -1) {
            bufferLength = LINE_LENGTH;
        }
        char* pBufferTmp = new char[length + bufferLength + 1];
        memset(pBufferTmp, 0, length + bufferLength + 1);
        memcpy(pBufferTmp, pBuffer, length);
        delete pBuffer;

        memcpy(pBufferTmp + length, buffer, bufferLength);
        pBuffer = pBufferTmp;
        if (returnIndex == -1) {
            length += bufferLength - 1;
            continue;
        }
        fprintf(stdout, "$ ");
*/
        m_running = pCallback->DebugCommand(pDebugCommand->GetBuffer());
        length = 0;
        delete[] pBuffer;
        pBuffer = NULL;
    }
    fprintf(stdout, "\n");
    pSharedMemory->Detach();
    pSharedMemory->Close();
    delete pSharedMemory;
    delete pSemaphore;
    delete pDebugCommand;
    return pArg;
}

