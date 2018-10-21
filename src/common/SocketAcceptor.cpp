#include "SocketAcceptor.h"
#include "Network.h"
#include "Exception.h"
#include "ErrorCode.h"
#include "Notify.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>

SocketAcceptor::SocketAcceptor() : m_iMaxConn(0), m_iSendLimit(0), m_iExpireTime(0),m_bUdpAutoClose(false), 
                                         m_lastCheckTime(0), m_pSockMgr(NULL), m_pConnMgr(NULL), m_pEvent(NULL)
{
    
}

SocketAcceptor::~SocketAcceptor()
{
    if(m_pSockMgr)
    {
        delete m_pSockMgr;
    }

    if(m_pConnMgr)
    {
        delete m_pConnMgr;
    }

    if(m_pEvent)
    {
        free(m_pEvent);
    }
}

/**
 * 模块初始化，加载配置信息
 */
int SocketAcceptor::init(void *config)
{
    sock_conf_t *pConf = (sock_conf_t*)config;

    if(pConf->maxconn <= 0 || pConf->entity.empty())
    {
        return -E_FAIL;
    }

    m_iMaxConn = pConf->maxconn;
    m_bUdpAutoClose = pConf->udpAutoClose;
    m_iExpireTime = pConf->expireTime;
    m_iSendLimit = pConf->maxCacheSize;

    int fd;
    for(size_t i = 0; i < pConf->entity.size(); ++i)
    {
        if((fd = createSocke(pConf->entity[i])) < 0)
        {
            LOG_ERROR("Create socket failed.");
            return -E_FAIL;
        }

        if(pConf->entity[i].tos >= 0)
        {
            setsockopt(fd, IPPROTO_IP, IP_TOS, &(pConf->entity[i].tos), sizeof(int));
        } 

        if(pConf->entity[i].type == SOCKET_TCP && pConf->entity[i].oob > 0)
        {
            int on = 1;
            setsockopt(fd, SOL_SOCKET, SO_OOBINLINE, &on, sizeof(on));
        }

        m_listen_map[fd] = pConf->entity[i];
    }

    m_pSockMgr = new SocketMgr();

    m_pConnMgr = new ConnManager(m_iMaxConn);

    m_pEvent = (struct epoll_event*)malloc(sizeof(struct epoll_event) * m_iMaxConn);
    if(m_pEvent == NULL)
    {
        LOG_ERROR("Malloc epoll_event failed.");
        return -E_FAIL;
    }

    uint64_t flow;
    ConnInfo* pConn;
    map<int, listen_sock_t>::iterator it;
    for(it = m_listen_map.begin(); it != m_listen_map.end(); ++it)
    {
        if(m_pConnMgr->addConn(flow, it->first, it->second.type, true) != 0)
        {
            LOG_ERROR("Add connection failed. ConnInfo:%s", it->second.toString().c_str());
            return -E_FAIL;
        }
        
        if(!m_pSockMgr->add(it->first, EV_READ, flow))
        {
            LOG_ERROR("Add connection failed. ConnInfo:%s", it->second.toString().c_str());
            m_pConnMgr->closeConn(flow);
            
            return -E_FAIL;
        }

        // 监听接口(TCP,UDP)，ip、port写入extInfo
        if(it->second.type != SOCKET_UNIX)
        {
            pConn = m_pConnMgr->getConnection(flow);
            
            pConn->extInfo.localIp = it->second.addr.ip;
            pConn->extInfo.localPort = it->second.addr.port;
        }
    }

    m_tranData.data = NULL;
    m_tranData.len = 0;
    m_tranData.extdata = NULL;
    m_tranData.owner = this;

    return E_OK;
}

/**
 * 数据监听
 */
int SocketAcceptor::poll(bool block)
{
    uint64_t flow;
    ConnInfo* pConn;
    map<int, listen_sock_t>::iterator it;
    int iActiveCount = m_pSockMgr->wait(m_pEvent, m_iMaxConn, block? 1000 : 0);

    for(int i = 0; i < iActiveCount; ++i)
    {
        flow = m_pEvent[i].data.u64;
        pConn = m_pConnMgr->getConnection(flow);
     
        if(unlikely(pConn == NULL))
        {
            continue;
        }
        
        if((it = m_listen_map.find(pConn->fd)) != m_listen_map.end())
        {
            if(m_pEvent[i].events & EPOLLIN)
            {
                if(it->second.type == SOCKET_UDP)
                {
                    handleUdpAccept(pConn);
                }
                else if(it->second.type == SOCKET_NOTIFY)
                {
                    Util::Notify::notify_recv(pConn->fd);
                }
                else if(it->second.type == SOCKET_TCP || it->second.type == SOCKET_UNIX)
                {
                    handleAccept(pConn);
                }
            }

            continue;
        }

        if(!(m_pEvent[i].events & (EPOLLIN | EPOLLOUT)))
        {   
            m_pSockMgr->del(pConn->fd);
            m_pConnMgr->closeConn(flow);
            
            continue;
        }

        if(pConn->seq == 0)
        {
            continue;
        }

        if(m_pEvent[i].events & EPOLLOUT)
        {
            int iSendLen;
            
            iSendLen = m_pConnMgr->sendWriteBuffer(flow);
            if(iSendLen == 0 && pConn->writeBuffer.getFinBit())
            {
                m_pSockMgr->del(pConn->fd);
                m_pConnMgr->closeConn(flow);

                continue;
            }

            if(iSendLen == 0)
            {
                m_pSockMgr->mod(pConn->fd, EV_READ, flow);
            }
            else if(iSendLen == -E_NEED_CLOSE)
            {
                if(m_funcs[CB_SENDERROR] != NULL)
                {
                    m_tranData.data = pConn->writeBuffer.data();
                    m_tranData.len = pConn->writeBuffer.length();
                    m_tranData.extdata = &(pConn->extInfo);

                    m_funcs[CB_SENDERROR](flow, &m_tranData, m_args[CB_SENDERROR]);
                }
                                    
                m_pSockMgr->del(pConn->fd);
                m_pConnMgr->closeConn(flow);

                continue;
            }
        }
        
        if(m_pEvent[i].events & EPOLLIN)
        {
            int iRet, iProcessLen;
            
            iRet = m_pConnMgr->recv(flow, &(m_tranData.data), m_tranData.len);
            if(likely(iRet == E_OK))
            {
                m_tranData.extdata = &(pConn->extInfo);

                iProcessLen = m_funcs[CB_RECVDATA](flow, &m_tranData, m_args[CB_RECVDATA]);
                
                if(iProcessLen >= 0)
                {
                    // 有没处理完数据，保存到readbuffer.
                    if(unlikely(iProcessLen < m_tranData.len))
                    {
                        try
                        {
                            pConn->readBuffer.append(m_tranData.data + iProcessLen, m_tranData.len - iProcessLen);
                        }
                        catch(...)
                        {
                            LOG_DEBUG("readBuffer exhaust memory.");
                            
                            m_pSockMgr->del(pConn->fd);
                            m_pConnMgr->closeConn(flow);
                            
                        }
                    }

                    if(iProcessLen > 0 && m_funcs[CB_RECVDONE] != NULL)
                    {
                        m_tranData.len = iProcessLen;
                        
                        m_funcs[CB_RECVDONE](flow, &m_tranData, m_args[CB_RECVDONE]);
                    }
                }
                else
                {
                    LOG_DEBUG("Flow[%lu] process return[%d] close connection.", flow, iProcessLen);
                    
                    m_pSockMgr->del(pConn->fd);
                    m_pConnMgr->closeConn(flow);
                }
            }
            else if(iRet == -E_NEED_CLOSE)
            {
                if(m_funcs[CB_DISCONNECT] != NULL)
                {
                    m_tranData.data = NULL;
                    m_tranData.len = 0;
                    m_tranData.extdata = &(pConn->extInfo);

                    m_funcs[CB_DISCONNECT](flow, &m_tranData, m_args[CB_DISCONNECT]);
                }

                LOG_DEBUG("client close connection.");
                
                m_pSockMgr->del(pConn->fd);
                m_pConnMgr->closeConn(flow);
            }
        }
    }

    checkExpire();

    return E_OK;
}

/**
 * 回包发送，返回发送字节数
 */
int SocketAcceptor::sendto(uint64_t flow, void *arg1, void* arg2)
{
    int iSendLen;
    transmit_data *pData = (transmit_data*)arg1;
    ConnInfo* pConn = m_pConnMgr->getConnection(flow);

    if(unlikely(pConn == NULL))
    {
        return -E_NOT_FIND;
    }

    if(unlikely(pData->len == 0 && pConn->seq != 0))
    {
        int iWriteLen = pConn->writeBuffer.length();
        if(iWriteLen > 0)
        {
            pConn->writeBuffer.setFinBit(true);
            
            return 0;
        }

        if(pConn->type != SOCKET_UDP)
        {
            m_pSockMgr->del(pConn->fd);
        }

        m_pConnMgr->closeConn(flow);
        
        return 0;
    }

    // 最大发送长度限制, 关闭连接
    if(unlikely(m_iSendLimit > 0 && pConn->writeBuffer.length() > m_iSendLimit))
    {
        m_pSockMgr->del(pConn->fd);
        m_pConnMgr->closeConn(flow);
        return 0;
    }

    iSendLen = m_pConnMgr->send(flow, pData->data, pData->len);
    if(likely(iSendLen >= 0))
    {
        if(iSendLen < pData->len)
        {
            if(pConn->type != SOCKET_UDP)
            {
                m_pSockMgr->mod(pConn->fd, EV_READ | EV_WRITE, flow);
                
                return iSendLen;
            }
            else  // UDP 一次写完, 否则出错
            {
                m_pConnMgr->closeConn(flow);
                
                return -E_FAIL;
            }
        }

        if(m_funcs[CB_SENDDONE] != NULL)
        {
            m_tranData.data = pData->data;
            m_tranData.len = pData->len;
            m_tranData.extdata = &(pConn->extInfo);

            m_funcs[CB_SENDDONE](flow, &m_tranData, m_args[CB_SENDDONE]);
        }

        if(pConn->type == SOCKET_UDP && m_bUdpAutoClose)
        {
            m_pConnMgr->closeConn(flow);
        }

        return iSendLen;
    }
    else if(iSendLen == -E_NEED_CLOSE)
    {
        if(pConn->type != SOCKET_UDP)
        {
            m_pSockMgr->del(pConn->fd);
        }

        m_pConnMgr->closeConn(flow);
    }

    return -E_FAIL;
}

/**
 * 加入通知FD
 */
int SocketAcceptor::addNotify(int groupID)
{
    uint64_t flow;
    int notifyFd  = Util::Notify::notify_init(2 * groupID + 1);

    if(m_pConnMgr->addConn(flow, notifyFd, SOCKET_NOTIFY, true) != 0)
    {
        return -E_FAIL;
    }

    if(!m_pSockMgr->add(notifyFd, EV_READ, flow))
    {
        m_pConnMgr->closeConn(flow);

        return -E_FAIL;
    }

    m_listen_map[notifyFd].type = SOCKET_NOTIFY;
    
    return E_OK;
}

/**
 * 根据协议类型创建socket
 */
int SocketAcceptor::createSocke(const listen_sock_t &sock)
{
    int fd = -1;
    int iRet;
    
    switch(sock.type)
    {
        case SOCKET_TCP:
            fd = socket(AF_INET, SOCK_STREAM, 0);
            break;
               
        case SOCKET_UDP:
            fd = socket(AF_INET, SOCK_DGRAM, 0);
            break;

        case SOCKET_UNIX:
            fd = socket(AF_UNIX, SOCK_STREAM, 0);
            break;

        default:
            break;
    }

    if(fd < 0)
    { 
        return -E_FAIL;
    }

    iRet = NetWork::setBlock(fd, false);
    if(unlikely(iRet != 0))
    {
        NetWork::closeSocket(fd);
        return -E_FAIL;
    }

    iRet = NetWork::setReuseAddr(fd);
    if(unlikely(iRet != 0))
    {
        NetWork::closeSocket(fd);
        return -E_FAIL;
    }

    if(sock.type == SOCKET_TCP)
    {
        struct sockaddr_in inAddr;
        inAddr.sin_family = AF_INET;
        inAddr.sin_addr.s_addr = sock.addr.ip;
        inAddr.sin_port = sock.addr.port;

        iRet = NetWork::doBind(fd, inAddr);
        if(unlikely(iRet != 0))
        {
            NetWork::closeSocket(fd);
            return -E_FAIL;
        }

        iRet = NetWork::doListen(fd, 512);
        if(unlikely(iRet != 0))
        {
            NetWork::closeSocket(fd);
            return -E_FAIL;
        }
    }
    else if(sock.type == SOCKET_UNIX)
    {
        struct sockaddr_un unAddr;
        unAddr.sun_family = AF_UNIX;
        snprintf(unAddr.sun_path, sizeof(unAddr.sun_path), "%s", sock.path);

        if(unlink(unAddr.sun_path) != 0 && errno != ENOENT)
        {
            return -E_FAIL;
        }

        iRet = NetWork::doBind(fd, unAddr);
        if(unlikely(iRet != 0))
        {
            NetWork::closeSocket(fd);
            return -E_FAIL;
        }
        
        iRet = NetWork::doListen(fd, 512);
        if(unlikely(iRet != 0))
        {
            NetWork::closeSocket(fd);
            return -E_FAIL;
        }
    }
    else if(sock.type == SOCKET_UDP)
    {
        iRet = NetWork::setRecvBufferSize(fd, 1024 * 1024);
        if(unlikely(iRet != 0))
        {
            NetWork::closeSocket(fd);
            return -E_FAIL;
        }

        struct sockaddr_in inAddr;
        inAddr.sin_family = AF_INET;
        inAddr.sin_addr.s_addr = sock.addr.ip;
        inAddr.sin_port = sock.addr.port;

        iRet = NetWork::doBind(fd, inAddr);
        if(unlikely(iRet != 0))
        {
            NetWork::closeSocket(fd);
            return -E_FAIL;
        }

        // UPD bind INADDR_ANY address, set IP_PKTINFO get local address
        if(sock.addr.ip == INADDR_ANY)
        {
            iRet = NetWork::setPktInfo(fd);
            if(unlikely(iRet != 0))
            {
                NetWork::closeSocket(fd);
                return -E_FAIL;
            }
        }
    }

    return fd;
}

/**
 * 处理TCP socket接入
 */
void SocketAcceptor::handleAccept(ConnInfo *pConn)
{
    int fd;
    int iRet;
    bool bSuccess;
    
    uint64_t flow;
    int acceptCount = 50;
    ConnInfo *pNewConn;
    
    while(acceptCount-- > 0)
    {
        fd = NetWork::doAccept(pConn->fd);
        if(unlikely(fd < 0))
        {
            LOG_ERROR("Accept failed, listInfo:%s, errMsg:%s", m_listen_map[pConn->fd].toString().c_str(), strerror(errno));
            break;
        }

        iRet = NetWork::setBlock(fd, false);
        if(unlikely(iRet != 0))
        {
            NetWork::closeSocket(fd);
            LOG_ERROR("Set unblock failed, errMsg:%s", strerror(errno));
            continue;
        }

        iRet = m_pConnMgr->addConn(flow, fd, pConn->type);
        if(unlikely(iRet != 0))
        {
            NetWork::closeSocket(fd);
            if(m_funcs[CB_OVERLOAD] != NULL)
            {
                m_tranData.data = NULL;
                m_tranData.len = 0;
                m_tranData.extdata = &(pConn->extInfo);
                
                m_funcs[CB_OVERLOAD](0, &m_tranData, m_args[CB_OVERLOAD]);
            }

            break;
        }

        pNewConn = m_pConnMgr->getConnection(flow);
        
        if(pConn->type == SOCKET_TCP)
        {
            NetWork::fdToLocalAddress(fd, pNewConn->extInfo.localIp, pNewConn->extInfo.localPort);
            NetWork::fdToRemoteAddress(fd, pNewConn->extInfo.remoteIp, pNewConn->extInfo.remotePort);
        }

        // 只处理TCP连接鉴权(unix本地调用不需要鉴权)
        if(m_funcs[CB_CONNECTED] != NULL && pConn->type == SOCKET_TCP)
        {
            m_tranData.data = NULL;
            m_tranData.len = 0;
            m_tranData.extdata = &(pNewConn->extInfo);

            iRet = m_funcs[CB_CONNECTED](flow, &m_tranData, m_args[CB_CONNECTED]);
            if(unlikely(iRet != 0))
            {
                LOG_ERROR("Receive unauthority connection.\r\n");
                m_pConnMgr->closeConn(flow);
                continue;
            }
        }

        if(m_listen_map[pConn->fd].tos >= 0)
        {
            setsockopt(fd, IPPROTO_IP, IP_TOS, &(m_listen_map[pConn->fd].tos), sizeof(int));
        }

        bSuccess = m_pSockMgr->add(fd, EV_READ, flow);
        if(unlikely(!bSuccess))
        {
            m_pConnMgr->closeConn(flow);
            continue;
        }
    }
}

/**
 * 处理UDP数据包接入
 */
void SocketAcceptor::handleUdpAccept(ConnInfo *pConn)
{
    int iRet;
    uint64_t flow;
    ConnInfo *pNewConn;
    int acceptCount = 50;
    
    while(acceptCount-- > 0)
    {
        iRet = m_pConnMgr->addConn(flow, pConn->fd, pConn->type);
        if(unlikely(iRet != 0))
        {
            if(m_funcs[CB_OVERLOAD] != NULL)
            {
                m_tranData.data = NULL;
                m_tranData.len = 0;
                m_tranData.extdata = &(pConn->extInfo);
                
                m_funcs[CB_OVERLOAD](0, &m_tranData, m_args[CB_OVERLOAD]);
            }
            
            break;
        }

        iRet = m_pConnMgr->recv(flow, &(m_tranData.data), m_tranData.len);
        if(likely(iRet == E_OK))
        {
            pNewConn = m_pConnMgr->getConnection(flow);

            // UPD 绑定地址为INADDR_ANY则，ip 从pktinfo里面取
            if(m_listen_map[pConn->fd].addr.ip != INADDR_ANY)
            {
                pNewConn->extInfo.localIp = m_listen_map[pConn->fd].addr.ip;
            }
            
            pNewConn->extInfo.localPort = m_listen_map[pConn->fd].addr.port;
            
            m_tranData.extdata = &(pNewConn->extInfo);

            if(m_funcs[CB_CONNECTED] != NULL)
            {
                if(m_funcs[CB_CONNECTED](flow, &m_tranData, m_args[CB_CONNECTED]) != 0)
                {
                    m_pConnMgr->closeConn(flow);
                    continue;
                }
            }

            int iProcessLen = m_funcs[CB_RECVDATA](flow, &m_tranData, m_args[CB_RECVDATA]);
            if(iProcessLen <= 0)
            {
                LOG_WARN("Process UDP packet failed.");
                m_pConnMgr->closeConn(flow);
            }
        }
        else
        {
            m_pConnMgr->closeConn(flow);
            break;
        }
    }
}


/**
 * 超时检测
 */
void SocketAcceptor::checkExpire()
{
    time_t now = time(NULL);
    if(m_iExpireTime != 0 && now - m_lastCheckTime > 1)
    {
        m_lastCheckTime = now;
        m_pConnMgr->checkExpire(now - m_iExpireTime, this);
    }
}

/**
 * 超时处理
 */
void SocketAcceptor::timeoutProcess(uint64_t flow)
{
    ConnInfo* pConn = m_pConnMgr->getConnection(flow);
    if(unlikely(pConn == NULL))
    {
        return;
    }

    if(m_funcs[CB_TIMEOUT] != NULL)
    {
        m_tranData.data = NULL;
        m_tranData.len = 0;
        m_tranData.extdata = NULL;

        m_funcs[CB_TIMEOUT](flow, &m_tranData, m_args[CB_TIMEOUT]);    
    }

    if(pConn->type != SOCKET_UDP)
    {
        m_pSockMgr->del(pConn->fd);
    }
            
    m_pConnMgr->closeConn(flow);
}


