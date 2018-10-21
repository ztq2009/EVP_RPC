#include "Exception.h"
#include "ShareMem.h"
#include "ErrorCode.h"
#include <error.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

Util::ShareShm::ShareShm(key_t key, int size) : m_iShmid(-1), m_iSize(size), m_bCreator(true), m_pAddr((void*)-1)
{
    if((m_iShmid = shmget(key, m_iSize, IPC_CREAT | IPC_EXCL | 0666)) < 0)
    {
        if(errno != EEXIST)
        {
            throw CException(E_SHM_ERROR, strerror(errno));
        }

        if((m_iShmid = shmget(key, m_iSize, 0666)) < 0)
        {
            throw CException(E_SHM_ERROR, strerror(errno)); 
        }

        m_bCreator = false;
    }

    if((m_pAddr = shmat(m_iShmid, NULL, 0)) == (void*)-1)
    {
        throw CException(E_SHM_ERROR, strerror(errno)); 
    }

    printf("ShareShm key:%d\r\n", key);
}

Util::ShareShm::~ShareShm()
{
    if(m_pAddr != (void*)-1)
    {
        shmdt(m_pAddr);
    }
}

/**
 * ��ȡ�����ڴ��׵�ַ
 */
void* Util::ShareShm::address()
{
    if(m_pAddr == NULL)
    {
        throw CException(E_NULL_POINTER, "Null Pointer");
    }
    
    return m_pAddr;
}

/**
 * ��ȡ�����ڴ�ID
 */
int Util::ShareShm::shmId()
{
    return m_iShmid;
}

/**
 * ��ȡ�����ڴ��С
 */
int Util::ShareShm::shmSize()
{
    return m_iSize;
}

/**
 * ���ٹ����ڴ�
 */
void Util::ShareShm::destroyShm()
{
    if(m_iShmid > 0)
    {
        shmctl(m_iShmid, IPC_RMID, NULL);
    }
}

/**
 * �Ƿ񴴽���
 */
bool Util::ShareShm::isCreator()
{
    return m_bCreator;
}


Util::ShareMap::ShareMap(string fileName, int len, int offset, int prot, int flag) : m_iSize(len), m_bCreator(true), m_pAddr((void*)-1)
{
    int fd;

    if(fileName.empty())
    {
        fd = -1;
    }
    else
    {
        if((fd = open(fileName.c_str(), O_CREAT | O_EXCL | O_RDWR, 0666)) < 0)
        {
            if(errno != EEXIST)
            {
                throw CException(E_MMAP_ERROR, strerror(errno));
            }

            if((fd = open(fileName.c_str(), O_RDWR, 0)) < 0)
            {
                throw CException(E_MMAP_ERROR, strerror(errno));
            }

            m_bCreator = false;
        }

        if(m_bCreator)
        {
            struct stat st;

            if(fstat(fd, &st) != 0)
            {
                throw CException(E_MMAP_ERROR, strerror(errno));
            }

            if(st.st_size < offset + len)
            {
                ftruncate(fd, offset + len);
            }
        }
    }

    if((m_pAddr = mmap(NULL, len, prot, flag, fd, offset)) == (void*)-1)
    {
        throw CException(E_MMAP_ERROR, strerror(errno));
    }
}

Util::ShareMap::ShareMap(int fd, int len, int offset, int prot, int flag) : m_iSize(len) , m_bCreator(false), m_pAddr((void*)-1)
{
    if(fd < 0)
    {
        m_bCreator = true;
    }
    
    if((m_pAddr = mmap(NULL, len, prot, flag, fd, offset)) == (void*)-1)
    {
        throw CException(E_MMAP_ERROR, strerror(errno));
    }
}

Util::ShareMap::~ShareMap()
{
    if(m_pAddr != (void*)-1)
    {
        munmap(m_pAddr, m_iSize);
    }
}

/**
 * �����ڴ��С
 */
int Util::ShareMap::mmapSize()
{
    return m_iSize;
}

/**
 * �����ڴ��׵�ַ
 */
void* Util::ShareMap::address()
{
    if(m_pAddr == NULL)
    {
        throw CException(E_NULL_POINTER, "Null Pointer");
    }
    
    return m_pAddr;
}

/**
 * �Ƿ񴴽���
 */
bool Util::ShareMap::isCreator()
{
    return m_bCreator;
}

