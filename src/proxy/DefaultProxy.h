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
 * proxy ģ��
 */
class CDefaultProxy : public Util::Shared
{
public:
	CDefaultProxy();

	virtual ~CDefaultProxy();

	/**
	 * proxy��ʼ���
	 */
	void start(int argc, char *argv[]);

	/**
	 * �ͻ������Ӵ���
	 */
	static int connectedActor(uint64_t flow, void* arg1, void *arg2);

	/**
	 * �ͻ��˶Ͽ����Ӵ���
	 */
	static int disconnectActor(uint64_t flow, void* arg1, void *arg2);

	/**
	 * �������ݴ���
	 */
	static int recvDataActor(uint64_t flow, void* arg1, void *arg2);

	/**
	 * shmAcceptor�������ݴ���
	 */
	static int recvDataActor2(uint64_t flow, void* arg1, void *arg2);

	/**
	 * ���ݽ������
	 */
	static int recvDoneActor(uint64_t flow, void* arg1, void *arg2);

	/**
	 * �������ݴ���
	 */
	static int sendDataActor(uint64_t flow, void* arg1, void *arg2);

	/**
	 * ���ͳ�����
	 */
	static int sendErrorActor(uint64_t flow, void* arg1, void *arg2);

	/**
	 * ���ݷ������
	 */
	static int sendDoneActor(uint64_t flow, void* arg1, void *arg2);

	/**
	 * ���ӳ�ʱ����
	 */
	static int timeOutActor(uint64_t flow, void* arg1, void *arg2);
	 
	/**
	 * ���ӹ��ش���
	 */
	static int overLoadActor(uint64_t flow, void* arg1, void *arg2);

private:

	/**
     * ʵ������
     */
	virtual void run();

	/**
     * ��������
     */
    int loadConf(const std::string &strFilePath);

	/**
     * ����IP�б�
     */
    int loadIPList(const std::string &strFileName);

	/**
	 * ����proxy��so
	 */
    int loadSoProxy(Module &stModule);

	/**
	 * proxy�����ڴ��ʼ��
	 */
	int initShmAcceptor(ConnectorShm &stShm, ShareConf &stShareCfg);
	
protected:	
	/**
     * �Ƿ�ֹͣ��ʶ
     */
	bool m_bStop;
	
	/**
     * proxy ������Ϣ
     */
	sock_conf_t m_stSrvConf;
	
	/**
	 * ip ��������ģʽ
	 */
	int m_iIPAccessMod;

	/**
	 * ����ip����
	 */
    set<unsigned int>  m_setIP;

	/**
	 * socketͨ�Ź���
	 */
	SocketAcceptor  m_sockAcceptor;

	/**
	 * so ��̬���ص�handle
	 */
    void *m_soHandle;

	/**
	 * proxyЭ�������
	 */
	IProxyProcess *m_soProxyPtr;

	/**
	 * groupID -> proxy���д���
	 */
	map<int, ShmAcceptor*> m_mapShmAcceptor;
};

typedef Util::Handle<CDefaultProxy>  CDefaultProxyPtr;

#endif



