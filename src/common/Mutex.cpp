#include "Mutex.h"
#include <errno.h>
#include <string.h>

Util::Mutex::Mutex()
{
    init();
}

Util::Mutex::~Mutex()
{
    pthread_mutex_destroy(&m_mutex);
}

void Util::Mutex::init()
{
    int ret;
    
    if((ret = pthread_mutex_init(&m_mutex, NULL)) != 0)
    {
        throw CException(errno, strerror(errno));
    }

}

void Util::Mutex::lock() const 
{
    int ret;
    if((ret = pthread_mutex_lock(&m_mutex)) != 0)
    {
        throw CException(errno, strerror(errno)); 
    }
}

bool Util::Mutex::tryLock() const
{
    int ret;

    ret = pthread_mutex_trylock(&m_mutex);
    if(ret != 0 && ret != EBUSY)
    {
        throw CException(errno, strerror(errno)); 
    }

    return ret == 0;
}

void Util::Mutex::unLock() const
{
    int ret;
    if((ret = pthread_mutex_unlock(&m_mutex)) != 0)
    {
        throw CException(errno, strerror(errno)); 
    }
}


