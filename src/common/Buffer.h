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
 * socket��д����
 */
class ConnBuffer
{
public:
	ConnBuffer();
	~ConnBuffer();

	/**
	 * ��ʼ��
	 */
	void init();

	/**
	 * �ɶ������׵�ַ
	 */
	char *data();
	
	/**
	 * �ɶ����ݳ���
	 */
	int length();

	/**
	 * �򻺳���׷������
	 */
	void append(const char* pData, int len);

	/**
	 * �����ݺ��������ĳ���
	 */
	void skip(int len);

	/**
	 * ����������
	 */	
	void reset();

	/**
	 * ����socket�ر�λ
	 */
	inline void setFinBit(bool val)
    {
        m_fin = val;
    }

	/**
	 * ��ȡsocket�ر�λ
	 */	
	inline bool getFinBit()
    {
        return m_fin;
    }

	/**
	 * �������Ƿ�Ϊ��
	 */
	inline bool empty()
	{
	    return m_in == m_out;
	}
	
protected:
	/**
	 * ��λ��
	 */
	int m_in;

	/**
	 * дλ��
	 */
	int m_out;

	/**
	 * ����������
	 */
	int m_capacity;

	/**
	 * socket�ر�λ
	 */
	bool m_fin;

	/**
	 * �������׵�ַ
	 */
	char *m_pData;
};


struct ioBuffer
{
    const void *io_base;   // �����׵�ַ
	size_t io_len;   // ���ݳ���
};

/*
 * �໺�����ṹͷ
 */
struct ioMsghdr
{
    struct ioBuffer* msg_iov;  // �໺������ַ
	int msg_iovlen;  // ����������
};

/*
 * MQ��Ϣͷ��Ϣ
 */
struct mqhead
{
    uint32_t in;   // ��λ��
	uint32_t out;  // дλ��
	uint32_t msg_count; // ��������Ϣ����
	uint32_t capacity;  // ��������
    uint64_t process_count; // ������Ϣ������
	char     reverse[1000]; // ����
};

/**
 * ���ڹ����ڴ��MQ
 */
class ShmMQ
{
public:
	ShmMQ();
	virtual ~ShmMQ();
	
	/**
	 * MQ��ʼ��
	 * @input: shmkey    �����ڴ�keyֵ
	 *         shmsize   �����ڴ��С
	 * @return:  0 : �ɹ�   ��0 : ʧ��
	 */
	int init(int shmKey, int shmsize);

	/**
	 * ���MQ
	 */
	int clear(int type = E_TYPE_NO);
	
	/**
	 * д�����ڴ�MQ(������д��)
	 * @input :   flow  socket��ʶ
	 *            data  �����׵�ַ
	 *            len   ���ݳ���
	 * @return:   >0 : MQ��Ϣ����   < 0 : ������
	 */
	int enqueue(uint64_t flow, const char* data, int len);

	/**
	 * д�����ڴ�MQ(�໺��д��, ���ٺϲ��������ڴ渴��)
	 */
	int enqueue(uint64_t flow, struct ioMsghdr &msghdr);

	/**
	 * ���ѹ����ڴ�MQ����
	 *@input:   flow  socket����Ψһ��ʶ
	 *          buf  �������׵�ַ
	 *          bufSize ����������
	 *@output:  bufSize ʵ���������ݳ���
	 *@return:  0 : �ɹ�   ��0 : ʧ��
	 */
	int dequeue(uint64_t &flow, char* buf, int &bufSize);

protected:
	/**
	 * �����ڴ�д�����ݣ�uPosλ��д�룬 
	 * д��uPosָ����һ��λ�ã����޸�outֵ
	 */
	void putMsg(uint32_t &uPos, const char* data, int len);

	/**
	 * �����ڴ�����ݣ�uPosλ�ö��� 
	 * ����uPosָ����һλ�ã� ���޸�inֵ
	 */
	void getMsg(uint32_t &uPos, char* data, int len);

public:
	/**
	 * �����ڴ滺������ַ
	 */
	int m_shmSize;

	/**
	 * MQ�����׵�ַ
	 */
	char *m_pData;

	/**
	 * MQͷ
	 */
	mqhead *m_pHead;

	/**
	 * �����ڴ����ģ��
	 */
	Util::ShareShm  *m_pShmShare;
};

#endif

