#include "Monitor.h"
#include "Exception.h"
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

Util::Monitor::Monitor() : m_nnotify(0)
{
    init();
}

Util::Monitor::~Monitor()
{
    pthread_cond_destroy(&m_cond);
}

void Util::Monitor::lock() 
{
    m_mutex.lock();

    m_nnotify = 0;
}

bool Util::Monitor::tryLock()
{
    bool isLock = m_mutex.tryLock();
    
    if(isLock)
    {
        m_nnotify = 0;
    }
    
    return isLock;
}

void Util::Monitor::unLock()
{
    notifyImpl();
    
    m_mutex.unLock();
}

void Util::Monitor::wait()
{
    int ret;
    
    notifyImpl();
    
    if((ret = pthread_cond_wait(&m_cond, &(m_mutex.m_mutex))) != 0)
    {
 
       throw CException(errno, strerror(errno));
    }
}

bool Util::Monitor::timedWait(int millsec)
{
    int ret;
    struct timeval a,b,res;
    struct timespec sp;

    b.tv_sec  = millsec / 1000;
    b.tv_usec = (millsec % 1000) * 1000;
        
    gettimeofday(&a, NULL);
    timeradd(&a, &b, &res);
    
    sp.tv_sec  = res.tv_sec;
    sp.tv_nsec = res.tv_usec * 1000;

    notifyImpl();
    
    ret = pthread_cond_timedwait(&m_cond, &(m_mutex.m_mutex), &sp);
    if(ret)
    {
        if(ret != ETIMEDOUT)
        {
            throw CException(errno, strerror(errno));
        }

        return false;
    }

    return true;
}

void Util::Monitor::notify()
{
    if(m_nnotify != -1)
	{
		++m_nnotify;
	}
}

void Util::Monitor::notifyAll()
{
	m_nnotify = -1;
}



void Util::Monitor::init()
{
	int ret;
	pthread_condattr_t condAttr;
	
	if((ret = pthread_condattr_init(&condAttr)) != 0)
	{
		throw CException(errno, strerror(errno));
	}
	
	if((ret = pthread_cond_init(&m_cond, &condAttr)) != 0)
	{
		pthread_condattr_destroy(&condAttr);
		throw CException(errno, strerror(errno));
	}
	
	if((ret = pthread_condattr_destroy(&condAttr)) != 0)
	{
		throw CException(errno, strerror(errno));
	}
}

void Util::Monitor::notifyImpl()
{
	int ret;
	
	if(m_nnotify != 0)
	{
		if(m_nnotify == -1)
		{
			if((ret = pthread_cond_broadcast(&m_cond)) != 0)
			{
				throw CException(errno, strerror(errno));
			}
		}
		else
		{
			while(m_nnotify-- > 0)
			{
				if((ret = pthread_cond_signal(&m_cond)) != 0)
				{
					throw CException(errno, strerror(errno));
				}
			}
		}
	}
}


	
	


