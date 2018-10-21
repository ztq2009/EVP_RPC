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

const int MAX_BUF = 1024*400; // 单行日志最大长度
const int SHIFT_FREQ = 128;  // 日志切换频度

/**
 * 日志类
 */
class CLog : public Util::Shared
{
public:
    
    /**
     * 日志级别
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
     * 日志切换模式
     */
    enum SHIFT_MODE
    {
        _SIZE_MODE = 1,  // 只按大小切换，超出后累加后缀索引号
        _DATE_MODE = 2  // 按日期切换，到下一天后切换日期索引号
    };
    
public:
    
    /**
     * 构造函数
     */
    CLog(const char* path, unsigned int max_size, unsigned int max_file = 10, SHIFT_MODE = _SIZE_MODE);

    /**
     * 析构函数
     */
    virtual ~CLog();

    /**
     * 设置日志级别
     */
    inline void setLogLevel(LOG_LEVEL level)    { _level = level; }

    /**
     * 设置日志ID
     */
    inline void setLogId(const string& strMsg)      { _msg_id = strMsg; }

    /**
     * 打印日志
     */
	int LogInfo(LOG_LEVEL level, const char* fmt, ...);

    /**
     * 打印错误log
     */
    int error(const char *fmt, ...);
    
    /**
     * 打印告警log
     */
    int warning(const char *fmt, ...);
    
    /**
     * 打印正常log
     */
    int normal(const char *fmt, ...);

    /**
     * 打印调试log
     */
    int debug(const char *fmt, ...);

    /**
     * 直接写日志
     */
    int raw(const char *fmt, ...);

protected:
    
    /**
     * 打开文件
     */
    int _open();

    /**
     * 关闭
     */
    void _close();

    /**
     * 切换日志文件
     */
    int _shift();

    /**
     * 记录日志
     */
    int _write(const char *szLog, int len);

    /**
     * 确定日志文件名
     */
    const char* _file_name(int index=0);

protected:

    /**
     * 切换模式
     */
    SHIFT_MODE _shift_mode;
    
    /**
     * 文件指针
     */
    int  _fd;

    /**
     * 日志文件路径
     */
    string _path;

    /**
     * 最大文件大小
     */
    unsigned int _max_size;

    /**
     * 消息号
     */
    string _msg_id;

    /**
     * 当前系统时间
     */
    struct tm _tm_now;

    /**
     * 消息序列号
     */
    string _log_id;
    
    /**
     * 最大文件个数
     */
    unsigned int _max_file;

    /**
     * 日志级别
     */
    LOG_LEVEL _level;
};

typedef Util::Handle<CLog>  CLogPtr;

/**
 * Log管理
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

