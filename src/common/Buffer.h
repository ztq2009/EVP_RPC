#ifndef __CONNECTOR_BUFFER_HEAD_H_
#define __CONNECTOR_BUFFER_HEAD_H_

#include <stdlib.h>
#include <string>
#include "ShareMem.h"

#define   DEFAULT_BUFFER_SIZE  8192

enum 
{
    E_TYPE_PRODUCE,
    E_TYPE_CONSUME,
    E_TYPE_NO
};

/**
 * socket读写缓冲
 */
class ConnBuffer
{
public:
	ConnBuffer();
	~ConnBuffer();

	/**
	 * 初始化
	 */
	void init();

	/**
	 * 可读数据首地址
	 */
	char *data();
	
	/**
	 * 可读数据长度
	 */
	int length();

	/**
	 * 向缓冲区追加数据
	 */
	void append(const char* pData, int len);

	/**
	 * 读数据后跳过读的长度
	 */
	void skip(int len);

	/**
	 * 缓冲区重置
	 */	
	void reset();

	/**
	 * 设置socket关闭位
	 */
	inline void setFinBit(bool val)
    {
        m_fin = val;
    }

	/**
	 * 获取socket关闭位
	 */	
	inline bool getFinBit()
    {
        return m_fin;
    }

	/**
	 * 缓冲区是否为空
	 */
	inline bool empty()
	{
	    return m_in == m_out;
	}
	
protected:
	/**
	 * 读位置
	 */
	int m_in;

	/**
	 * 写位置
	 */
	int m_out;

	/**
	 * 缓冲区容量
	 */
	int m_capacity;

	/**
	 * socket关闭位
	 */
	bool m_fin;

	/**
	 * 缓冲区首地址
	 */
	char *m_pData;
};


struct ioBuffer
{
    const void *io_base;   // 数据首地址
	size_t io_len;   // 数据长度
};

/*
 * 多缓冲区结构头
 */
struct ioMsghdr
{
    struct ioBuffer* msg_iov;  // 多缓冲区地址
	int msg_iovlen;  // 缓冲区个数
};

/*
 * MQ消息头信息
 */
struct mqhead
{
    uint32_t in;   // 读位置
	uint32_t out;  // 写位置
	uint32_t msg_count; // 队列中消息条数
	uint32_t capacity;  // 队列容量
    uint64_t process_count; // 处理消息总条数
	char     reverse[1000]; // 保留
};

/**
 * 基于共享内存的MQ
 */
class ShmMQ
{
public:
	ShmMQ();
	virtual ~ShmMQ();
	
	/**
	 * MQ初始化
	 * @input: shmkey    共享内存key值
	 *         shmsize   共享内存大小
	 * @return:  0 : 成功   非0 : 失败
	 */
	int init(int shmKey, int shmsize);

	/**
	 * 清除MQ
	 */
	int clear(int type = E_TYPE_NO);
	
	/**
	 * 写共享内存MQ(单缓冲写入)
	 * @input :   flow  socket标识
	 *            data  数据首地址
	 *            len   数据长度
	 * @return:   >0 : MQ消息个数   < 0 : 错误码
	 */
	int enqueue(uint64_t flow, const char* data, int len);

	/**
	 * 写共享内存MQ(多缓冲写入, 减少合并带来的内存复制)
	 */
	int enqueue(uint64_t flow, struct ioMsghdr &msghdr);

	/**
	 * 消费共享内存MQ数据
	 *@input:   flow  socket连接唯一标识
	 *          buf  缓冲区首地址
	 *          bufSize 缓冲区长度
	 *@output:  bufSize 实际消费数据长度
	 *@return:  0 : 成功   非0 : 失败
	 */
	int dequeue(uint64_t &flow, char* buf, int &bufSize);

protected:
	/**
	 * 共享内存写入数据，uPos位置写入， 
	 * 写入uPos指向下一个位置，不修改out值
	 */
	void putMsg(uint32_t &uPos, const char* data, int len);

	/**
	 * 共享内存读数据，uPos位置读， 
	 * 读后uPos指向下一位置， 不修改in值
	 */
	void getMsg(uint32_t &uPos, char* data, int len);

public:
	/**
	 * 共享内存缓冲区地址
	 */
	int m_shmSize;

	/**
	 * MQ数据首地址
	 */
	char *m_pData;

	/**
	 * MQ头
	 */
	mqhead *m_pHead;

	/**
	 * 共享内存管理模块
	 */
	Util::ShareShm  *m_pShmShare;
};

#endif

