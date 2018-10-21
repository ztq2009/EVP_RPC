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
  * ���ü�������, ��Handle���ʹ��
  * ʵ��ͨ������ָ�빦��
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
	 * ���ü�����
	 */
	void _incRef()
	{
	   int count =  __sync_fetch_and_add(&m_refCount, 1);
	   assert(count >= 0);
	}

	/**
	 * ���ü�����
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
	 * �����ü���
	 */
	int getRef()
	{
		return __sync_fetch_and_sub(&m_refCount, 0);
	}

	/**
	 * ����Ϊ���Զ�ɾ��
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




