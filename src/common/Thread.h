/**
  * FileName: Thread.h
  * Author: Created by tyreezhang
  * History:
  */

#ifndef __COM_UTIL_THREAD_H__
#define __COM_UTIL_THREAD_H__

#include <string>
#include "Shared.h"
#include "Mutex.h"
#include "Handle.h"

namespace Util
{

/**
 * �̻߳���
 */
class Thread : public Util::Shared
{
public:
	Thread();
	Thread(const std::string &);

	virtual ~Thread();

	 /**
	  * �߳�ִ�з��������巽��������ʵ��
	  */
	virtual void run() = 0;
	 
	/**
	 * �߳��Ƿ���
	 */
	bool isAlive();

	/**
	 * �߳�����
	 */
	void start();

	/**
	 * ���߳�Ϊ����״̬���������ⲿ����
	 */
	void _done();

	/**
	 * �ȴ��߳����н���
	 */
	void join();

	/**
	 * ���߳�����Ϊdetach, ����Ҫjoin�ͷ���Դ
	 */
	void detach();

	/**
	 * ��ȡ���߳�ID��ʶ
	 */
	pthread_t ID();

private:
	/**
	 * ���ɸ���
	 */
	Thread(const Thread&);
	
	Thread& operator=(const Thread&);
	
private:
	bool                 m_started;
	bool                 m_running;
	Mutex                m_mutex;
	pthread_t            m_threadId;
	const std::string    m_name;
};

typedef Handle<Thread>  ThreadPtr;
}

#endif

