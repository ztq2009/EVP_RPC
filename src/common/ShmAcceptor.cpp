#include "ShmAcceptor.h"
#include <unistd.h>
#include <string.h>

#define    MAX_MSG_SIZE   10*1024*1024  // 10M

ShmProducer::ShmProducer()
{
    m_iNotifyFd = -1;
    m_pShmMQ = NULL;
    m_pMutex = NULL;
}

ShmProducer::~ShmProducer()
{
    if(m_pShmMQ)
    {
        delete m_pShmMQ;
        m_pShmMQ = NULL;
    }
}

int ShmProducer::init(int shmKey, int shmSize)
{
    m_pShmMQ = new ShmMQ();

    return m_pShmMQ->init(shmKey, shmSize);
}

int ShmProducer::clear()
{
    try
    {
        Util::VSemMutex::PLock  _fileLock(m_pMutex);

        return m_pShmMQ->clear(E_TYPE_PRODUCE);
    }
    catch(CException &e)
    {
        return -E_SHM_LOCK_FAIL;
    }
}

int ShmProducer::produce(uint64_t flow, struct ioMsghdr &msghdr)
{
    try
    {
        int ret;
        Util::VSemMutex::PLock _fileLock(m_pMutex);
        
        ret = m_pShmMQ->enqueue(flow, msghdr);
        if(unlikely(ret == -E_SHMMQ_CEHCK_FAIL))
        {
            m_pShmMQ->clear(E_TYPE_PRODUCE);
            
            return ret;
        }

        if(ret > 0 && (ret == 1 || ret % 50 == 0))
        {
            if(write(m_iNotifyFd, "W", 1) < 0)
            {
                LOG_DEBUG("Produce Notify Failed.");
            }
        }

        return ret;
    }
    catch(CException &e)
    {
        return -E_SHM_LOCK_FAIL;
    }
}

int ShmProducer::produce(uint64_t flow, const char* data, int len)
{
    try
    {
        int ret;
        Util::VSemMutex::PLock _fileLock(m_pMutex);
        
        ret = m_pShmMQ->enqueue(flow, data, len);
        if(unlikely(ret == -E_SHMMQ_CEHCK_FAIL))
        {
            m_pShmMQ->clear(E_TYPE_PRODUCE);
            
            return ret;
        }

        if(ret > 0 && (ret == 1 || ret % 50 == 0))
        {
            if(unlikely(write(m_iNotifyFd, "W", 1) < 0))
            {
                LOG_ERROR("Produce Notify Failed.");
            }
        }

        return ret;
    }
    catch(CException &e)
    {
        return -E_SHM_LOCK_FAIL;
    }
}


ShmConsumer::ShmConsumer()
{
    m_pShmMQ = NULL;
}

ShmConsumer::~ShmConsumer()
{
    if(m_pShmMQ)
    {
        delete m_pShmMQ;
        m_pShmMQ = NULL;
    }
}

int ShmConsumer::init(int shmKey, int shmSize)
{
    m_pShmMQ = new ShmMQ();
    
    return m_pShmMQ->init(shmKey, shmSize);
  
}

int ShmConsumer::clear()
{
    try
    {
        Util::VSemMutex::PLock  _fileLock(m_pMutex);

        return m_pShmMQ->clear(E_TYPE_CONSUME);
    }
    catch(CException &e)
    {
        return -E_SHM_LOCK_FAIL;
    }
}

int ShmConsumer::consume(uint64_t& flow, char *data, int &len)
{
    try
    {
        int ret;
        Util::VSemMutex::PLock  _fileLock(m_pMutex);

        ret = m_pShmMQ->dequeue(flow, data, len);
        if(unlikely(ret == -E_SHMMQ_CEHCK_FAIL))
        {
            m_pShmMQ->clear(E_TYPE_CONSUME);
        }

        return ret;
    }
    catch(CException &e)
    {
        return -E_SHM_LOCK_FAIL;
    }
}


ShmAcceptor::ShmAcceptor()
{
    m_pProducer = NULL;
    m_pConsumer = NULL;

    m_iMsgTimeout = -1;

    memset(&m_tranData, 0, sizeof(m_tranData));
}

ShmAcceptor::~ShmAcceptor()
{
    if(m_pProducer)
    {
        delete m_pProducer;
        m_pProducer = NULL;
    }

    if(m_pConsumer)
    {
        delete m_pConsumer;
        m_pConsumer = NULL;
    }

    if(m_tranData.data)
    {
        free(m_tranData.data);
        m_tranData.data = NULL;
    }
}


int ShmAcceptor::init(void *config)
{
    shm_conf_t* conf = (shm_conf_t*)config;

    // 生产者
    m_pProducer = new ShmProducer();
    if(m_pProducer->init(conf->shmKeyProducer, conf->shmSizeProducer))
    {
        return -E_FAIL;
    }

    m_pProducer->setNotify(conf->notifyFd);
    m_pProducer->setMutex(conf->pMutexProducer);

    // 消费者
    m_pConsumer = new ShmConsumer();
    if(m_pConsumer->init(conf->shmKeyConsumer, conf->shmSizeConsumer))
    {
        return -E_FAIL;
    }

    m_pConsumer->setMutex(conf->pMutexConsumer);

    m_tranData.len = MAX_MSG_SIZE;
    m_tranData.data = (char *)malloc(MAX_MSG_SIZE);
    if(m_tranData.data == NULL)
    {
        return -E_FAIL;
    }

    m_tranData.owner = this;
    m_tranData.extdata = NULL;
    m_iMsgTimeout = conf->msgTimeout;
    
    return E_OK;

}

int ShmAcceptor::poll(bool block)
{
    int ret;
    int processCnt = 0;
    uint64_t flow;
    
    do
    {
        m_tranData.len = MAX_MSG_SIZE;
        ret = m_pConsumer->consume(flow, m_tranData.data, m_tranData.len);
        if(ret != 0)
        {
            break;
        }

        ++processCnt;

        m_funcs[CB_RECVDATA](flow, &m_tranData, m_args[CB_RECVDATA]);
        
    }while(block);

    return processCnt;
}

int ShmAcceptor::sendto(uint64_t flow, void *arg1, void* arg2)
{
    transmit_data* pData = (transmit_data*)arg1;

    struct ioBuffer iobuffer[2];
    struct ioMsghdr msgHead;

    msgHead.msg_iov = iobuffer;
    msgHead.msg_iovlen = 1;

    iobuffer[0].io_base = pData->data;
    iobuffer[0].io_len = pData->len;

    if(pData->extdata)
    {
        iobuffer[1].io_base = pData->extdata;
        iobuffer[1].io_len = sizeof(struct connExtInfo);

        msgHead.msg_iovlen = 2;
    }

    int ret;

    ret = m_pProducer->produce(flow, msgHead);
    if(likely(ret > 0))
    {
        if(m_funcs[CB_SENDDATA])
        {
            m_funcs[CB_SENDDATA](flow, arg1, m_args[CB_SENDDATA]);
        }
        
        ret = 0;
    }
    else
    {
        if(m_funcs[CB_SENDERROR])
        {
            m_funcs[CB_SENDERROR](flow, arg1, m_args[CB_SENDERROR]);
        }
    }

    return ret;
}


