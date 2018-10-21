#include "DefaultProxy.h"

int main(int argc, char *argv[])
{
    try
    {
        CDefaultProxyPtr pDefaultProxy = new CDefaultProxy();
        pDefaultProxy->start(argc, argv);
    }
    catch(CException &e)
    {
        LOG_ERROR("DefaultProxy throw exception. ErrMSg:%s", e.what());
    }

    CLogHelper::destroy();
}


