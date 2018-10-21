#include "ProxyProcess.h"

CProxyProcess::CProxyProcess()
{
	printf("CProxyProcess construct.\r\n");
}

CProxyProcess::~CProxyProcess()
{
	printf("CProxyProcess destruct.\r\n");
}

int CProxyProcess::init(const string &strCfg, void* arg)
{
	printf("CProxyProcess init.\r\n");
	return 0;
}

int CProxyProcess::close(uint64_t flow, void* arg1, void* arg2)
{
	printf("CProxyProcess close.\r\n");
	return 0;
}

int CProxyProcess::input(uint64_t flow, void* arg1, void* arg2)
{
	printf("CProxyProcess input.\r\n");
	transmit_data *pData = (transmit_data*)arg1;
	
	return pData->len;
}

int CProxyProcess::route(uint64_t flow, void* arg1, void* arg2)
{
	printf("CProxyProcess route.\r\n");
	return 1;
}

int CProxyProcess::exception(uint64_t flow, void* arg1, void* arg2)
{
    printf("CProxyProcess exception.\r\n");
    return 0;	
}

int CProxyProcess::fini(void* arg)
{
	printf("CProxyProcess fini\r\n");
	return 0;
}


	
extern "C" IProxyProcess* createProxy()
{
	return new CProxyProcess();
}

