#ifndef __INTERFACE_PROCESS_HEAD_H_
#define __INTERFACE_PROCESS_HEAD_H_

#include <string>
#include <stdint.h>

using std::string;

/**
 * so定义proxy处理类基
 */
class IProxyProcess
{
public:
	IProxyProcess(){}
	virtual ~IProxyProcess(){}

	virtual int init(const string &strCfg, void* arg) = 0;
	virtual int close(uint64_t flow, void* arg1, void* arg2) = 0;

	virtual int input(uint64_t flow, void* arg1, void* arg2) = 0;
	virtual int route(uint64_t flow, void* arg1, void* arg2) = 0;

	virtual int exception(uint64_t flow, void* arg1, void* arg2) = 0;
	virtual int fini(void* arg) = 0;
};

/**
 * so定义worker处理类基
 */
class IWorkerProcess
{
public:
	IWorkerProcess(){}
	virtual ~IWorkerProcess(){}

	virtual int init(const string &strCfg, void* arg) = 0;
	virtual int fini(void* arg) = 0;

	virtual int process(uint64_t flow, void* arg1, void* arg2) = 0;
};


typedef IProxyProcess*  (*createProxyProcess)();
typedef IWorkerProcess* (*createWorkerPorcess)();

#endif


