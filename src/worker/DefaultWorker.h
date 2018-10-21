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
 * worker ģ��
 */
class CDefaultWorker : public Util::Shared
{
public:
	CDefaultWorker();

	virtual ~CDefaultWorker();

	/**
	 * worker���
	 */
    void start(int argc, char *argv[]);

	/**
	 * �������ݴ���
	 */
	static int recvDataActor(uint64_t flow, void* arg1, void *arg2);

private:
	/**
	 * ������
	 */
	void run();
	
	/**
	 * ���ó�ʼ��
	 */
	int loadConf(const string &strFileName);

	/**
	 * ����workerЭ��ģ��
	 */
	int loadSoWorker(const Module &stSoModule);

	/**
	 * ��ʼ�������ڴ����
	 */
	int initShmAcceptor(Worker &stWorker, ShareConf &stShareCfg);

public:
	/**
     * �Ƿ�ֹͣ��ʶ
     */
	bool m_bStop;
	
	/**
	 * ����������ʶ
	 */
	int m_iIndex;

    /**
     * ��ʱʱ��
     */
	int m_iTimeOut;

    /**
     * epoll��ʶ
     */
	int m_efd;
	
    /**
	 * so ��̬���ص�handle
	 */
    void *m_soHandle;

	/**
	 * worker��Ϣ����
	 */
    ShmAcceptor m_shmAcceptor;	

	/**
	 * workerЭ�������
	 */
	IWorkerProcess *m_soWorkerPtr;	
	
};

typedef Util::Handle<CDefaultWorker> CDefaultWorkerPtr;


#endif



