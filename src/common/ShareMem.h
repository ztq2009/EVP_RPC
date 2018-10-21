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
 * 共享内存创建公共类
 */
class ShareShm
{
public:
    ShareShm(key_t key, int size);
	
	~ShareShm();

    /**
	 * 获取共享内存首地址
	 */
	void *address();
	
    /**
	 * 获取共享内存ID
	 */
	int shmId();

    /**
	 * 获取共享内存大小
	 */
	int shmSize();

    /**
	 * 销毁共享内存
	 */
	void destroyShm();

    /**
	 * 是否创建者
	 */
	bool isCreator();
	
private:
    /**
	 * 共享内存id
	 */
    int m_iShmid;

    /**
	 * 共享内存大小
	 */
    int m_iSize;
	
    /**
	 * 是否首次创建
	 */
    bool m_bCreator;

    /**
	 * 共享内存首地址
	 */
	void* m_pAddr;
};


/**
 * 共享内存创建公共类
 */
class ShareMap
{
public:
	ShareMap(string fileName, int len, int offset = 0, int prot = (PROT_READ | PROT_WRITE), int flag = MAP_SHARED);

	ShareMap(int fd, int len, int offset = 0, int prot = (PROT_READ | PROT_WRITE), int flag = MAP_SHARED);

	~ShareMap();

    /**
	 * 共享内存大小
	 */
	int mmapSize();

    /**
	 * 共享内存首地址
	 */
	void *address();

    /**
	 * 是否创建者
	 */
	 bool isCreator();
	
private:
    /**
	 * 共享内存大小
	 */
    int m_iSize;
	
    /**
	 * 是否首次创建
	 */
    bool m_bCreator;

    /**
	 * 共享内存首地址
	 */
	void* m_pAddr;
};

}

#endif