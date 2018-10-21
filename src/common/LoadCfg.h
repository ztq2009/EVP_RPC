#ifndef __LOAD_CFG_HEAD_H_
#define __LOAD_CFG_HEAD_H_

#include <string>
#include <vector>
#include "CLog.h"

using std::string;
using std::vector;

/**
 * socket�����������
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
 * �����ڴ�����������
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
		maxCacheSize = 0; // Ĭ�ϲ�������
	}
};

struct ConnectorShm
{
    vector<ConnectorEntity> entity;
};

/**
 * ��־�ļ�����
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
 * ����ģ������
 */
struct Module
{
    string bin;
	string etc;
};

/**
 * ������������������
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
 * worker�����ڴ��������
 */
struct Worker
{
    int groupID;
	int expireTime;
	int sendSize;
	int recvSize;
};

/**
 * �����ڴ桢�ź�������key
 */
struct ShareConf
{
    int shmBaseKey;
	int semBaseKey;
};

/**
 * proxy ���ü�����
 */
class ProxyConf
{
public:
    ProxyConf();
	~ProxyConf();
	
	/**
	 * ����proxy�����ļ�
	 */
    bool parseConf(const string &fileName);

	/**
	 * ��ȡ��־����
	 */
	LogConf& getLogConf();

	/**
	 * ��ȡ����������Ϣ
	 */
	AcceptorSock& getAcceptorConf();

	/**
	 * ��ȡ������Ϣ
	 */
	ConnectorShm& getConnectorConf();
	
	/**
	 * ����ģ����Ϣ
	 */
	Module& getModuleConf();

	/**
	 * ����Ȩ������
	 */
	Iptables& getIptables();

	/**
	 * �����ڴ桢�ź���BaseKey
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
 * worker ���ü�����
 */
class WorkerConf
{
public:
	WorkerConf();
	~WorkerConf();

	/**
	 * ����worker�����ļ�
	 */
    bool parseConf(string fileName);

	/**
	 * ��ȡlog����
	 */	
	LogConf& getLogConf();

	/**
	 * ��ȡworker�����ļ�
	 */
    Worker& getWorkerConf();

	/**
	 * ��ȡģ�������ļ�
	 */
	Module& getModuleConf();

	/**
	 * �����ڴ桢�ź���BaseKey
	 */
	ShareConf& getShareConf();
	
protected:
	Module       m_module_cfg;
	LogConf      m_log_cfg;
	Worker       m_worker_cfg;
	ShareConf    m_share_cfg;
};


#endif


