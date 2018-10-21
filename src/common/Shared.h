/**
  * FileName: Shared.h
  * Author: Created by tyreezhang
  * History:
  */

#ifndef __COM_UTIL_SHARED_H__
#define __COM_UTIL_SHARED_H__

#include <assert.h>
#include <unistd.h>

namespace Util
{

/**
  * 引用计数基类, 与Handle配合使用
  * 实现通用智能指针功能
  */
class Shared
{
public:
    Shared() :  m_noDelete(false), m_refCount(0)
    {
    }

	virtual ~Shared()
	{
	    
	}

	/**
	 * 引用计数加
	 */
	void _incRef()
	{
	   int count =  __sync_fetch_and_add(&m_refCount, 1);
	   assert(count >= 0);
	}

	/**
	 * 引用计数减
	 */
	void _decRef()
	{
	    int count = __sync_fetch_and_sub(&m_refCount, 1);
		if(count == 1 && !m_noDelete)
		{
		    m_noDelete = true;
			delete this;
		}
	}

	/**
	 * 读引用计数
	 */
	int getRef()
	{
		return __sync_fetch_and_sub(&m_refCount, 0);
	}

	/**
	 * 设置为非自动删除
	 */
	void _setNoDelete(bool noDelete)
	{
	    m_noDelete = noDelete;
	}
	
private:
	volatile bool m_noDelete;
	volatile int  m_refCount;
};

}


#endif




