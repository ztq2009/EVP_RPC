#ifndef __PROXY_PROCESS_HEAD_H__
#define __PROXY_PROCESS_HEAD_H__

#include "IAcceptor.h"
#include "IProcess.h"
#include <stdio.h>

class CProxyProcess : public IProxyProcess
{
public:
    CProxyProcess();
	~CProxyProcess();
	
    int init(const string &strCfg, void* arg);
	int close(uint64_t flow, void* arg1, void* arg2);

	int input(uint64_t flow, void* arg1, void* arg2);
	int route(uint64_t flow, void* arg1, void* arg2);

	int exception(uint64_t flow, void* arg1, void* arg2);
	int fini(void* arg);
};


#endif
