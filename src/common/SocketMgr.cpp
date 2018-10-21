#include "SocketMgr.h"
#include "Exception.h"
#include "ErrorCode.h"
#include <errno.h>
#include <string.h>
#include <unistd.h>


SocketMgr::SocketMgr()
{
    m_efd = epoll_create(512);
    if(m_efd < 0)
    {
        throw CException(E_CREATE_EPOLL_ERROR, strerror(errno));
    }
}

SocketMgr::~SocketMgr()
{
    if(m_efd > 0)
    {
        close(m_efd);
    }
}

bool SocketMgr::add(int fd, int op, uint64_t data)
{
    uint32_t evop = 0;
    struct epoll_event event;

    if(op & EV_READ)
    {
        evop |= EPOLLIN;
    }

    if(op & EV_WRITE)
    {
        evop |= EPOLLOUT;
    }

    if(op & EV_ET)
    {
        evop |= EPOLLET;
    }

    if(!evop)
    {
        return false;
    }

    event.events = evop;
    event.data.u64 = data;

    if(epoll_ctl(m_efd, EPOLL_CTL_ADD, fd, &event) == -1)
    {
        return false;
    }

    return true;
}

bool SocketMgr::mod(int fd, int op, uint64_t data)
{
    uint32_t evop = 0;
    struct epoll_event event;

    if(op & EV_READ)
    {
        evop |= EPOLLIN;
    }

    if(op & EV_WRITE)
    {
        evop |= EPOLLOUT;
    }

    if(op & EV_ET)
    {
        evop |= EPOLLET;
    }

    if(!evop)
    {
        return false;
    }

    event.events = evop;
    event.data.u64 = data;

    if(epoll_ctl(m_efd, EPOLL_CTL_MOD, fd, &event) == -1)
    {
        return false;
    }

    return true;
}

bool SocketMgr::del(int fd)
{
    if(epoll_ctl(m_efd, EPOLL_CTL_DEL, fd, NULL) == -1)
    {
        return false;
    }

    return true;
}

int SocketMgr::wait(struct epoll_event *pevents, int max_event, int timeout)
{
    return epoll_wait(m_efd, pevents, max_event, timeout);
}

