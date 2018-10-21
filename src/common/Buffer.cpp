#include "Buffer.h"
#include "Exception.h"
#include "ErrorCode.h"
#include "Common.h"
#include <algorithm>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define   MAGIC           0x40676542    //Beg@
#define   MAGIC_SIZE      4   
#define   MQ_HEAD_SIZE    (MAGIC_SIZE + sizeof(uint32_t) + sizeof(uint64_t))    // magic + len + flow

ConnBuffer::ConnBuffer()
{
    init();
}

ConnBuffer::~ConnBuffer()
{
    if(m_pData)
    {
        free(m_pData);
        m_pData = NULL;
    }
}


/**
 * 初始化
 */
void ConnBuffer::init()
{
    m_in = 0;
    m_out = 0;
    m_capacity = 0;
    
    m_fin = false;
    m_pData = NULL;
}


/**
 * 可读数据首地址
 */
char* ConnBuffer::data()
{
    if(m_pData == NULL)
    {
        return NULL;
    }

    if(m_in >= m_capacity)
    {
        reset();
        return NULL;
    }

    return m_pData + m_in;
}

/**
 * 可读数据长度
 */
int ConnBuffer::length()
{ 
    return m_out - m_in;
}

/**
 * 向缓冲区追加数据
 */
void ConnBuffer::append(const char* pData, int len)
{
    if(m_fin)
    {
        return;
    }
    
    if(m_pData == NULL)
    {
        m_capacity = std::max(DEFAULT_BUFFER_SIZE, alignPageSize(len));
        m_pData = (char*)malloc(m_capacity);

        if(m_pData == NULL)
        {
            m_capacity = 0;
            throw CException(E_NO_MEMORY, "Without Enough Memroy.");
        }
    }

    if(m_out + len <= m_capacity)
    {
        memcpy(m_pData + m_out, pData, len);
        m_out += len;
    }
    else
    {
        int dataLen = m_out - m_in;
        if(dataLen + len <= m_capacity)
        {
            memmove(m_pData, m_pData + m_in, dataLen);
            memcpy(m_pData + dataLen, pData, len);
            m_in = 0;
            m_out = dataLen + len;
        }
        else
        {
            int allocSize = m_capacity + alignPageSize(len);
            char *pAddr = (char*)malloc(allocSize);

            if(pAddr == NULL)
            {
                throw CException(E_NO_MEMORY, "Realloc Without Memory.");
            }

            memcpy(pAddr, m_pData + m_in, dataLen);
            memcpy(pAddr + dataLen, pData, len);

            delete m_pData;
            m_pData = pAddr;
            m_in = 0;
            m_out = dataLen + len;
        }
    }
}

/**
 * 读数据后跳过读的长度
 */
void ConnBuffer::skip(int len)
{
    if(m_pData == NULL)
    {
        return;
    }
    
    if(m_in + len >= m_out)
    {
        free(m_pData);
        m_pData = NULL;

        m_in = 0;
        m_out = 0;
        m_capacity = 0;
    }
    else
    {
        m_in += len;
    }
}

/**
 * 缓冲区重置
 */ 
void ConnBuffer::reset()
{
    if(m_pData) 
    {
        free(m_pData);
        m_pData = NULL;
    }

    m_in = 0;
    m_out = 0;
    m_capacity = 0;
    m_fin = false;
}


ShmMQ::ShmMQ()
{
    m_shmSize = 0;
    m_pData = NULL;
    m_pShmShare = NULL;
    
}

ShmMQ::~ShmMQ()
{
    if(m_pShmShare)
    {
        delete m_pShmShare;
		m_pShmShare = NULL;
    }
}

/**
 * MQ初始化
 * @input: shmkey    共享内存key值
 *         shmsize   共享内存大小
 * @return:  0 : 成功   非0 : 失败
 */
int ShmMQ::init(int shmKey, int shmsize)
{
    int iAllocSize = sizeof(struct mqhead) + shmsize;

    try
    {
        m_pShmShare = new Util::ShareShm(shmKey, iAllocSize);
        m_pHead = (mqhead*)m_pShmShare->address();
        m_pData = (char*)(m_pHead + 1);
        m_shmSize = shmsize;
        
        if(m_pShmShare->isCreator())
        {
            memset(m_pHead, 0, sizeof(mqhead));
            m_pHead->capacity = shmsize;
        }

        if(m_pHead->capacity != shmsize)
        {
            return -E_FAIL;
        }

        mlock(m_pHead, 24);
    }
    catch(CException &e)
    {
        return -E_FAIL;
    }
}

/**
 * 清除MQ; 根据类型清除
 */
int ShmMQ::clear(int type)
{
    if(unlikely(m_pHead == NULL))
    {
        return -E_FAIL;
    }

    if(type == E_TYPE_PRODUCE)
    {
        m_pHead->out = m_pHead->in;
    }
    else if(type == E_TYPE_CONSUME)
    {
        m_pHead->in = m_pHead->out;
    }
    else
    {
        m_pHead->in  = 0;
        m_pHead->out = 0;
    }

    __sync_lock_test_and_set(&(m_pHead->msg_count), 0);

    return E_OK;
}




/**
 * 共享内存读数据，uPos位置读， 
 * 读后uPos指向下一位置， 不修改in值
 */
void ShmMQ::getMsg(uint32_t &uPos, char* data, int len)
{
    if(unlikely(data == NULL || len == 0))
    {
        return;
    }

    uint32_t uTailLen = m_shmSize - uPos;
    if(len <= uTailLen)
    {
        memcpy(data, m_pData + uPos, len);
        uPos += len;
        
        if(uPos == m_shmSize)
        {
            uPos = 0;
        }
    }
    else
    {
        memcpy(data, m_pData + uPos, uTailLen);
        memcpy(data + uTailLen, m_pData, len - uTailLen);
        uPos = len - uTailLen;
    }
}

/**
 * 共享内存写入数据，uPos位置写入， 
 * 写入uPos指向下一个位置，不修改out值
 */
void ShmMQ::putMsg(uint32_t &uPos, const char* data, int len)
{
    uint32_t uTailLen = m_shmSize - uPos;

    if(len <= uTailLen)
    {
        memcpy(m_pData + uPos, data, len);
        uPos += len;

        if(uPos == m_shmSize)
        {
            uPos = 0;
        }
    }
    else
    {
       memcpy(m_pData + uPos, data, uTailLen);
       memcpy(m_pData, data + uTailLen, len - uTailLen);
       uPos = len - uTailLen;
    }
}

/**
 * 写共享内存MQ(单缓冲写入)
 */
int ShmMQ::enqueue(uint64_t flow, const char* data, int len)
{
    struct ioBuffer iobuffer;
    struct ioMsghdr msghdr;

    iobuffer.io_base = data;
    iobuffer.io_len = len;

    msghdr.msg_iov = &iobuffer;
    msghdr.msg_iovlen = 1;

    return enqueue(flow, msghdr);
    
}

/**
 * 写共享内存MQ(多缓冲写入)
 */
int ShmMQ::enqueue(uint64_t flow, struct ioMsghdr &msghdr)
{
    char szBuffer[MQ_HEAD_SIZE];

    if(unlikely(m_pHead->in >= m_shmSize || m_pHead->out >= m_shmSize))
    {
        return -E_SHMMQ_CEHCK_FAIL;
    }
    
    uint32_t uFreeLen = m_pHead->out >= m_pHead->in? m_shmSize - (m_pHead->out - m_pHead->in) : m_pHead->in - m_pHead->out;
    uint32_t uDataLen = 0;

    for(int i = 0; i < msghdr.msg_iovlen; ++i)
    {
        uDataLen += msghdr.msg_iov[i].io_len;
    }

    if(unlikely(MQ_HEAD_SIZE + uDataLen >= uFreeLen))
    {
        return -E_SHMMQ_FULL;
    }

    uint32_t uPos = m_pHead->out;
    uint32_t uTotalLen = MQ_HEAD_SIZE + uDataLen;

    *(uint32_t*)szBuffer = MAGIC;
    *(uint32_t*)(szBuffer + MAGIC_SIZE) = uTotalLen;
    memcpy(szBuffer + MAGIC_SIZE + sizeof(uint32_t), &flow, sizeof(uint64_t));

    // 写消息头
    putMsg(uPos, szBuffer, MQ_HEAD_SIZE);

    // 写具体消息内容
    for(int i = 0; i < msghdr.msg_iovlen; ++i)
    {
        putMsg(uPos, (char*)msghdr.msg_iov[i].io_base, msghdr.msg_iov[i].io_len);
    }

    m_pHead->out = uPos;

    return __sync_add_and_fetch(&(m_pHead->msg_count), 1);

}

/**
 * 消费共享内存MQ数据
 *@input:   flow  socket连接唯一标识
 *          buf  缓冲区首地址
 *          bufSize 缓冲区长度
 *@output:  bufSize 实际消费数据长度
 *@return:  0 : 成功   非0 : 失败
 */
int ShmMQ::dequeue(uint64_t &flow, char* buf, int &bufSize)
{
    if(unlikely(m_pHead->in >= m_shmSize || m_pHead->out >= m_shmSize))
    {
        return -E_SHMMQ_CEHCK_FAIL;
    }

    if(m_pHead->in == m_pHead->out)
    {
        return -E_SHMMQ_EMPTY;
    }

    uint32_t uDataLen = (m_pHead->out >= m_pHead->in)? m_pHead->out - m_pHead->in : m_shmSize - (m_pHead->in - m_pHead->out);
    if(unlikely(uDataLen < MQ_HEAD_SIZE))
    {
        return -E_SHMMQ_CEHCK_FAIL;
    }

    char szBuf[MQ_HEAD_SIZE];
    uint32_t uPos = m_pHead->in;

    getMsg(uPos, szBuf, MQ_HEAD_SIZE);

    //检查MQ头
    if(unlikely(*(uint32_t*)szBuf != MAGIC))
    {
        return -E_SHMMQ_CEHCK_FAIL;
    }

    // 长度合法性检查
    uint32_t uMsgTotalLen = *(uint32_t*)(szBuf + MAGIC_SIZE);
    if(unlikely(uMsgTotalLen < MQ_HEAD_SIZE || uMsgTotalLen > uDataLen))
    {
        return -E_SHMMQ_CEHCK_FAIL;
    }

    if(unlikely(bufSize < uMsgTotalLen - MQ_HEAD_SIZE))
    {
        return -E_SHMMQ_NO_BUFFER;
    }
    
    // 消息条数先减， 再处理
    __sync_sub_and_fetch(&(m_pHead->msg_count), 1);
    
    flow = *(uint64_t*)(szBuf + MAGIC_SIZE + sizeof(uint32_t));

    // 读消息数据
    if(uMsgTotalLen > MQ_HEAD_SIZE)
    {
        getMsg(uPos, buf, uMsgTotalLen - MQ_HEAD_SIZE);
    }

    m_pHead->in = uPos;
    bufSize = uMsgTotalLen - MQ_HEAD_SIZE;
    
    __sync_add_and_fetch(&(m_pHead->process_count), 1);

    return E_OK;
}

