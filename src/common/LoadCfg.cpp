#include "Loadcfg.h"
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"
#include "rapidxml_print.hpp"
#include "Common.h"
#include <string.h>
#include <stdlib.h>
#include <string>

using std::string;

#define CEHCK_NOT_NULL(pNode, errMsg)   \
if(pNode == NULL)   \
{ \
    throw CException(E_PARSE_FILE, errMsg); \
}

/**
 * 获取节点整形属性值
 **/
static void NODE_ATTR_INT(rapidxml::xml_node<>* pNode, const char *attrName, int &value)
{
    rapidxml::xml_attribute<> *attr = pNode->first_attribute(attrName);
    if(attr)
    {
        char *pValue = attr->value();
        if(pValue && strlen(pValue) > 0)
        {
            value = atoi(pValue);
        }
    }
}

/**
 * 获取节点整形属性值
 **/
static void NODE_ATTR_UINT(rapidxml::xml_node<>* pNode, const char *attrName, unsigned int &value)
{
    rapidxml::xml_attribute<> *attr = pNode->first_attribute(attrName);
    if(attr)
    {
        char *pValue = attr->value();
        if(pValue && strlen(pValue) > 0)
        {
            value = atoi(pValue);
        }
    }
}


/**
 * 获取节点string属性值
 **/
static void NODE_ATTR_STR(rapidxml::xml_node<>* pNode, const char *attrName, string &value)
{
    rapidxml::xml_attribute<> *attr = pNode->first_attribute(attrName);
    if(attr)
    {
        char *pValue = attr->value();
        if(pValue)
        {
            value = pValue;
        }
    }
}

ProxyConf::ProxyConf()
{
}

ProxyConf::~ProxyConf()
{
}

/**
 * 解析proxy配置文件
 */
bool ProxyConf::parseConf(const string &fileName)
{
    try
    {
        rapidxml::xml_document<> doc;
        rapidxml::file<> _file(fileName.c_str());

        doc.parse<0>(_file.data());
        
        rapidxml::xml_node<>* proxy = doc.first_node("proxy");
        CEHCK_NOT_NULL(proxy, "Without Node 'poxy'");

        // 解析acceptor
        rapidxml::xml_node<>* acceptor = proxy->first_node("acceptor");
        CEHCK_NOT_NULL(acceptor, "Without Node 'proxy.acceptor'");

        string strTmp;
        NODE_ATTR_INT(acceptor, "timeout", m_acceptor_cfg.timeout);
        NODE_ATTR_INT(acceptor, "maxconn", m_acceptor_cfg.maxconn);
        NODE_ATTR_INT(acceptor, "maxcachesize", m_acceptor_cfg.maxCacheSize);
        NODE_ATTR_STR(acceptor, "udpclose", strTmp);

        m_acceptor_cfg.udpclose = (strTmp == "true");

        rapidxml::xml_node<>* entry = acceptor->first_node("entry");
        while(entry)
        {
            struct AcceptorEntity entity;

            NODE_ATTR_STR(entry, "type", entity.type);
            NODE_ATTR_STR(entry, "if", entity.ip);
            NODE_ATTR_STR(entry, "path", entity.path);
            NODE_ATTR_UINT(entry, "port", entity.port);
            NODE_ATTR_INT(entry, "tos", entity.tos);
            NODE_ATTR_INT(entry, "oob", entity.oob);

            m_acceptor_cfg.entity.push_back(entity);

            entry = entry->next_sibling("entry");
        }

        //解析connector
        rapidxml::xml_node<>* connector =  proxy->first_node("connector");
        CEHCK_NOT_NULL(connector, "Without Node 'poxy.connector'");

        entry = connector->first_node("entry");
        while(entry)
        {
            struct ConnectorEntity entity;

            NODE_ATTR_INT(entry, "groupid", entity.groupid);
            NODE_ATTR_INT(entry, "send_size", entity.sendSize);
            NODE_ATTR_INT(entry, "recv_size", entity.recvSize);
            NODE_ATTR_INT(entry, "expire_time", entity.expireTimeout);

            // 设置队列大小以M为单位
            entity.sendSize *= 1024 * 1024;
            entity.recvSize *= 1024 * 1024;
            
            m_connector_cfg.entity.push_back(entity);

            entry = entry->next_sibling("entry");
        }

        //解析iptable
        rapidxml::xml_node<>* iptable =  proxy->first_node("iptable");
        CEHCK_NOT_NULL(iptable, "Without Node 'poxy.iptable'");

        NODE_ATTR_STR(iptable, "whitelist", m_iptables_cfg.whiteList);
        NODE_ATTR_STR(iptable, "blacklist", m_iptables_cfg.blackList);

        // 解析log
        rapidxml::xml_node<>* log =  proxy->first_node("log");
        CEHCK_NOT_NULL(log, "Without Node 'poxy.log'");

        NODE_ATTR_INT(log, "level", m_log_cfg.level);
        NODE_ATTR_INT(log, "file_size", m_log_cfg.maxFileSize);
        NODE_ATTR_INT(log, "file_num", m_log_cfg.maxFileNum);
        NODE_ATTR_STR(log, "path", m_log_cfg.path);
        NODE_ATTR_STR(log, "prefix", m_log_cfg.prefix);

        // 解析module
        rapidxml::xml_node<>* module =  proxy->first_node("module");
        CEHCK_NOT_NULL(log, "Without Node 'poxy.module'");

        NODE_ATTR_STR(module, "bin", m_module_cfg.bin);
        NODE_ATTR_STR(module, "etc", m_module_cfg.etc);

        //解析shm
        rapidxml::xml_node<>* shm = proxy->first_node("shm");
        CEHCK_NOT_NULL(shm, "Without Node 'poxy.shm'");

        NODE_ATTR_INT(shm, "shmbase", m_share_cfg.shmBaseKey);
        NODE_ATTR_INT(shm, "sembase", m_share_cfg.semBaseKey);

        return true;
        
    }
    catch(rapidxml::parse_error &e)
    {
        printf("proxy parse file[%s] failed, errMsg:%s\r\n", fileName.c_str(), e.what());
    }
    catch(CException &e)
    {
        printf("proxy parse file[%s] failed, errMsg:%s\r\n", fileName.c_str(), e.what());
    }

    return false;
}

/**
 * 获取日志配置
 */
LogConf& ProxyConf::getLogConf()
{
    return m_log_cfg;
}

/**
 * 获取监听配置信息
 */
AcceptorSock& ProxyConf::getAcceptorConf()
{
    return m_acceptor_cfg;
}

/**
 * 获取队列信息
 */
ConnectorShm& ProxyConf::getConnectorConf()
{
    return m_connector_cfg;
}

/**
 * 处理模块信息
 */
Module& ProxyConf::getModuleConf()
{
    return m_module_cfg;
}

/**
 * 连接权限配置
 */
Iptables& ProxyConf::getIptables()
{
    return m_iptables_cfg;
}


/**
 * 共享内存、信号量BaseKey
 */
ShareConf& ProxyConf::getShareConf()
{
    return m_share_cfg;
}


WorkerConf::WorkerConf()
{
}

WorkerConf::~WorkerConf()
{
}

/**
 * 解析worker配置文件
 */
bool WorkerConf::parseConf(string fileName)
{
    try
    {
        rapidxml::xml_document<> doc;
        rapidxml::file<> _file(fileName.c_str());

        doc.parse<0>(_file.data());

        rapidxml::xml_node<>* worker = doc.first_node("worker");
        CEHCK_NOT_NULL(worker, "Without Node 'worker'");

        // 解析entry
        rapidxml::xml_node<>* entry = worker->first_node("entry");
        CEHCK_NOT_NULL(entry, "Without Node 'proxy.entry'");

        NODE_ATTR_INT(entry, "groupid", m_worker_cfg.groupID);
        NODE_ATTR_INT(entry, "send_size", m_worker_cfg.sendSize);
        NODE_ATTR_INT(entry, "recv_size", m_worker_cfg.recvSize);
        NODE_ATTR_INT(entry, "expire_time", m_worker_cfg.expireTime);

        // 设置大小以M为
        m_worker_cfg.sendSize *= 1024 * 1024;
        m_worker_cfg.recvSize *= 1024 * 1024;

        // 解析log
        rapidxml::xml_node<>* log =  worker->first_node("log");
        CEHCK_NOT_NULL(log, "Without Node 'worker.log'");

        NODE_ATTR_INT(log, "level", m_log_cfg.level);
        NODE_ATTR_INT(log, "file_size", m_log_cfg.maxFileSize);
        NODE_ATTR_INT(log, "file_num", m_log_cfg.maxFileNum);
        NODE_ATTR_STR(log, "path", m_log_cfg.path);
        NODE_ATTR_STR(log, "prefix", m_log_cfg.prefix);

        // 解析module
        rapidxml::xml_node<>* module =  worker->first_node("module");
        CEHCK_NOT_NULL(log, "Without Node 'worker.module'");

        NODE_ATTR_STR(module, "bin", m_module_cfg.bin);
        NODE_ATTR_STR(module, "etc", m_module_cfg.etc);

        //解析shm
        rapidxml::xml_node<>* shm = worker->first_node("shm");
        CEHCK_NOT_NULL(shm, "Without Node 'worker.shm'");

        NODE_ATTR_INT(shm, "shmbase", m_share_cfg.shmBaseKey);
        NODE_ATTR_INT(shm, "sembase", m_share_cfg.semBaseKey);

        return true;
    }
    catch(rapidxml::parse_error &e)
    {
        printf("worker parse file[%s] failed, errMsg:%s\r\n", fileName.c_str(), e.what());
        return false;
    }
    catch(CException &e)
    {
        printf("worker parse file[%s] failed, errMsg:%s\r\n", fileName.c_str(), e.what());
        return false;
    }

    return false;
}

/**
 * 获取log配置
 */	
LogConf& WorkerConf::getLogConf()
{
    return m_log_cfg;
}

/**
 * 获取worker配置文件
 */
Worker& WorkerConf::getWorkerConf()
{
    return m_worker_cfg;
}

/**
 * 获取模块配置文件
 */
Module& WorkerConf::getModuleConf()
{
    return m_module_cfg;
}

/**
 * 共享内存、信号量BaseKey
 */
ShareConf& WorkerConf::getShareConf()
{
    return m_share_cfg;
}


