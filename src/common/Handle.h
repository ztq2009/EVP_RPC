/**
  * FileName: Handle.h
  * Author: Created by tyreezhang
  * History:
  */
  

#ifndef __COM_UTIL_HANDLE_H__
#define __COM_UTIL_HANDLE_H__

#include "Exception.h"

namespace Util
{

template<typename T>
class Handle
{
public:
	Handle(T *p = NULL)
	{
	    m_ptr = p;
		if(m_ptr != NULL)
		{
		    m_ptr->_incRef();
		}
	}

	template<typename Y>
	Handle(const Handle<Y> &r)
	{
	    m_ptr = r.m_ptr;
		if(m_ptr != NULL)
		{
		    m_ptr->_incRef();
		}
	}

	Handle(const Handle &r)
	{
	    m_ptr = r.m_ptr;
		if(m_ptr != NULL)
		{
		    m_ptr->_incRef();
		}
	}

	virtual ~Handle()
    {
        if(m_ptr)
        {
            m_ptr->_decRef();
        }
    }

	Handle& operator=(T* p)
	{
	    if(m_ptr != p)
	    {
	        if(p)
	        {
	            p->_incRef();
	        }

			T* ptr = m_ptr;
			m_ptr = p;

			if(ptr)
			{
			    ptr->_decRef();
			}
	    }

		return *this;
	}

	Handle& operator=(const Handle &rhs)
	{
	    if(m_ptr != rhs.m_ptr)
	    {
	        if(rhs.m_ptr)
	        {
	            rhs.m_ptr->_incRef();
	        }

			T* ptr = m_ptr;
			m_ptr = rhs.m_ptr;

			if(ptr)
			{
			    ptr->_decRef();
			}
	    }

		return *this;
	}

    template<typename U>
	Handle& operator=(const Handle<U> &rhs)
    {
        if(m_ptr != rhs.m_ptr)
        {
            if(rhs.m_ptr)
            {
                rhs.m_ptr->_incRef();
            }

			T* ptr = m_ptr;
			m_ptr = rhs.m_ptr;

			if(ptr)
			{
			    ptr->_decRef();
			}
        }
    }

	T* get() const
	{
	    return m_ptr;
	}

	T* operator->() const
	{
	    if(m_ptr == NULL)
	    {
	        THROW_EXCEPTION(CException, -1, "Null pointer.")
	    }

		return m_ptr;
	}

	T& operator*() const
	{
	    if(m_ptr == NULL)
	    {
	        THROW_EXCEPTION(CException, -1, "Null pointer.")
	    }

		return *m_ptr;
	}

	operator bool() const
	{
	    return m_ptr != NULL;
	}

private:
	T* m_ptr;
};

template<typename T, typename U>
inline bool operator==(const Handle<T> &lhs, const Handle<U> &rhs)
{
    T *l = lhs.get();
	T* r = rhs.get();

	if(l && r)
	{
	    return *l == *r;
	}

	return !l && !r;
}

template<typename T, typename U>
inline bool operator!=(const Handle<T> &lhs, const Handle<U> &rhs)
{
    return !operator==(lhs,rhs);
}


template<typename T, typename U>
inline bool operator<(const Handle<T> &lhs, const Handle<U> &rhs)
{
	T *l = lhs.get();
	T* r = rhs.get();

	if(l && r)
	{
	    return *l < *r;
	}

	return !l && r;
}

template<typename T, typename U>
inline bool operator<=(const Handle<T> &lhs, const Handle<U> &rhs)
{
    return  (lhs < rhs || lhs == rhs);
}


template<typename T, typename U>
inline bool operator>(const Handle<T> &lhs, const Handle<U> &rhs)
{
    return  !(lhs < rhs || lhs == rhs);
}

template<typename T, typename U>
inline bool operator>=(const Handle<T> &lhs, const Handle<U> &rhs)
{
    return !(lhs < rhs);
}

}

#endif



