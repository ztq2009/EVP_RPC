#ifndef __CONNECTION_MANAGER_HEAD_H_
#define __CONNECTION_MANAGER_HEAD_H_

#include <stdlib.h>
#include "Exception.h"
#include "Buffer.h"
#include <stdint.h>
#include "IAcceptor.h"
#include "Common.h"

/**
 * socket连接相关信息
 * 一个连接一个ConnInfo
 */
struct ConnInfo
{
    int  fd;    // 文件描述符
	int  type;  // socket类型， TCP_SOCK, UDP_SOCK, UNIX_SOCK等
	int  index; // ConnInfo索引 
	time_t access; // 最后一次访问时间
	uint32_t  seq; // 唯一序号标示，防过期请求
	
	ConnInfo* next;   // 链表next
	ConnInfo* prev;   // 链表prev
	connExtInfo extInfo; // 扩展信息
	ConnBuffer readBuffer; // 读缓冲区
	ConnBuffer writeBuffer; //写缓冲区

	void clear() 
	{
	    fd = -1;
		type = SOCKET_UNKNOW;
		seq = 0;
		access = 0;
		
		extInfo.type = SOCKET_UNKNOW;
		extInfo.recvTime = 0;
		extInfo.localIp = 0;
		extInfo.localPort = 0;
		extInfo.remoteIp = 0;
		extInfo.remotePort = 0;

		readBuffer.reset();
		writeBuffer.reset();
	}

	uint64_t flow()
    {
        uint64_t flow;
        flow = (uint32_t)index | ((uint64_t)seq << 32);

		return flow;
    }
};


/**
 * 双向链表
 */
struct stItemLink_t
{
    ConnInfo *head;
	ConnInfo *tail;
};

/**
 * ConnInfo管理模块
 */
class ConnManager
{
public:
    ConnManager(int maxconn);
	~ConnManager();

	/**
	 * 申请一个ConnInfo
	 * @input:  fd 文件描述符
	 *          type  描述符类型
	 *          noSeq 是否分配唯一标识序号(监听接口不分配)
	 * @output: flow 连接唯一标识
	 * @return:  0: 成功  非0 : 错误码
	 */
	int addConn(uint64_t &flow, int fd, int type, bool noSeq = false);

	/**
	 * 释放ConnInfo
	 */
	int closeConn(uint64_t flow);

	/**
	 * flow连接收包
	 */
	int recv(uint64_t flow, char **pBuf, int &iSize);

	/**
	 * flow连接发包
	 */
	int send(uint64_t flow, const char *pData, int len);

	/**
	 * 缓冲区发包
	 */
	int sendWriteBuffer(uint64_t flow);

	/**
	 * 超时检测
	 */
	void checkExpire(time_t time, ITimeoutProcessor *processor);

	/**
	 * 根据flow获取连接信息
	 */
	inline ConnInfo* getConnection(uint64_t flow)
	{
	    int index = flow & 0xFFFFFFFF;
	    uint32_t seq = (flow >> 32) & 0xFFFFFFFF;

	    if(unlikely(index < 0 || index >= m_iMaxCon || m_pConnData[index].seq != seq))
	    {
	        return NULL;
	    }

	    return m_pConnData + index;
	}
	
protected:
	/**
	 * 最大连接数
	 */
	int m_iMaxCon;

	/**
	 * 使用连接数
	 */
	int m_iUsed;

	/**
	 * seqno 递增生成数
	 */
	uint32_t m_uIndex;

	/**
	 * 空闲连接链表
	 */
	stItemLink_t m_freeLink;

	/**
	 * 使用连接链表
	 */
	stItemLink_t m_usedLink;

	/**
	 * 连接信息首地址
	 */
	ConnInfo *m_pConnData;
};

#endif

