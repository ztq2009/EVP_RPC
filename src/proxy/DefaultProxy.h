#ifndef __DEFAULT_PROXY_HEAD_H__
#define __DEFAULT_PROXY_HEAD_H__

#include "Handle.h"
#include "Shared.h"
#include "Loadcfg.h"
#include "SocketAcceptor.h"
#include "ShmAcceptor.h"
#include "IProcess.h"
#include <set>

using namespace std;

enum
{
    E_IP_ACCESS_WHITELIST = 1,
	E_IP_ACCESS_BLACKLIST = 2,
};

/**
 * proxy 模块
 */
class CDefaultProxy : public Util::Shared
{
public:
	CDefaultProxy();

	virtual ~CDefaultProxy();

	/**
	 * proxy开始入口
	 */
	void start(int argc, char *argv[]);

	/**
	 * 客户端连接处理
	 */
	static int connectedActor(uint64_t flow, void* arg1, void *arg2);

	/**
	 * 客户端断开连接处理
	 */
	static int disconnectActor(uint64_t flow, void* arg1, void *arg2);

	/**
	 * 接收数据处理
	 */
	static int recvDataActor(uint64_t flow, void* arg1, void *arg2);

	/**
	 * shmAcceptor接收数据处理
	 */
	static int recvDataActor2(uint64_t flow, void* arg1, void *arg2);

	/**
	 * 数据接收完成
	 */
	static int recvDoneActor(uint64_t flow, void* arg1, void *arg2);

	/**
	 * 发送数据处理
	 */
	static int sendDataActor(uint64_t flow, void* arg1, void *arg2);

	/**
	 * 发送出错处理
	 */
	static int sendErrorActor(uint64_t flow, void* arg1, void *arg2);

	/**
	 * 数据发送完成
	 */
	static int sendDoneActor(uint64_t flow, void* arg1, void *arg2);

	/**
	 * 连接超时处理
	 */
	static int timeOutActor(uint64_t flow, void* arg1, void *arg2);
	 
	/**
	 * 连接过载处理
	 */
	static int overLoadActor(uint64_t flow, void* arg1, void *arg2);

private:

	/**
     * 实际运行
     */
	virtual void run();

	/**
     * 解析配置
     */
    int loadConf(const std::string &strFilePath);

	/**
     * 加载IP列表
     */
    int loadIPList(const std::string &strFileName);

	/**
	 * 加载proxy端so
	 */
    int loadSoProxy(Module &stModule);

	/**
	 * proxy共享内存初始化
	 */
	int initShmAcceptor(ConnectorShm &stShm, ShareConf &stShareCfg);
	
protected:	
	/**
     * 是否停止标识
     */
	bool m_bStop;
	
	/**
     * proxy 配置信息
     */
	sock_conf_t m_stSrvConf;
	
	/**
	 * ip 访问限制模式
	 */
	int m_iIPAccessMod;

	/**
	 * 配置ip集合
	 */
    set<unsigned int>  m_setIP;

	/**
	 * socket通信管理
	 */
	SocketAcceptor  m_sockAcceptor;

	/**
	 * so 动态加载的handle
	 */
    void *m_soHandle;

	/**
	 * proxy协议解析类
	 */
	IProxyProcess *m_soProxyPtr;

	/**
	 * groupID -> proxy队列处理
	 */
	map<int, ShmAcceptor*> m_mapShmAcceptor;
};

typedef Util::Handle<CDefaultProxy>  CDefaultProxyPtr;

#endif



