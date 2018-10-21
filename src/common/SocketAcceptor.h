/**
  * FileName: SocketAcceptor.h
  * Author: Created by tyreezhang
  * History:
  */

#ifndef _SOCKET_ACCEPTOR_HEAD_H_
#define _SOCKET_ACCEPTOR_HEAD_H_

#include <map>
#include <vector>
#include "IAcceptor.h"
#include "SocketMgr.h"
#include "ConnManager.h"
#include "Network.h"
#include <sstream>

using std::map;
using std::vector;

typedef struct st_sock_addr sock_addr_t;
typedef struct st_listen_sock listen_sock_t;
typedef struct st_sock_conf sock_conf_t;

// socket地址标识
struct st_sock_addr
{
    unsigned ip;
	unsigned short port;
};

// 监听socket标识
struct st_listen_sock
{
    int type;
    int tos;
	int oob;
	
	union {
		sock_addr_t addr;
		char path[128];
	};

	std::string toString()
    {
        std::ostringstream oss;

		oss << "Listen sock:" << type << "|" << tos << "|" << oob << "|";
		
		if(SOCKET_UNIX == type)
	    {
	        oss << path << std::endl;
	    }
		else
		{
		    oss << NetWork::addrToString(addr.ip, addr.port) << std::endl;
		}

		return oss.str();
    }
};

// 监听配置
struct st_sock_conf
{
    vector<listen_sock_t> entity;
	int maxconn;            // 最大连接数
	int udpAutoClose;       // UDP回包自动关闭
	int expireTime;         // 连接超时时间
	int maxCacheSize;       // 最大写cache长度
};

/**
 * socket接入模块
 */
class SocketAcceptor : public IAcceptor, public ITimeoutProcessor
{
public:
    SocketAcceptor();
	
	~SocketAcceptor();

	/**
     * 模块初始化，加载配置信息
     */
    int init(void *config);

	/**
     * 数据监听
     */
	int poll(bool block);

	/**
     * 加入通知FD
     */
	int addNotify(int groupID);

	/**
     * 回包发送，返回发送字节数
     */
	int sendto(uint64_t flow, void *arg1, void* arg2);
	
protected:
	/**
     * 根据协议类型创建socket
     */
	int createSocke(const listen_sock_t &sock);

	/**
     * 处理TCP socket接入
     */
	void handleAccept(ConnInfo *pConn);
	
	/**
     * 处理UDP数据包接入
     */
	void handleUdpAccept(ConnInfo *pConn);
	
	/**
     * 超时检测
     */
	void checkExpire();

	/**
     * 超时处理
     */
	void timeoutProcess(uint64_t flow);
	
public:
    /**
     * 支持最大连接数
     */
	int  m_iMaxConn;

	/**
     *最大发包长度
     */
	int  m_iSendLimit;

	/**
     * 超时时间
     */
	int  m_iExpireTime;

	/**
     * UDP回包自动关闭
     */
	bool m_bUdpAutoClose;

	/**
     *最后一次超时检测时间
     */
    time_t m_lastCheckTime;

	/**
     *socket监听管理(Epoll)
     */
	SocketMgr* m_pSockMgr;

	/**
     *conn资源管理
     */
	ConnManager* m_pConnMgr; 

	/**
     *数据交互
     */
	transmit_data m_tranData;

	/**
     * epoll  event事件
     */
	struct epoll_event *m_pEvent;

	/**
     * 接入配置
     */
	map<int, listen_sock_t> m_listen_map;
};

#endif

