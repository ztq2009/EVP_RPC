/**
  * FileName: RefCounted.h
  * Author: Created by tyreezhang
  * History:
  */

#ifndef __COM_UTIL_REFCOUNTED_H__
#define __COM_UTIL_REFCOUNTED_H__

#include "Exception.h"
#include <algorithm>

namespace Util
{

template<typename T>
struct  DestoryPointer
{
	void operator()(const T* p) const
	{
	    delete p;
	}
};

/**
  * ÷«ƒ‹÷∏’Î
  */
  
template<typename T, typename Destroy = DestoryPointer<T> >
class SmartPtr
{
public:
	typedef SmartPtr<T, Destroy> ThisType;

	explicit SmartPtr(T* p) : m_ptr(p), m_refcount_ptr(NULL)
	{
	    if(m_ptr)
	    {
	        m_refcount_ptr = new int(1);
	    }
	}
	
	SmartPtr(const ThisType& r) : m_ptr(r.m_ptr), m_refcount_ptr(r.m_ptr? r.m_refcount_ptr : NULL)
	{
	    if(m_refcount_ptr)
	    {
	        ++(*m_refcount_ptr);
	    }
	}

	~SmartPtr()
	{
	    if(m_refcount_ptr && (--(*m_refcount_ptr) == 0))
	    {
	         m_destroyFunc(m_ptr);
			 delete m_refcount_ptr;
	    }
	}
	
	ThisType& operator=(T* p)
	{
	    ThisType(p).swap(*this);

		return *this;
	}

	ThisType& operator=(const ThisType& r) 
	{
	    ThisType(r).swap(*this);

		return *this;
	}

	T* operator->() const
	{
	    if(m_ptr) 
	    {
	        THROW_EXCEPTION(CException, -1, "Null pointer exception.");
	    }

		return m_ptr;
	}

	T& operator*() const
	{
	    if(m_ptr) 
	    {
	        THROW_EXCEPTION(CException, -1, "Null pointer exception.");
	    }

		return *m_ptr;
	}

	operator bool() const
	{
	    return m_ptr != NULL;
	}

	operator void*() const
    {
        return m_ptr;
    }

	T* raw() const
	{
	    return m_ptr;
	}

protected:
	void swap(ThisType &r)
	{
	    std::swap(m_ptr, r.m_ptr);
		std::swap(m_refcount_ptr, r.m_refcount_ptr);
	}

private:
	T*       m_ptr;
	int*     m_refcount_ptr; 
	Destroy  m_destroyFunc;
};

}



#endif



