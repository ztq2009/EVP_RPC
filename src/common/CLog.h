#ifndef _CLOG_HEAD_H_
#define _CLOG_HEAD_H_

#include <stdio.h>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdarg.h>
#include <map>
#include "Handle.h"
#include "Shared.h"


using std::string;
using std::map;

const int MAX_BUF = 1024*400; // ������־��󳤶�
const int SHIFT_FREQ = 128;  // ��־�л�Ƶ��

/**
 * ��־��
 */
class CLog : public Util::Shared
{
public:
    
    /**
     * ��־����
     */
    enum LOG_LEVEL
    {
        _NONE = 1, 
        _ERROR = 2,  
        _WARNING = 3,  
        _NORMAL = 4, 
        _DEBUG = 5 
    };

    /**
     * ��־�л�ģʽ
     */
    enum SHIFT_MODE
    {
        _SIZE_MODE = 1,  // ֻ����С�л����������ۼӺ�׺������
        _DATE_MODE = 2  // �������л�������һ����л�����������
    };
    
public:
    
    /**
     * ���캯��
     */
    CLog(const char* path, unsigned int max_size, unsigned int max_file = 10, SHIFT_MODE = _SIZE_MODE);

    /**
     * ��������
     */
    virtual ~CLog();

    /**
     * ������־����
     */
    inline void setLogLevel(LOG_LEVEL level)    { _level = level; }

    /**
     * ������־ID
     */
    inline void setLogId(const string& strMsg)      { _msg_id = strMsg; }

    /**
     * ��ӡ��־
     */
	int LogInfo(LOG_LEVEL level, const char* fmt, ...);

    /**
     * ��ӡ����log
     */
    int error(const char *fmt, ...);
    
    /**
     * ��ӡ�澯log
     */
    int warning(const char *fmt, ...);
    
    /**
     * ��ӡ����log
     */
    int normal(const char *fmt, ...);

    /**
     * ��ӡ����log
     */
    int debug(const char *fmt, ...);

    /**
     * ֱ��д��־
     */
    int raw(const char *fmt, ...);

protected:
    
    /**
     * ���ļ�
     */
    int _open();

    /**
     * �ر�
     */
    void _close();

    /**
     * �л���־�ļ�
     */
    int _shift();

    /**
     * ��¼��־
     */
    int _write(const char *szLog, int len);

    /**
     * ȷ����־�ļ���
     */
    const char* _file_name(int index=0);

protected:

    /**
     * �л�ģʽ
     */
    SHIFT_MODE _shift_mode;
    
    /**
     * �ļ�ָ��
     */
    int  _fd;

    /**
     * ��־�ļ�·��
     */
    string _path;

    /**
     * ����ļ���С
     */
    unsigned int _max_size;

    /**
     * ��Ϣ��
     */
    string _msg_id;

    /**
     * ��ǰϵͳʱ��
     */
    struct tm _tm_now;

    /**
     * ��Ϣ���к�
     */
    string _log_id;
    
    /**
     * ����ļ�����
     */
    unsigned int _max_file;

    /**
     * ��־����
     */
    LOG_LEVEL _level;
};

typedef Util::Handle<CLog>  CLogPtr;

/**
 * Log����
 */
class CLogHelper
{
public:
    static void addLogPtr(const string &strModule, CLog *pLog, bool bDefault = false);

	static CLog* getLogPtr(const string &strModule);

	static CLog* getLogPtr();

	static void destroy();
	
protected:
	static CLog* _def_logptr;
	static map<string, CLog*> _log_pool;
};

#define LOG_DEBUG(msg, ...)    CLogHelper::getLogPtr()->LogInfo(CLog::_DEBUG,   "[%s:%d]"msg, __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_NORMAL(msg, ...)   CLogHelper::getLogPtr()->LogInfo(CLog::_NORMAL,  "[%s:%d]"msg, __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_WARN(msg, ...)     CLogHelper::getLogPtr()->LogInfo(CLog::_WARNING, "[%s:%d]"msg, __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_ERROR(msg, ...)    CLogHelper::getLogPtr()->LogInfo(CLog::_ERROR,   "[%s:%d]"msg, __FILE__, __LINE__, ##__VA_ARGS__)

#endif

