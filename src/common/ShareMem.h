/**
  * FileName: ShareMem.h
  * Author: Created by tyreezhang
  * History:
  */
#ifndef __COM_UTIL_SHARE_MEMORY_H__
#define __COM_UTIL_SHARE_MEMORY_H__

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <string>
#include <errno.h>

namespace Util
{

using std::string;

/**
 * �����ڴ洴��������
 */
class ShareShm
{
public:
    ShareShm(key_t key, int size);
	
	~ShareShm();

    /**
	 * ��ȡ�����ڴ��׵�ַ
	 */
	void *address();
	
    /**
	 * ��ȡ�����ڴ�ID
	 */
	int shmId();

    /**
	 * ��ȡ�����ڴ��С
	 */
	int shmSize();

    /**
	 * ���ٹ����ڴ�
	 */
	void destroyShm();

    /**
	 * �Ƿ񴴽���
	 */
	bool isCreator();
	
private:
    /**
	 * �����ڴ�id
	 */
    int m_iShmid;

    /**
	 * �����ڴ��С
	 */
    int m_iSize;
	
    /**
	 * �Ƿ��״δ���
	 */
    bool m_bCreator;

    /**
	 * �����ڴ��׵�ַ
	 */
	void* m_pAddr;
};


/**
 * �����ڴ洴��������
 */
class ShareMap
{
public:
	ShareMap(string fileName, int len, int offset = 0, int prot = (PROT_READ | PROT_WRITE), int flag = MAP_SHARED);

	ShareMap(int fd, int len, int offset = 0, int prot = (PROT_READ | PROT_WRITE), int flag = MAP_SHARED);

	~ShareMap();

    /**
	 * �����ڴ��С
	 */
	int mmapSize();

    /**
	 * �����ڴ��׵�ַ
	 */
	void *address();

    /**
	 * �Ƿ񴴽���
	 */
	 bool isCreator();
	
private:
    /**
	 * �����ڴ��С
	 */
    int m_iSize;
	
    /**
	 * �Ƿ��״δ���
	 */
    bool m_bCreator;

    /**
	 * �����ڴ��׵�ַ
	 */
	void* m_pAddr;
};

}

#endif