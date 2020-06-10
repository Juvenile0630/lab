#include <stdio.h>
#include <stdlib.h>
#include "DtDrLog.h"
#include "DtDrManager.h"
#include "DtDrDebugCommand.h"


int main( int argc, char* argv[] )
{
//    CLOGLEVEL(LOG_LEVEL_WARNING);
//    CLOGLEVEL(LOG_LEVEL_DETAIL);
    CLOGLEVEL(LOG_LEVEL_WARNING);
//    LOGLEVEL(LOG_LEVEL_DETAIL);
    LOGLEVEL(LOG_LEVEL_INFORMATION);
    VER("test verbose");
    DET("test detail");
    INFO("test information");
    WARN("test Warning");
    ERR("test Error");
    FLUSH();

    DtDrManager *pManager = new DtDrManager();

    DtDrDebugCommand* pDebugCommand = DtDrDebugCommand::GetInstance();
    pDebugCommand->Start(pManager);

    pManager->Start(NULL);

    pManager->Join();
    pDebugCommand->Join();

    delete pManager;

    return 0;
}

