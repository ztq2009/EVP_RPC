/**
  * FileName: Monitor.h
  * Author: Created by tyreezhang
  * History:
  */

#ifndef __COM_UTIL_MONITOR_H__
#define __COM_UTIL_MONITOR_H__

#include <pthread.h>
#include "Mutex.h"
#include "Lock.h"

namespace Util
{

class Monitor
{
public:
	typedef LockT<Monitor> Lock;

	Monitor();
	~Monitor();

	void lock() ;
	bool tryLock();
	
	void unLock();
	void wait();
	
	bool timedWait(int millsec);
	void notify();

	void notifyAll();
	

protected:
    void init();
	void notifyImpl();

private:
	int             m_nnotify;
	Mutex           m_mutex;
	pthread_cond_t  m_cond;
	
	
};

}

#endif

