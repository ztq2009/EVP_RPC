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
 * 线程基类
 */
class Thread : public Util::Shared
{
public:
	Thread();
	Thread(const std::string &);

	virtual ~Thread();

	 /**
	  * 线程执行方法，具体方法由子类实现
	  */
	virtual void run() = 0;
	 
	/**
	 * 线程是否存活
	 */
	bool isAlive();

	/**
	 * 线程启动
	 */
	void start();

	/**
	 * 置线程为结束状态，不建议外部调用
	 */
	void _done();

	/**
	 * 等待线程运行结束
	 */
	void join();

	/**
	 * 将线程设置为detach, 不需要join释放资源
	 */
	void detach();

	/**
	 * 获取本线程ID标识
	 */
	pthread_t ID();

private:
	/**
	 * 不可复制
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

