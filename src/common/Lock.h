/**
  * FileName: Lock.h
  * Author: Created by tyreezhang
  * History:
  */
  

#ifndef __COM_UTIL_LOCK_H__
#define __COM_UTIL_LOCK_H__

#include "Exception.h"

namespace Util
{

/**
  * 自动加锁、解锁封装
  */
template<typename T>
class LockT
{
public:
	LockT(T& mutex) : m_acquired(false), m_mutex(mutex)
	{
	    m_mutex.lock();
		m_acquired = true;
	}

	~LockT()
    {
        if(m_acquired)
        {
            m_mutex.unLock();
			m_acquired = false;
        }
    }

	/**
	 * 上锁； 重复上锁会抛异常
	 */
	void acquire()
	{
	    if(m_acquired)
	    {    
	        THROW_EXCEPTION(CException, -1, "repeat acquire lock.");
	    }

		m_mutex.lock();
		m_acquired = true;
	}

	/**
	 * 解锁
	 */
	void release()
	{
	    if(!m_acquired)
	    {
	        THROW_EXCEPTION(CException, -1, "repeat release lock.");
	    }

		m_mutex.unLock();
		m_acquired = false;
	}

	/**
	 * 非阻塞性，获取锁
	 * true: 成功 false:失败
	 */
	bool tryAcquire()
	{
	    if(m_acquired)
	    {    
	        THROW_EXCEPTION(CException, -1, "alread get lock.");
	    }

		m_acquired = m_mutex.tryLock();
		return m_acquired;
	}

	/**
	 * 检查是否获取锁
	 */
	bool acquired()
	{
	    return m_acquired;
	}

private:
	bool      m_acquired;
	T&        m_mutex;
};

/**
  * 自动加锁、解锁封装(指针版)
  */
template<typename T>
class LockP
{
public:
	LockP(T* pMutex) : m_acquired(false), m_pMutex(pMutex)
	{
	    if(m_pMutex)
	    {
			m_pMutex->lock();
			m_acquired = true;
	    }
	}

	~LockP()
    {
        if(m_acquired && m_pMutex)
        {
            m_pMutex->unLock();
			m_acquired = false;
        }
    }

	/**
	 * 上锁； 重复上锁会抛异常
	 */
	void acquire()
	{
	    if(m_acquired && m_pMutex)
	    {    
	        THROW_EXCEPTION(CException, -1, "repeat acquire lock.");
	    }

		if(m_pMutex)
		{
		    m_pMutex->lock();
		    m_acquired = true;
		}
	}

	/**
	 * 解锁
	 */
	void release()
	{
	    if(!m_acquired && m_pMutex)
	    {
	        THROW_EXCEPTION(CException, -1, "repeat release lock.");
	    }

		if(m_pMutex)
	    {
	        m_pMutex->unlock();
			m_acquired = false;
	    }
	}

	/**
	 * 非阻塞性，获取锁
	 * true: 成功 false:失败
	 */
	bool tryAcquire()
	{
	    if(m_acquired && m_pMutex)
	    {    
	        THROW_EXCEPTION(CException, -1, "alread get lock.");
	    }

		if(m_pMutex)
	    {
		    m_acquired = m_pMutex->tryLock();
		    return m_acquired;
		}

		return false;
	}

	/**
	 * 检查是否获取锁
	 */
	bool acquired()
	{
	    return m_acquired;
	}

private:
	bool      m_acquired;
	T*        m_pMutex;
};

}

#endif



