#include <stdio.h> 
#include <unistd.h>
#include "DefaultWorker.h"
#include "CLog.h"
#include "LoadCfg.h"
#include "Network.h"
#include "IProcess.h"
#include <fstream>
#include <dlfcn.h>
#include "Notify.h"
#include "TimeElapse.h"



CDefaultWorker::CDefaultWorker() : m_bStop(false), m_iTimeOut(0)
{
}

CDefaultWorker::~CDefaultWorker()
{
    if(m_efd > 0)
    {
        close(m_efd);
        m_efd = -1;
    }
    
    CLogHelper::destroy();
}

/**
 * worker入口
 */
void CDefaultWorker::start(int argc, char *argv[])
{
    if(argc < 3)
    {
        printf("usage:%s <configFile> <index>\r\n", argv[0]);
        exit(-1);
    }

    m_iIndex = atoi(argv[2]);
    if(loadConf(argv[1]) != 0)
    {
        printf("Load worker configure[%s] failed.\r\n", argv[1]);
        exit(-1);
    }

    run();
}


/**
 * 处理函数
 */
void CDefaultWorker::run()
{
    int iProcessCnt;
    int iWaitProcess;
    struct epoll_event stEvent;
    CTimeElapse timeElapse;
    
    while(!m_bStop)
    {
        timeElapse.reset();
        
        iProcessCnt = m_shmAcceptor.poll(false);

        LOG_DEBUG("worker poll cost:%d\r\n", timeElapse.costTime());

        if(iProcessCnt == 0)
        {
            iWaitProcess = epoll_wait(m_efd, &stEvent, 1, 1000);
            if(iWaitProcess > 0)
            {
                Util::Notify::notify_recv(stEvent.data.fd);
            }
        }

        
    }
}


/**
 * 配置初始化
 */
int CDefaultWorker::loadConf(const string &strFileName)
{
    WorkerConf workerCfg;
    if(!workerCfg.parseConf(strFileName))
    {
        printf("Worker loadconf failed.\r\n");
        return -E_FAIL;
    }

    // 日志初始化
    LogConf &stLogCfg = workerCfg.getLogConf();
    CLog* pLog = new CLog(stLogCfg.path.c_str(), stLogCfg.maxFileSize, stLogCfg.maxFileNum, CLog::_DATE_MODE);
    pLog->setLogLevel((CLog::LOG_LEVEL)stLogCfg.level);
    CLogHelper::addLogPtr("DefaultWorker", pLog, true);

    // worker so 初始化
    int iRet;
    Module &stSoModule = workerCfg.getModuleConf();
    if((iRet = loadSoWorker(stSoModule)) != 0)
    {
        LOG_ERROR("Load worker so failed.\r\n");
        return -1;
    }

    //共享内存队列初始化
    ShareConf stShareCfg = workerCfg.getShareConf();
    Worker stWorkerCfg = workerCfg.getWorkerConf();
    if((iRet = initShmAcceptor(stWorkerCfg, stShareCfg)) != 0)
    {
        LOG_ERROR("Initialize worker shm queue failed, ret:%d\r\n", iRet);
        return -2;
    }

    m_iTimeOut = stWorkerCfg.expireTime;
    m_shmAcceptor.regCallBack(CB_RECVDATA, CDefaultWorker::recvDataActor, this);

    m_efd = epoll_create(512);
    if(m_efd < 0)
    {
        LOG_ERROR("Create epoll failed, errMsg:%s.\r\n", strerror(errno));
        return -1;
    }

    int iFd = Util::Notify::notify_init(2 * stWorkerCfg.groupID);
    if(iFd < 0)
    {
        LOG_ERROR("Create worker notify failed.\r\n");
        return -2;
    }

    struct epoll_event stEvent;
    stEvent.events = EPOLLIN;
    stEvent.data.fd = iFd;

    if(epoll_ctl(m_efd, EPOLL_CTL_ADD, iFd, &stEvent))
    {
        LOG_ERROR("Add event failed.\r\n");
        return -3;
    }

    LOG_DEBUG("DefaultWorker loadConf success.\r\n");

    return 0;
}


/**
 * 加载worker协议模块
 */
int CDefaultWorker::loadSoWorker(const Module &stSoModule)
{
    m_soHandle = dlopen(stSoModule.bin.c_str(), RTLD_NOW);
    if(!m_soHandle)
    {
        LOG_ERROR("dlopen faild. ErrMsg:%s\r\n", dlerror());
        return -1;
    }

    createWorkerPorcess createFunc = (createWorkerPorcess)dlsym(m_soHandle, "createWorker");
    if(createFunc == NULL)
    {
        LOG_ERROR("dlsym createWorker failed. ErrMsg:%s\r\n", dlerror());
        return -2;
    }

    m_soWorkerPtr = createFunc();
    if(m_soWorkerPtr == NULL)
    {
        LOG_ERROR("create soProxyptr null.\r\n");
        return -3;
    }

    int iRet;
    if((iRet = m_soWorkerPtr->init(stSoModule.etc, this)) != 0)
    {
        LOG_ERROR("soWorker init failed. iRet:%d\r\n", iRet);
        return -4;
    }

    return 0;
}

/**
 * 初始化共享内存队列
 */
int CDefaultWorker::initShmAcceptor(Worker &stWorker, ShareConf &stShareCfg)
{
    shm_conf_t stShmConf;
        
    stShmConf.groupID = stWorker.groupID;
    stShmConf.msgTimeout = stWorker.expireTime;
    stShmConf.notifyFd = Util::Notify::notify_init(2 * stWorker.groupID + 1);
    stShmConf.shmKeyProducer = stShareCfg.shmBaseKey + 2 * stWorker.groupID + 1;
    stShmConf.shmSizeProducer = stWorker.sendSize;

    stShmConf.shmKeyConsumer = stShareCfg.shmBaseKey + 2 * stWorker.groupID;
    stShmConf.shmSizeConsumer = stWorker.recvSize;
    stShmConf.pMutexProducer = new Util::VSemMutex(stShareCfg.semBaseKey + 2 * stWorker.groupID);
    stShmConf.pMutexConsumer = new Util::VSemMutex(stShareCfg.semBaseKey + 2 * stWorker.groupID + 1);

    int iRet;
    if((iRet = m_shmAcceptor.init(&stShmConf)) != 0)
    {
        LOG_ERROR("Worker acceptor init failed. iRet:%d\r\n", iRet);
        return -1;
    }

    return 0;
}

/**
 * 接收数据处理
 */
int CDefaultWorker::recvDataActor(uint64_t flow, void* arg1, void *arg2)
{
    transmit_data *pData = (transmit_data*)arg1;
    CDefaultWorker *pProxy = (CDefaultWorker*)arg2;

    if(likely(pData->len > 0))
    {
        connExtInfo *pExtInfo = NULL;
        ShmAcceptor *pShmAccepter = (ShmAcceptor*)pData->owner;
        
        pData->len -= sizeof(connExtInfo);
        pData->extdata = pData->data + pData->len;
        
        pExtInfo = (connExtInfo*)pData->extdata;

        LOG_DEBUG("Receive Data:%d extData:%d\r\n", pData->len, sizeof(connExtInfo));

        

        int64_t now = getTime();
        int64_t delay = now - pExtInfo->recvTime;

        if(pShmAccepter->m_iMsgTimeout > 0 && delay > pShmAccepter->m_iMsgTimeout)
        {
            if(pExtInfo->type == SOCKET_UDP)
            {
                transmit_data stData;
                stData.data = NULL;
                stData.len = 0;
                stData.extdata = NULL;

                pShmAccepter->sendto(flow, &stData, NULL);
            }

            return 0;
        }

        int iRet = pProxy->m_soWorkerPtr->process(flow, arg1, arg2);
        if(unlikely(iRet != 0))
        {
            LOG_ERROR("Process failed. close the connection.\r\n");
            
            transmit_data stData;
            stData.data = NULL;
            stData.len = 0;
            stData.extdata = NULL;

            pShmAccepter->sendto(flow, &stData, NULL);

            return -1;
        }

        return 0;
    }

    return -1;
}

