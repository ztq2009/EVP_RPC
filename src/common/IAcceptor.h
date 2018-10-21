#ifndef __INTERFACE_CONNECTOR_HEAD_H_
#define __INTERFACE_CONNECTOR_HEAD_H_

#include <stdint.h>
#include <string.h>
#include <time.h>

/**
 * 回调函数类型
 */
enum 
{
    CB_CONNECTED = 0,
	CB_DISCONNECT, 
	CB_RECVDATA,
	CB_RECVDONE,
	CB_SENDDATA,
	CB_SENDERROR,
	CB_SENDDONE,
	CB_OVERLOAD,
	CB_TIMEOUT
};

/**
 * 监听类型
 */
enum
{
    SOCKET_TCP = 0,
	SOCKET_UDP,
	SOCKET_UNIX,
	SOCKET_NOTIFY,
	SOCKET_CONTROL,
	SOCKET_UNKNOW
};

/**
 * 数据传输类、一般为回调函数arg1参数
 */
struct transmit_data
{
    int len;
	char* data;
	void* owner;
	void* extdata;
};

/**
 * 连接扩展信息(作为transmit_data中extdata部分)
 */
struct connExtInfo
{
    int type;
	unsigned int localIp;
	unsigned int remoteIp;
	unsigned short localPort;
	unsigned short remotePort;
	time_t recvTime;
};

/**
 * 回调函数定义
 */
typedef int (*cb_func)(uint64_t flow, void* arg1, void *arg2);

/**
 * 接入处理基类
 */
class IAcceptor
{
public:
    IAcceptor()
    {
        memset(m_funcs, 0, sizeof(m_funcs));
		memset(m_args, 0, sizeof(m_args));
    }

	virtual ~IAcceptor()
	{
	}

	virtual int init(void *config) = 0;

	virtual int poll(bool block) = 0;

	virtual int sendto(uint64_t flow, void *arg1, void* arg2) = 0;

	virtual int regCallBack(int type, cb_func func, void* arg)
	{
	    if(type <= CB_TIMEOUT)
	    {
	        m_funcs[type] = func;
			m_args[type] = arg;
	    }

		return 1;
	}

protected:
	cb_func m_funcs[CB_TIMEOUT + 1];
	void*   m_args[CB_TIMEOUT + 1];
};


/**
 * 超时处理基类
 */
class ITimeoutProcessor
{
public:
    virtual void timeoutProcess(uint64_t flow) = 0;
};

#endif




