#include <stdarg.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include "CLog.h"
#include "Common.h"


/**
 * ÿ�μ�¼����־����
 */
#define WRITE_LOG(buf, str) \
    int iBufLen = 0; \
    time_t tnow = time(NULL); \
    localtime_r(&tnow, &_tm_now); \
    iBufLen = snprintf(buf, sizeof(buf)-2, \
            "[%04d-%02d-%02d %02d:%02d:%02d][%d][%s][%s]", \
            _tm_now.tm_year + 1900, _tm_now.tm_mon + 1, \
            _tm_now.tm_mday, _tm_now.tm_hour, _tm_now.tm_min, _tm_now.tm_sec, \
            getpid(), str, _msg_id.c_str()); \
\
    va_list ap;\
    va_start(ap, fmt);\
    iBufLen += vsnprintf(buf + iBufLen, sizeof(buf) - iBufLen - 2, fmt, ap);\
    va_end(ap);\
    buf[iBufLen] = '\n';\
    buf[++iBufLen] = '\0';\
    return _write(buf, iBufLen);

static const char *gLevelDesc[] = {"NONE", "NONE", "ERROR", "WARNING", "NORMAL", "DEBUG"};

/**
 * ���캯��
 */
CLog::CLog(const char *path, unsigned int max_size, unsigned int max_num, SHIFT_MODE shft_mod)
{
    // ��Ա��ʼ��
    _path = path;
    _max_size = max_size;
    _max_file = max_num;
    _level = _DEBUG;
    _shift_mode = shft_mod;
    _fd = -1;
}


/**
 * ��������
 */
CLog::~CLog()
{
    // �ر��ļ�
    _close();
}

/**
 * ���ļ�
 */
int CLog::_open()
{
    string strFileName = _file_name();
    
    // �ȹر��ļ�
    _close();

    // ���ļ�
    if ((_fd = open(strFileName.c_str(), O_CREAT | O_WRONLY | O_APPEND, 0644)) < 0) 
    {
        _fd = -1;
        return -1;
    }

    return 0;
}

/**
 * �ر��ļ�
 */
void CLog::_close()
{
    if (_fd != -1) 
    {
        close(_fd);
        _fd = -1;
    }
}


/**
 * ��ӡ��־
 */
int CLog::LogInfo(LOG_LEVEL level, const char* fmt, ...)
{
    char szBuf[MAX_BUF];
    
    if(unlikely(level > _level || level < _NONE || level > _DEBUG))
    {
        return -1;
    }

    int iBufLen = 0;
    time_t tnow = time(NULL);
    localtime_r(&tnow, &_tm_now);

    iBufLen = snprintf(szBuf, sizeof(szBuf) - 2, "[%04d-%02d-%02d %02d:%02d:%02d][%d][%s][%s]",
                       _tm_now.tm_year + 1900, _tm_now.tm_mon + 1, _tm_now.tm_mday, _tm_now.tm_hour, 
                       _tm_now.tm_min, _tm_now.tm_sec, getpid(), gLevelDesc[level], _msg_id.c_str());
    
    va_list ap;
    va_start(ap, fmt);

    iBufLen += vsnprintf(szBuf + iBufLen, sizeof(szBuf) - iBufLen - 2, fmt, ap);
    va_end(ap);
    szBuf[iBufLen] = '\n';
    szBuf[++iBufLen] = '\0';

    return _write(szBuf, iBufLen);
}


/**
 * ��ӡ����log
 */
int CLog::error(const char *fmt, ...)
{
    char buf[MAX_BUF];
    
    if (_level < _ERROR) 
    {
        return 0;
    }

    WRITE_LOG(buf, "ERROR");
}

/**
 * ��ӡ�澯log
 */
int CLog::warning(const char *fmt, ...)
{
    char buf[MAX_BUF];
    
    if (_level < _WARNING) 
    {
        return 0;
    }

    WRITE_LOG(buf, "WARNING");
}

/**
 * ��ӡ����log
 */
int CLog::normal(const char *fmt, ...)
{
    char buf[MAX_BUF];
    
    if (_level < _NORMAL) 
    {
        return 0;
    }
    
    WRITE_LOG(buf, "NORMAL");
}

/**
 * ��ӡ����log
 */
int CLog::debug(const char *fmt, ...)
{
    char buf[MAX_BUF];
    
    if (_level < _DEBUG) 
    {
        return 0;
    }
    WRITE_LOG(buf, "DEBUG");
}

/**
 * ֱ��д��־
 */
int CLog::raw(const char *fmt, ...)
{
    char buf[MAX_BUF];
    int iBufLen = 0;
    
    va_list ap;
    va_start(ap, fmt);
    iBufLen += vsnprintf(buf + iBufLen, sizeof(buf) - iBufLen, fmt, ap);
    va_end(ap);
    
    return _write(buf, iBufLen);
}

/**
 * ��¼��־
 */
int CLog::_write(const char *str, int len)
{  
    // ��־����������
    static int __count = 0;

    if (_fd == -1) 
    {
        if (_open() != 0) 
        {
            return -1;
        }
    }

    int ret = write(_fd, str, len);
    if (ret < 0) 
    {
        _close();
        return ret;
    }
    
    // �Ƿ�Ҫִ����־�л�
    if((__count++) >= SHIFT_FREQ)
    {
       _shift();
       __count = 0;
    }
        
    return 0;
}

/**
 * �л���־�ļ�
 */
int CLog::_shift()
{
    struct stat stStat;
    string strNewFile, strOldFile;
    
    // ���´��ļ�
    if(_open() !=0) return -1;
    
    // ���Ե�ǰ��־�ļ���С
    if(fstat(_fd, &stStat) < 0) 
    {
        _close();  
        return -1;
    }

    // ����ǰ�ļ���СС�����ֵ
    if (stStat.st_size < (int)_max_size) 
    {
        return 0;
    }

    // ɾ�����һ����־�ļ�
    strNewFile = _file_name(_max_file - 1);
    if (access(strNewFile.c_str(), F_OK) == 0) 
    {
        if(remove(strNewFile.c_str()) != 0)  return -1;
    }

    // �ۼ��ļ����(�л��ļ���)
    for(int i = _max_file - 2; i >= 0; i--) 
    {
        strOldFile = _file_name(i);
        strNewFile = _file_name(i+1);

        if(access(strOldFile.c_str(), F_OK) == 0) 
        {
            if(rename(strOldFile.c_str(), strNewFile.c_str()) != 0)  break;
        }
    }

    // �ر��ļ�
    _close();  
    
    return 0;
}

/**
 * ȷ����־�ļ���
 * @input:  index  �ļ��������
 */
const char* CLog::_file_name(int index)
{
    static char szFile[256] = {0};
    char szSuffix[128] = {0};

    // �ļ�����׺
    if(index == 0)
    {
        snprintf(szSuffix, sizeof(szSuffix), ".log");
    }
    else
    {
        snprintf(szSuffix, sizeof(szSuffix), "%d.log", index);
    }

    if(_shift_mode == _DATE_MODE)
    {    
        snprintf(szFile, sizeof(szFile), "%s%04d%02d%02d%s", _path.c_str(), _tm_now.tm_year + 1900, _tm_now.tm_mon + 1,  _tm_now.tm_mday, szSuffix);
    }
    else
    {
        snprintf(szFile, sizeof(szFile), "%s%s", _path.c_str(), szSuffix);
    }

    return szFile;
}


map<string, CLog*> CLogHelper::_log_pool;
CLog* CLogHelper::_def_logptr = NULL;

void CLogHelper::addLogPtr(const string &strModule, CLog * pLog, bool bDefault)
{
    if(_log_pool.count(strModule) > 0)
    {
        return; 
    }

    if(bDefault)
    {
       _def_logptr = pLog;
    }

    _log_pool[strModule] = pLog;
}

CLog* CLogHelper::getLogPtr(const string &strModule)
{
    if(unlikely(_log_pool.count(strModule) == 0))
    {
        THROW_EXCEPTION(CException, E_FAIL, "Without Module[%s] Added.", strModule.c_str());
    }

    return _log_pool[strModule];
}

CLog* CLogHelper::getLogPtr()
{
    if(unlikely(_def_logptr == NULL))
    {
        throw CException(E_NULL_POINTER, "Log Null Pointer.");
    }

    return _def_logptr;
}

void CLogHelper::destroy()
{
    for(map<string, CLog*>::iterator it = _log_pool.begin(); it != _log_pool.end(); ++it)
    {
        if(it->second)
        {
            delete it->second;
        }
    }

    _log_pool.clear();
    _def_logptr = NULL;
}



