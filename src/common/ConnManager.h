#ifndef __CONNECTION_MANAGER_HEAD_H_
#define __CONNECTION_MANAGER_HEAD_H_

#include <stdlib.h>
#include "Exception.h"
#include "Buffer.h"
#include <stdint.h>
#include "IAcceptor.h"
#include "Common.h"

/**
 * socket���������Ϣ
 * һ������һ��ConnInfo
 */
struct ConnInfo
{
    int  fd;    // �ļ�������
	int  type;  // socket���ͣ� TCP_SOCK, UDP_SOCK, UNIX_SOCK��
	int  index; // ConnInfo���� 
	time_t access; // ���һ�η���ʱ��
	uint32_t  seq; // Ψһ��ű�ʾ������������
	
	ConnInfo* next;   // ����next
	ConnInfo* prev;   // ����prev
	connExtInfo extInfo; // ��չ��Ϣ
	ConnBuffer readBuffer; // ��������
	ConnBuffer writeBuffer; //д������

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
 * ˫������
 */
struct stItemLink_t
{
    ConnInfo *head;
	ConnInfo *tail;
};

/**
 * ConnInfo����ģ��
 */
class ConnManager
{
public:
    ConnManager(int maxconn);
	~ConnManager();

	/**
	 * ����һ��ConnInfo
	 * @input:  fd �ļ�������
	 *          type  ����������
	 *          noSeq �Ƿ����Ψһ��ʶ���(�����ӿڲ�����)
	 * @output: flow ����Ψһ��ʶ
	 * @return:  0: �ɹ�  ��0 : ������
	 */
	int addConn(uint64_t &flow, int fd, int type, bool noSeq = false);

	/**
	 * �ͷ�ConnInfo
	 */
	int closeConn(uint64_t flow);

	/**
	 * flow�����հ�
	 */
	int recv(uint64_t flow, char **pBuf, int &iSize);

	/**
	 * flow���ӷ���
	 */
	int send(uint64_t flow, const char *pData, int len);

	/**
	 * ����������
	 */
	int sendWriteBuffer(uint64_t flow);

	/**
	 * ��ʱ���
	 */
	void checkExpire(time_t time, ITimeoutProcessor *processor);

	/**
	 * ����flow��ȡ������Ϣ
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
	 * ���������
	 */
	int m_iMaxCon;

	/**
	 * ʹ��������
	 */
	int m_iUsed;

	/**
	 * seqno ����������
	 */
	uint32_t m_uIndex;

	/**
	 * ������������
	 */
	stItemLink_t m_freeLink;

	/**
	 * ʹ����������
	 */
	stItemLink_t m_usedLink;

	/**
	 * ������Ϣ�׵�ַ
	 */
	ConnInfo *m_pConnData;
};

#endif

