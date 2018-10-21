#include "Exception.h"

CException::CException(int errNo, const std::string &errDesc, const char *szFile, const int line) throw()
{
    m_errno = errNo;
    m_errDesc = errDesc;

    m_file = szFile;
    m_line = line;
}

CException::~CException() throw()
{
}

const char* CException::what() const throw()
{
    return m_errDesc.c_str();
}
	



