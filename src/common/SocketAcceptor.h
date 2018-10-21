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

// socket��ַ��ʶ
struct st_sock_addr
{
    unsigned ip;
	unsigned short port;
};

// ����socket��ʶ
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

// ��������
struct st_sock_conf
{
    vector<listen_sock_t> entity;
	int maxconn;            // ���������
	int udpAutoClose;       // UDP�ذ��Զ��ر�
	int expireTime;         // ���ӳ�ʱʱ��
	int maxCacheSize;       // ���дcache����
};

/**
 * socket����ģ��
 */
class SocketAcceptor : public IAcceptor, public ITimeoutProcessor
{
public:
    SocketAcceptor();
	
	~SocketAcceptor();

	/**
     * ģ���ʼ��������������Ϣ
     */
    int init(void *config);

	/**
     * ���ݼ���
     */
	int poll(bool block);

	/**
     * ����֪ͨFD
     */
	int addNotify(int groupID);

	/**
     * �ذ����ͣ����ط����ֽ���
     */
	int sendto(uint64_t flow, void *arg1, void* arg2);
	
protected:
	/**
     * ����Э�����ʹ���socket
     */
	int createSocke(const listen_sock_t &sock);

	/**
     * ����TCP socket����
     */
	void handleAccept(ConnInfo *pConn);
	
	/**
     * ����UDP���ݰ�����
     */
	void handleUdpAccept(ConnInfo *pConn);
	
	/**
     * ��ʱ���
     */
	void checkExpire();

	/**
     * ��ʱ����
     */
	void timeoutProcess(uint64_t flow);
	
public:
    /**
     * ֧�����������
     */
	int  m_iMaxConn;

	/**
     *��󷢰�����
     */
	int  m_iSendLimit;

	/**
     * ��ʱʱ��
     */
	int  m_iExpireTime;

	/**
     * UDP�ذ��Զ��ر�
     */
	bool m_bUdpAutoClose;

	/**
     *���һ�γ�ʱ���ʱ��
     */
    time_t m_lastCheckTime;

	/**
     *socket��������(Epoll)
     */
	SocketMgr* m_pSockMgr;

	/**
     *conn��Դ����
     */
	ConnManager* m_pConnMgr; 

	/**
     *���ݽ���
     */
	transmit_data m_tranData;

	/**
     * epoll  event�¼�
     */
	struct epoll_event *m_pEvent;

	/**
     * ��������
     */
	map<int, listen_sock_t> m_listen_map;
};

#endif

