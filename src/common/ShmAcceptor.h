#ifndef __SHM_CONNECTOR_HEAD_H_
#define __SHM_CONNECTOR_HEAD_H_

#include "Buffer.h"
#include "IAcceptor.h"
#include "Common.h"
#include "VSemMutex.h"
#include <errno.h>
#include <string.h>
#include <sys/file.h>

#define LOCK_TYPE_NONE		0x0		//不锁
#define LOCK_TYPE_PRODUCER	0x1		//写锁
#define LOCK_TYPE_CONSUMER	0x2		//读锁


typedef struct st_shm_conf shm_conf_t;

struct st_shm_conf
{
    int shmKeyProducer;
	int shmSizeProducer;
	int shmKeyConsumer;
	int shmSizeConsumer;
	int msgTimeout;
	int groupID;
	int notifyFd;
	
	Util::VSemMutex* pMutexProducer;
	Util::VSemMutex* pMutexConsumer;
}; 

/**
 * 生产者(写数据入MQ)
 */
class ShmProducer
{
public:
    ShmProducer();
	virtual ~ShmProducer();

	int init(int shmKey, int shmSize);
	int clear();

	void setNotify(int fd){ m_iNotifyFd = fd; }
	void setMutex(Util::VSemMutex* pMutex){ m_pMutex = pMutex;}

	int produce(uint64_t flow, struct ioMsghdr &msghdr);
	int produce(uint64_t flow, const char* data, int len);
protected:
	int     m_iNotifyFd;
	ShmMQ*  m_pShmMQ;
	
	Util::VSemMutex* m_pMutex;
};

/**
 * 消费者(读MQ数据)
 */
class ShmConsumer
{
public:
    ShmConsumer();
	virtual ~ShmConsumer();

	int init(int shmKey, int shmSize);
    int clear();

	void setMutex(Util::VSemMutex* pMutex){ m_pMutex = pMutex;}
	int consume(uint64_t& flow, char *data, int &len);

protected:
	ShmMQ  *m_pShmMQ;
	Util::VSemMutex* m_pMutex;
};

/**
 * 共享内存队列数据接入
 */
class ShmAcceptor : public IAcceptor
{
public:
	ShmAcceptor();
	virtual ~ShmAcceptor();
	
	int init(void *config);
	int poll(bool block);

	int sendto(uint64_t flow, void *arg1, void* arg2);

public:
	int m_iMsgTimeout;
	ShmProducer *m_pProducer;
	ShmConsumer *m_pConsumer;
	transmit_data m_tranData;
};

#endif


