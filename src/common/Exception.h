#ifndef __COM_UTIL_EXCEPTION_HEAD__
#define __COM_UTIL_EXCEPTION_HEAD__

#include <stdio.h>
#include <string>
#include <exception>

class CException : public std::exception
{
public:
	CException(int errNo, const std::string &errDesc, const char *szFile = __FILE__, const int line = __LINE__) throw();

	virtual ~CException() throw();

	inline std::string file(){ return m_file; }
	inline int line(){return m_line;}

	inline int error(){return m_errno;}
	virtual const char* what() const throw();
	
private:
    int            m_errno;
	int            m_line;
	std::string    m_file;
	std::string    m_errDesc;
};


#define  DEFINE_EXCEPTION(clsName)  \
class clsName : public CException \
{ \
public: \
	clsName(int errNo, const std::string &errDesc, const char *szFile = __FILE__, const int line = __LINE__) throw() : CException(errNo, errDesc, szFile, line) {} \
    ~clsName() throw() {} \
}; 

#define THROW_EXCEPTION(cls, errNo, ...)  { \
    char _szBuf[10240] = {0}; \ 
    snprintf(_szBuf, sizeof(_szBuf), ##__VA_ARGS__); \ 
    throw cls(errNo, _szBuf, __FILE__, __LINE__); \ 
}

#endif


