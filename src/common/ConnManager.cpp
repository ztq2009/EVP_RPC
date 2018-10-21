#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "ConnManager.h"
#include "Network.h"


#define INIT_STLINK(link) link.head = link.tail = NULL

static const unsigned C_READ_BUFFER_SIZE = 5 * 1024 * 1024;
static char RecvBuffer[C_READ_BUFFER_SIZE];

/**
 * 链表队尾新增节点
 */
static void addTail(stItemLink_t *pLink, ConnInfo *pConn)
{
    pConn->next = pConn->prev = NULL;
	
    if(pLink->tail)
    {
        pLink->tail->next = pConn;
		pConn->prev = pLink->tail;
    }
	else
	{
	    pLink->head = pConn;
	}

	pLink->tail = pConn;
}

/**
 * 链表删除节点
 */
static void removeItem(stItemLink_t *pLink, ConnInfo *pConn)
{
    if(pLink->head == pConn)
    {
        pLink->head = pConn->next;
		
		if(pLink->head)
		{
		    pLink->head->prev = NULL;
		}
    }
	else
	{
	    if(pConn->prev)
	    {
	        pConn->prev->next = pConn->next;
	    }
	}

	if(pLink->tail == pConn)
	{
	    pLink->tail = pConn->prev;

		if(pLink->tail)
		{
		    pLink->tail->next = NULL;
		}
	}
	else
	{
	    if(pConn->next)
	    {
	        pConn->next->prev = pConn->prev;
	    }
	}

	pConn->prev = pConn->next = NULL;
}

/**
 * 链表队头删除节点
 */
static ConnInfo* popHead(stItemLink_t *pLink)
{
    ConnInfo* pConn = NULL;

    if(unlikely(pLink->head == NULL))
    {
        return NULL;
    }

    pConn = pLink->head;
    if((pLink->head = pConn->next) != NULL)
    {
        pConn->next->prev = NULL;
    }
    else
    {
        pLink->tail = NULL;
    }
    
    return pConn;    
}

ConnManager::ConnManager(int maxconn) : m_iMaxCon(maxconn), m_iUsed(0), m_uIndex(0)
{
    m_pConnData = (ConnInfo*)malloc(sizeof(ConnInfo) * m_iMaxCon);
    if(m_pConnData == NULL)
    {
        throw CException(E_NO_MEMORY, "ConnManager Malloc Failed.");
    }
    
    INIT_STLINK(m_freeLink);
    INIT_STLINK(m_usedLink);

    for(int i = 0; i < m_iMaxCon; ++i)
    {
        m_pConnData[i].fd = -1;
        m_pConnData[i].seq = 0;
        m_pConnData[i].type = SOCKET_UNKNOW;
        m_pConnData[i].index = i;
        m_pConnData[i].access = 0;

        m_pConnData[i].extInfo.type = SOCKET_UNKNOW;
        m_pConnData[i].extInfo.recvTime = 0;
        
        m_pConnData[i].readBuffer.init();
        m_pConnData[i].writeBuffer.init();
        
        addTail(&m_freeLink, m_pConnData + i);
    }
}

ConnManager::~ConnManager()
{
    for(ConnInfo* pConn = m_usedLink.head; pConn != NULL; pConn = pConn->next)
    {
        // UDP 子请求fd与父请求相同， 只释放一次
        if(pConn->type == SOCKET_UDP && pConn->seq != 0)
        {
            continue;
        }

        NetWork::closeSocket(pConn->fd);
    }
    
    if(m_pConnData)
    {
        free(m_pConnData);
        m_pConnData = NULL;
    }
}

/**
 * 申请一个ConnInfo
 * @input:  fd 文件描述符
 *          type  描述符类型
 *          noSeq 是否分配唯一标识序号(监听接口不分配)
 * @output: flow 连接唯一标识
 * @return:  0: 成功  非0 : 错误码
 */
int ConnManager::addConn(uint64_t &flow, int fd, int type, bool noSeq)
{
    ConnInfo* pConn = popHead(&m_freeLink);
    if(unlikely(pConn == NULL))
    {
        return -E_NO_CONNECT;
    }

    if(m_uIndex == 0)
    {
        ++m_uIndex;
    }

    pConn->fd = fd;
    pConn->type = type;
    pConn->access = getTime();
    pConn->seq = noSeq? 0 : m_uIndex++;
    pConn->extInfo.type = type;

    flow = pConn->flow();

    ++m_iUsed;
    
    addTail(&m_usedLink, pConn);

    return E_OK;    
}

/**
 * 释放ConnInfo
 */
int ConnManager::closeConn(uint64_t flow)
{
    int index = flow & 0xFFFFFFFF;
    uint32_t seq = (flow >> 32) & 0xFFFFFFFF;

    if(unlikely(index < 0 || index >= m_iMaxCon))
    {
        return -E_NOT_FIND;
    }

    ConnInfo* pConn = m_pConnData + index;
    if(unlikely(pConn->seq != seq))
    {
        return -E_NOT_FIND;
    } 

    if(pConn->type != SOCKET_UDP)
    {
        close(pConn->fd);
    }

    removeItem(&m_usedLink, pConn);
    
    pConn->clear();
    
    addTail(&m_freeLink, pConn);

    --m_iUsed;

    return E_OK;
}

/**
 * flow连接收包
 */
int ConnManager::recv(uint64_t flow, char **pBuf, int &iSize)
{
    ConnInfo* pConn = getConnection(flow);
    if(unlikely(pConn == NULL))
    {
        return -E_NOT_FIND;
    }

    if(likely(pBuf != NULL))
    {
        *pBuf = RecvBuffer;
    }
    
    pConn->access = pConn->extInfo.recvTime = getTime();

    int recvLen = 0;
    if(pConn->type != SOCKET_UDP)
    {
        int iLeftLen = pConn->readBuffer.length();

        if(unlikely(iLeftLen >= C_READ_BUFFER_SIZE))
        {
            return -E_NEED_CLOSE;
        }
        
        recvLen = read(pConn->fd, RecvBuffer + iLeftLen, C_READ_BUFFER_SIZE - iLeftLen);
        if(likely(recvLen > 0))
        { 
            // copy 没处理完的数据
            if(unlikely(iLeftLen > 0))
            {
                memcpy(RecvBuffer, pConn->readBuffer.data(), iLeftLen);
                pConn->readBuffer.reset();
            }
            
            iSize = iLeftLen + recvLen;

            return E_OK;
        }
        else if(recvLen == 0)
        {
            return -E_NEED_CLOSE;
        }
        else 
        {
            if(errno != EAGAIN)
            {
                return -E_NEED_CLOSE;
            }

            iSize = iLeftLen;

            return -E_READ_AGAIN;
        }
    }
    else
    {
        char szBuf[1024];
        struct sockaddr_in inAddr;
        struct iovec iov;
        struct msghdr mh;

        iov.iov_base = RecvBuffer;
        iov.iov_len = C_READ_BUFFER_SIZE;
        
        mh.msg_name = &inAddr;
        mh.msg_namelen = sizeof(inAddr);
        mh.msg_iov = &iov;
        mh.msg_iovlen = 1;
        mh.msg_control= szBuf;
        mh.msg_controllen= sizeof(szBuf);
        mh.msg_flags = 0;
        
        recvLen = recvmsg(pConn->fd, &mh, 0);
        if(likely(recvLen > 0))
        {
            pConn->extInfo.remoteIp = inAddr.sin_addr.s_addr;
            pConn->extInfo.remotePort = inAddr.sin_port;

            // 获取本机IP, 对于INADDR_ANY获取真实本机IP
            for( struct cmsghdr* cmsg = CMSG_FIRSTHDR(&mh); cmsg; cmsg = CMSG_NXTHDR(&mh, cmsg))
            {
                if((cmsg->cmsg_level == IPPROTO_IP) && (cmsg->cmsg_type == IP_PKTINFO))
                {
                    struct in_pktinfo* pkinfo = (struct in_pktinfo*)CMSG_DATA(cmsg);
                    pConn->extInfo.localIp = pkinfo->ipi_addr.s_addr;
                    break;
                }
            }

            iSize = recvLen;

            return E_OK;
        }

        return -E_NEED_CLOSE;
    }
}

/**
 * flow连接发包, 返回发送到网络字节数
 */
int ConnManager::send(uint64_t flow, const char *pData, int len)
{
    ConnInfo* pConn = getConnection(flow);
    if(unlikely(pConn == NULL))
    {
        return -E_NOT_FIND;
    }

    pConn->access = getTime();

    int sendLen = 0;
    if(pConn->type != SOCKET_UDP)
    {
        if(pConn->writeBuffer.length() == 0)
        {
            sendLen = write(pConn->fd, pData, len);
            if(unlikely(sendLen < 0))
            {
                if(errno != EAGAIN && errno != EINPROGRESS)
                {
                    return -E_NEED_CLOSE;
                }

                sendLen = 0;
            }
            else
            {
                pData += sendLen;
                len -= sendLen;
            }
        }

        if(len > 0)
        {
            try
            {
                pConn->writeBuffer.append(pData, len);
            }
            catch(...)
            {
                // 内存不够，关闭连接
                return -E_NEED_CLOSE;
            }
        }

        return sendLen;
    }
    else
    {
        struct sockaddr_in inAddr;

        inAddr.sin_family = AF_INET;
        inAddr.sin_addr.s_addr = pConn->extInfo.remoteIp;
        inAddr.sin_port = pConn->extInfo.remotePort;

        sendLen = sendto(pConn->fd, pData, len, 0, (struct sockaddr *)&inAddr, sizeof(inAddr));
        if(unlikely(sendLen != len))
        {
            return -E_NEED_CLOSE;
        }

        return sendLen;
    }
}


/**
 * 缓冲区发包
 */
int ConnManager::sendWriteBuffer(uint64_t flow)
{
    ConnInfo* pConn = getConnection(flow);
    if(unlikely(pConn == NULL))
    {
        return -E_NOT_FIND;
    }

    pConn->access = getTime();

    if(pConn->writeBuffer.length() == 0)
    {
        return 0;
    }

    if(pConn->type != SOCKET_UDP)
    {
        int sendLen = 0;
        sendLen = write(pConn->fd, pConn->writeBuffer.data(), pConn->writeBuffer.length());
        if(unlikely(sendLen < 0))
        {
            if(errno != EAGAIN)
            {
                return -E_NEED_CLOSE;
            }

            return -E_SEND_AGAIN;
        }
        else if(unlikely(sendLen == 0))
        {
            return -E_SEND_AGAIN;
        }
        else
        {
            pConn->writeBuffer.skip(sendLen);
            if(pConn->writeBuffer.length() == 0)
            {
                return 0;
            }
            
            return sendLen;
        }
    }
    else
    {
        return -E_NEED_CLOSE;
    }

    return E_OK;
}

/**
 * 超时检测
 */
void ConnManager::checkExpire(time_t time, ITimeoutProcessor *processor)
{
    ConnInfo* pConn, *pNext;
    for(pConn = m_usedLink.head; pConn != NULL; pConn = pNext)
    {
        // pConn节点超时处理会被删除，预先取下一个节点
        pNext = pConn->next;

        // 监听节点不做超时判断
        if(pConn->seq == 0)
        {
            continue;
        }
        
        if(pConn->access <= time)
        {
            processor->timeoutProcess(pConn->flow());
        }
    }
}

