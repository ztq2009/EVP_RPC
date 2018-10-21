#include <stdio.h> 
#include <unistd.h>
#include "DefaultProxy.h"
#include "CLog.h"
#include "LoadCfg.h"
#include "Network.h"
#include "IProcess.h"
#include <fstream>
#include <dlfcn.h>
#include "Notify.h"
#include "TimeElapse.h"



CDefaultProxy::CDefaultProxy()
{
}

CDefaultProxy::~CDefaultProxy()
{
    if(m_soHandle != NULL)
    {
        dlclose(m_soHandle);
        m_soHandle = NULL;
    }
}

void CDefaultProxy::start(int argc, char *argv[])
{
    if(argc < 2)
    {
        printf("usage:%s <configFile>\r\n", argv[0]);
        exit(-1);
    }
    
    if(loadConf(argv[1]) != 0)
    {
        printf("Load proxy configure[%s] failed.\r\n", argv[1]);
        exit(-1);
    }

    run();
}

void CDefaultProxy::run()
{
    int iProcessCnt = 0;
    CTimeElapse timeElapse;

    while(!m_bStop)
    {
        m_sockAcceptor.poll(iProcessCnt == 0);
        
        iProcessCnt = 0;
        timeElapse.reset();
        
        for(map<int, ShmAcceptor*>::iterator it = m_mapShmAcceptor.begin(); it != m_mapShmAcceptor.end(); ++it)
        {
            iProcessCnt += it->second->poll(true);
        }

        LOG_DEBUG("Shm poll cost time:%d", timeElapse.costTime());
    }
}

/**
 * 解析配置
 */
int CDefaultProxy::loadConf(const std::string &strFilePath)
{
    ProxyConf proxyCfg;
    if(!proxyCfg.parseConf(strFilePath))
    {
        printf("parse proxy file failed.\r\n");
        return -1;
    }


    AcceptorSock &acceptCfg = proxyCfg.getAcceptorConf();

    m_stSrvConf.expireTime = acceptCfg.timeout;
    m_stSrvConf.maxconn = acceptCfg.maxconn;
    m_stSrvConf.udpAutoClose = acceptCfg.udpclose;
    m_stSrvConf.maxCacheSize = acceptCfg.maxCacheSize;

    for(size_t i = 0; i < acceptCfg.entity.size(); ++i)
    {
        listen_sock_t stSockCfg;
        
        AcceptorEntity &stEntity = acceptCfg.entity[i];

        if(strncasecmp(stEntity.type.c_str(), "tcp", 3) == 0)
        {
            stSockCfg.type = SOCKET_TCP;
        }
        else if(strncasecmp(stEntity.type.c_str(), "udp", 3) == 0)
        {
            stSockCfg.type = SOCKET_UDP;
        }
        else if(strncasecmp(stEntity.type.c_str(), "unix", 4) == 0)
        {
            stSockCfg.type = SOCKET_UNIX;
        }
        else 
        {
            printf("UnKnow socket type:%s\r\n", stEntity.type.c_str());
            continue;
        }

        stSockCfg.tos = stEntity.tos;
        stSockCfg.oob = stEntity.oob;

        if(stSockCfg.type != SOCKET_UNIX)
        {
            struct sockaddr_in  inAddr;
            if(NetWork::getIfAddress(stEntity.ip, stEntity.port, inAddr) != 0)
            {
                printf("Get no interface address:%s:%d\r\n", stEntity.ip.c_str(), stEntity.port);
                continue;
            }

            stSockCfg.addr.ip = inAddr.sin_addr.s_addr;
            stSockCfg.addr.port = inAddr.sin_port;
        }
        else
        {
            strncpy(stSockCfg.path, stEntity.ip.c_str(), sizeof(stSockCfg.path) - 1);
        }

        m_stSrvConf.entity.push_back(stSockCfg);
    }

    // 系统日志
    LogConf stLogCfg = proxyCfg.getLogConf();
    CLog* pLog = new CLog(stLogCfg.path.c_str(), stLogCfg.maxFileSize, stLogCfg.maxFileNum, CLog::_DATE_MODE);
    pLog->setLogLevel((CLog::LOG_LEVEL)stLogCfg.level);
    CLogHelper::addLogPtr("DefaultProxy", pLog, true);

    // 接入黑名单和白名单
    Iptables stIPTable = proxyCfg.getIptables();
    m_iIPAccessMod = stIPTable.whiteList.empty()? E_IP_ACCESS_BLACKLIST : E_IP_ACCESS_WHITELIST;
    loadIPList((m_iIPAccessMod == E_IP_ACCESS_WHITELIST)? stIPTable.whiteList : stIPTable.blackList);
    if(!m_setIP.empty())
    {
        m_sockAcceptor.regCallBack(CB_CONNECTED, CDefaultProxy::connectedActor, this);
    }
    
    // 加载proxy so处理方法
    int iRet;
    Module &stSoModule = proxyCfg.getModuleConf();
    if((iRet = loadSoProxy(stSoModule)) != 0)
    {
        LOG_ERROR("Loadsoproxy failed. iRet:%d", iRet);
        return -2;
    }

    //监听初始话
    if((iRet = m_sockAcceptor.init(&m_stSrvConf)) != 0)
    {
        LOG_ERROR("sockAcceptor init failed. ret:%d", iRet);
        return -3;
    }

    m_sockAcceptor.regCallBack(CB_RECVDATA, CDefaultProxy::recvDataActor, this);
    m_sockAcceptor.regCallBack(CB_OVERLOAD, CDefaultProxy::overLoadActor, this);
    
    // worker消费处理队列
    ConnectorShm &stConnShm = proxyCfg.getConnectorConf();
    ShareConf &stShareCfg = proxyCfg.getShareConf();
    if((iRet = initShmAcceptor(stConnShm, stShareCfg)) != 0)
    {
        LOG_ERROR("initShmAcceptor failed. ret:%d", iRet);
        return -4;
    }
    
    return 0;
}

/**
 * 加载IP列表
 */
int CDefaultProxy::loadIPList(const std::string &strFileName)
{
    std::ifstream  fin(strFileName.c_str());

    if(!fin)
    {
        LOG_DEBUG("LoadIPList failed, file '%s' not exist please check it again.", strFileName.c_str());
        return -1;
    }

    m_setIP.clear();
    
    char buff[1024];
    unsigned int uIP;
	while(fin.getline(buff,1024))
	{ 
	    uIP = NetWork::stringToIpAddr(buff);
	    m_setIP.insert(uIP);
	}

    fin.close();

    return 0;
}

/**
 * 加载proxy端so
 */
int CDefaultProxy::loadSoProxy(Module &stModule)
{
    m_soHandle = dlopen(stModule.bin.c_str(), RTLD_NOW);
    if(!m_soHandle)
    {
        LOG_ERROR("dlopen faild. ErrMsg:%s", dlerror());
        return -1;
    }
    
    createProxyProcess createFunc = (createProxyProcess)dlsym(m_soHandle, "createProxy");
    if(createFunc == NULL)
    {
        LOG_ERROR("dlsym createProxy failed. ErrMsg:%s", dlerror());
        return -2;
    }

    m_soProxyPtr = createFunc();
    if(m_soProxyPtr == NULL)
    {
        LOG_ERROR("create soProxyptr null.");
        return -3;
    }

    int iRet;
    if((iRet = m_soProxyPtr->init(stModule.etc, this)) != 0)
    {
        LOG_ERROR("soProxy init failed, ret:%d", iRet);
        return -4;
    }

    return 0;
}


/**
 * proxy共享内存初始化
 */
int CDefaultProxy::initShmAcceptor(ConnectorShm &stShm, ShareConf &stShareCfg)
{
    int iRet;
    for(size_t i = 0; i < stShm.entity.size(); ++i)
    {
        ConnectorEntity &stEntity = stShm.entity[i];
        ShmAcceptor *pShmAcept = new ShmAcceptor();

        shm_conf_t stShmConf;
        
        stShmConf.groupID = stEntity.groupid;
        stShmConf.msgTimeout = stEntity.expireTimeout;
        stShmConf.notifyFd = Util::Notify::notify_init(2 * stEntity.groupid);
        stShmConf.shmKeyProducer = stShareCfg.shmBaseKey + 2 * stEntity.groupid;
        stShmConf.shmSizeProducer = stEntity.sendSize;

        stShmConf.shmKeyConsumer = stShareCfg.shmBaseKey + 2 * stEntity.groupid + 1;
        stShmConf.shmSizeConsumer = stEntity.recvSize;
        stShmConf.pMutexProducer = NULL;
        stShmConf.pMutexConsumer = NULL;
        
        if((iRet = pShmAcept->init(&stShmConf)) != 0)
        {
            LOG_ERROR("Init ShmAcceptor failed, ret:%d", iRet);
            return -1;
        }

        if(m_sockAcceptor.addNotify(stEntity.groupid) != 0)
        {
            LOG_ERROR("AddNotify failed.");
            return -2;
        }

        pShmAcept->regCallBack(CB_RECVDATA, CDefaultProxy::recvDataActor2, this);

        m_mapShmAcceptor[stEntity.groupid] = pShmAcept;
    }

    LOG_DEBUG("Initialize group size[%d]", m_mapShmAcceptor.size());

    return 0;
}

/**
 * 客户端连接处理
 */
int CDefaultProxy::connectedActor(uint64_t flow, void* arg1, void *arg2)
{
    transmit_data *pData = (transmit_data*)arg1;
    CDefaultProxy *pProxy = (CDefaultProxy*)arg2;
    int iAllowed = 1;

    if(pProxy->m_iIPAccessMod == E_IP_ACCESS_BLACKLIST)
    {
        if(pProxy->m_setIP.count(((connExtInfo*)pData->extdata)->remoteIp) == 0)
        {
            iAllowed = 0;
        }
    }
    else if(pProxy->m_setIP.count(((connExtInfo*)pData->extdata)->remoteIp) > 0)
    {
        iAllowed = 0;
    }

    return iAllowed;
}


/**
 * 接收数据处理
 */
int CDefaultProxy::recvDataActor(uint64_t flow, void* arg1, void *arg2)
{
    transmit_data *pData = (transmit_data*)arg1;
    CDefaultProxy *pProxy = (CDefaultProxy*)arg2;

    int iRet;
    int iProcessLen = 0, iProtoLen = 0;
    
    while(pData->len > 0 && (iProtoLen = pProxy->m_soProxyPtr->input(flow, arg1, arg2)) > 0)
    {
        if(iProtoLen > pData->len)
        {
            return -1;
        }

        iProcessLen += iProtoLen;
        unsigned int uRouteNo = pProxy->m_soProxyPtr->route(flow, arg1, arg2);

        map<int, ShmAcceptor*>::iterator it =  pProxy->m_mapShmAcceptor.find((int)uRouteNo);
        if(likely(it != pProxy->m_mapShmAcceptor.end()))
        {
            transmit_data stData;
            stData.data = pData->data;
            stData.len = iProtoLen;
            stData.extdata = pData->extdata;
            stData.owner = pData->owner;

            iRet = it->second->sendto(flow, &stData, arg2);
            if(iRet < 0)
            {
                LOG_WARN("flow[%u] process message failed. iRet:%d", flow, iRet);
            }
        }
        else
        {
            LOG_ERROR("Without group route[%u] flow[%lu] iProtoLen[%d]", uRouteNo, flow, iProtoLen);
        }

        pData->data += iProtoLen;
        pData->len -= iProtoLen;
    }

    if(iProtoLen < 0)
    {
        return -2;
    }

    return iProcessLen;
}

/**
 * shmAcceptor接收数据处理
 */
int CDefaultProxy::recvDataActor2(uint64_t flow, void* arg1, void *arg2)
{
    CDefaultProxy *pProxy = (CDefaultProxy*)arg2;
    int iSendLen = pProxy->m_sockAcceptor.sendto(flow, arg1, NULL);
    if(iSendLen < 0)
    {
        LOG_WARN("Send Message Faild, flow[%lu] iret:%d", flow, iSendLen);   
    }

    return 0;
}


/**
 * 连接过载处理
 */
int CDefaultProxy::overLoadActor(uint64_t flow, void* arg1, void *arg2)
{
    return 0;
}


