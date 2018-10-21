#include "WorkerProcess.h"
#include "ShmAcceptor.h"

CWorkerProcess::CWorkerProcess()
{
	printf("CWorkerProcess construct.\r\n");
}
	
CWorkerProcess::~CWorkerProcess()
{
	printf("CWorkerProcess destruct.\r\n");
}
	
int CWorkerProcess::init(const string &strCfg, void* arg)
{
	printf("CWorkerProcess init file:%s\r\n", strCfg.c_str());
	
	return 0;
}
	
int CWorkerProcess::fini(void* arg)
{
	printf("CWorkerProcess fini.\r\n");
	
	return 0;
}
	
int CWorkerProcess::process(uint64_t flow, void* arg1, void* arg2)
{
	printf("CWorkerProcess process.\r\n");
	
	transmit_data *pData = (transmit_data*)arg1;
	ShmAcceptor *pShmAcceptor = (ShmAcceptor*)pData->owner;
	
	pData->extdata = NULL;
	
	pShmAcceptor->sendto(flow, arg1, NULL);
	
	printf("Receive data:%s\r\n", string(pData->data, pData->len).c_str());
	
	return 0;
}

extern "C" IWorkerProcess* createWorker()
{
	return new CWorkerProcess();
}
