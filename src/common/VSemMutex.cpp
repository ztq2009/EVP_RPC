#include "VSemMutex.h"
#include "Exception.h"
#include <string.h>
#include <errno.h>
#include "ErrorCode.h"


Util::VSemMutex::VSemMutex(int key, int perm)
{    
       // 信号量已经存在
    if((m_semID = semget(key, 1, IPC_CREAT | IPC_EXCL | perm)) == -1)
    {
        if(errno != EEXIST)
        {
            throw CException(E_SEM_LOCK_INIT_FAIL, strerror(errno));
        }

        if((m_semID = semget(key, 0, 0)) == -1)
        {
            throw CException(E_SEM_LOCK_INIT_FAIL, strerror(errno));
        }
    }
    else 
    {
        // 信号量第一次创建、初始化
        unsigned int initArray[] = {1};
        
        if(semctl(m_semID, 0, SETALL, initArray) < 0)
        {
            throw CException(errno, strerror(errno));
        }
    }
}

Util::VSemMutex::~VSemMutex()
{
}

void Util::VSemMutex::lock() const
{
    struct sembuf sbuf;
    
    sbuf.sem_num = 0;
    sbuf.sem_op  = -1;
    sbuf.sem_flg = SEM_UNDO;

    while(true)
    {
        if(semop(m_semID, &sbuf, 1) < 0)
        {
            if(errno == EINTR)
            {
                continue;
            }
            else
            {
                throw CException(errno, strerror(errno));
            }
        }

        break;
    }
}

bool Util::VSemMutex::tryLock() const
{
    struct sembuf sbuf;
    
    sbuf.sem_num = 0;
    sbuf.sem_op  = -1;
    sbuf.sem_flg = IPC_NOWAIT | SEM_UNDO;

    if(semop(m_semID, &sbuf, 1) == -1)
    {
        return false;
    }

    return true;
}

void Util::VSemMutex::unLock() const
{
    struct sembuf sbuf;
    
    sbuf.sem_num = 0;
    sbuf.sem_op  = 1;
    sbuf.sem_flg = SEM_UNDO;

    while(true)
    {
        if(semop(m_semID, &sbuf, 1) < 0)
        {
            if(errno == EINTR)
            {
                continue;
            }
            else
            {
                throw CException(errno, strerror(errno));
            }
        }

        break;
    }
}

