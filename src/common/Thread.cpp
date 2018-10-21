#include "Thread.h"
#include <errno.h>
#include <string.h>

static void* start_hook(void *arg)
{
    Util::Thread* threadPtr = (Util::Thread*)arg;
    Util::ThreadPtr ptr(threadPtr);

    try
    {
        threadPtr->_decRef();
        threadPtr->run();
    }
    catch(...)
    {
    }

    threadPtr->_done();

    return NULL;
}

Util::Thread::Thread() : m_started(false), m_running(false), m_name("")
{

}

Util::Thread::Thread(const std::string &name) : m_started(false), m_running(false), m_name(name)
{
}

Util::Thread::~Thread()
{
}


bool Util::Thread::isAlive()
{
    Mutex::Lock _lock(m_mutex);
    return m_running;
}

void Util::Thread::_done()
{
    Mutex::Lock _lock(m_mutex);
    m_running = false;
}

void Util::Thread::start()
{
    Mutex::Lock _lock(m_mutex);
    if(m_started)
    {
        THROW_EXCEPTION(CException, -1, "repeat start thread.");
    }

     _incRef();

    int ret;
    if((ret = pthread_create(&m_threadId, NULL, start_hook, this)) != 0)
    {
        throw CException(errno, strerror(errno));
    }

    m_started = true;
    m_running = true;
}

void Util::Thread::join()
{
    if(pthread_join(m_threadId, NULL) != 0)
    {
        throw CException(errno, strerror(errno));
    }
}

void Util::Thread::detach()
{
    int ret;
    if((ret = pthread_detach(m_threadId)) != 0)
    {
        throw CException(errno, strerror(errno));
    }
}

pthread_t Util::Thread::ID()
{
    return m_threadId;
}




