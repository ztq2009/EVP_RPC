#include "DefaultWorker.h"

int main(int argc, char *argv[])
{
    try
    {
        CDefaultWorkerPtr pDefaultWorker = new CDefaultWorker();
        pDefaultWorker->start(argc, argv);
    }
    catch(CException &e)
    {
        LOG_ERROR("Default worker throw excepton, errMsg:%s", e.what());
    }
}

