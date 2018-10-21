/**
  * FileName: Mutex.h
  * Author: Created by tyreezhang
  * History:
  */

#ifndef __COM_UTIL_MUTEX_HEAD_H__
#define __COM_UTIL_MUTEX_HEAD_H__

#include "Exception.h"
#include "Lock.h"
#include <pthread.h>

namespace Util
{

class Monitor;

/**
 * »¥³âËø
 **/
class Mutex
{
public:
	typedef LockT<Mutex> Lock;
	
	Mutex();
	~Mutex();

	void lock() const ;
	bool tryLock() const;
	
	void unLock() const;

    
protected:
	void init();

private:
	friend class Monitor;
	
	mutable pthread_mutex_t m_mutex;
};

}

#endif




