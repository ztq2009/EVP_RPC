#ifndef __LOAD_CFG_HEAD_H_
#define __LOAD_CFG_HEAD_H_

#include <string>
#include <vector>
#include "CLog.h"

using std::string;
using std::vector;

/**
 * socket接入相关配置
 */
struct AcceptorEntity
{
    string type;
    string ip;
	unsigned int port;
	string path;
	int tos;
	int oob;

	AcceptorEntity() 
	{
	    path.clear();
		tos = -1;
		oob = 0;
	}
};

/**
 * 共享内存队列相关配置
 */
struct ConnectorEntity
{
    int groupid;
	int sendSize;
	int recvSize;
	int expireTimeout;

	ConnectorEntity()
    {
        sendSize = 600;   // default 600M
		recvSize = 600;   // default 600M
		expireTimeout = 0;
    }
};

struct AcceptorSock
{
    vector<AcceptorEntity> entity;
	int timeout;
	bool udpclose;
	int maxconn;
	int maxCacheSize;

	AcceptorSock()
	{
	    timeout = 0;
		udpclose = true;
		maxconn = 1000000;
		maxCacheSize = 0; // 默认不作限制
	}
};

struct ConnectorShm
{
    vector<ConnectorEntity> entity;
};

/**
 * 日志文件配置
 */
struct LogConf
{
    int level;
	int maxFileSize;
	int maxFileNum;
	string path;
	string prefix;

	LogConf()
	{
	    level = CLog::_NORMAL;
		maxFileSize = 20;   // default 10M
		maxFileNum = 5;
		path = "../log/";
	    prefix = "";
	}
};

/**
 * 加载模块配置
 */
struct Module
{
    string bin;
	string etc;
};

/**
 * 黑名单、白名单配置
 */
struct Iptables
{
    string whiteList;
	string blackList;

	Iptables()
	{
	    whiteList.clear();
		blackList.clear();
	}
};

/**
 * worker共享内存相关配置
 */
struct Worker
{
    int groupID;
	int expireTime;
	int sendSize;
	int recvSize;
};

/**
 * 共享内存、信号量基础key
 */
struct ShareConf
{
    int shmBaseKey;
	int semBaseKey;
};

/**
 * proxy 配置加载类
 */
class ProxyConf
{
public:
    ProxyConf();
	~ProxyConf();
	
	/**
	 * 解析proxy配置文件
	 */
    bool parseConf(const string &fileName);

	/**
	 * 获取日志配置
	 */
	LogConf& getLogConf();

	/**
	 * 获取监听配置信息
	 */
	AcceptorSock& getAcceptorConf();

	/**
	 * 获取队列信息
	 */
	ConnectorShm& getConnectorConf();
	
	/**
	 * 处理模块信息
	 */
	Module& getModuleConf();

	/**
	 * 连接权限配置
	 */
	Iptables& getIptables();

	/**
	 * 共享内存、信号量BaseKey
	 */
	ShareConf& getShareConf();

protected:
	Module        m_module_cfg;
	LogConf       m_log_cfg;
	Iptables      m_iptables_cfg;
	ShareConf     m_share_cfg;
	ConnectorShm  m_connector_cfg;
	AcceptorSock  m_acceptor_cfg;
};

/**
 * worker 配置加载类
 */
class WorkerConf
{
public:
	WorkerConf();
	~WorkerConf();

	/**
	 * 解析worker配置文件
	 */
    bool parseConf(string fileName);

	/**
	 * 获取log配置
	 */	
	LogConf& getLogConf();

	/**
	 * 获取worker配置文件
	 */
    Worker& getWorkerConf();

	/**
	 * 获取模块配置文件
	 */
	Module& getModuleConf();

	/**
	 * 共享内存、信号量BaseKey
	 */
	ShareConf& getShareConf();
	
protected:
	Module       m_module_cfg;
	LogConf      m_log_cfg;
	Worker       m_worker_cfg;
	ShareConf    m_share_cfg;
};


#endif


