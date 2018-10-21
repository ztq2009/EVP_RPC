#ifndef __DEFAULT_WORKER_HEAD_H__
#define __DEFAULT_WORKER_HEAD_H__

#include "Handle.h"
#include "Shared.h"
#include "Loadcfg.h"
#include "SocketAcceptor.h"
#include "ShmAcceptor.h"
#include "IProcess.h"
#include <set>

using namespace std;

/**
 * worker 模块
 */
class CDefaultWorker : public Util::Shared
{
public:
	CDefaultWorker();

	virtual ~CDefaultWorker();

	/**
	 * worker入口
	 */
    void start(int argc, char *argv[]);

	/**
	 * 接收数据处理
	 */
	static int recvDataActor(uint64_t flow, void* arg1, void *arg2);

private:
	/**
	 * 处理函数
	 */
	void run();
	
	/**
	 * 配置初始化
	 */
	int loadConf(const string &strFileName);

	/**
	 * 加载worker协议模块
	 */
	int loadSoWorker(const Module &stSoModule);

	/**
	 * 初始化共享内存队列
	 */
	int initShmAcceptor(Worker &stWorker, ShareConf &stShareCfg);

public:
	/**
     * 是否停止标识
     */
	bool m_bStop;
	
	/**
	 * 进程索引标识
	 */
	int m_iIndex;

    /**
     * 超时时间
     */
	int m_iTimeOut;

    /**
     * epoll标识
     */
	int m_efd;
	
    /**
	 * so 动态加载的handle
	 */
    void *m_soHandle;

	/**
	 * worker消息队列
	 */
    ShmAcceptor m_shmAcceptor;	

	/**
	 * worker协议解析类
	 */
	IWorkerProcess *m_soWorkerPtr;	
	
};

typedef Util::Handle<CDefaultWorker> CDefaultWorkerPtr;


#endif



