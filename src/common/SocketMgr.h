/**
  * FileName: SocketMgr.h
  * Author: Created by tyreezhang
  * History:
  */

#ifndef __SOCKET_MANAGER_HEAD_H_
#define __SOCKET_MANAGER_HEAD_H_

#include <stdlib.h>
#include <stdint.h>
#include <sys/epoll.h>

#define   EV_READ      0x00000001
#define   EV_WRITE     0x00000002
#define   EV_ET        0x00000004


class SocketMgr
{
public:
	SocketMgr();
	~SocketMgr();

	bool add(int fd, int op, uint64_t data);
	bool mod(int fd, int op, uint64_t data);

	bool del(int fd);
	int  wait(struct epoll_event *pevents, int max_event, int timeout);

protected:
	int  m_efd;
};


#endif



