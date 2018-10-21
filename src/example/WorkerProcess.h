#ifndef __WORKER_PROCESS_HEAD_H__
#define __WORKER_PROCESS_HEAD_H__

#include "IAcceptor.h"
#include "IProcess.h"
#include <stdio.h>

class CWorkerProcess : public IWorkerProcess
{
public:
    CWorkerProcess();
	
	~CWorkerProcess();
	
	int init(const string &strCfg, void* arg);
	
	int fini(void* arg);
	
	int process(uint64_t flow, void* arg1, void* arg2);
};

#endif
