#include "Notify.h"
#include "Common.h"
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/**
 * fifo���д���
 */
int Util::Notify::notify_init(int key)
{
    char szBuf[1024] = {0};
    
    snprintf(szBuf, sizeof(szBuf), "/tmp/.notify_%d", key);

    if(mkfifo(szBuf, 0666) < 0)
    {
        if(errno != EEXIST)
        {
            LOG_DEBUG("create fifo '%s' failed.", szBuf);

            throw CException(E_NOTIFY_INIT, strerror(errno));
        }
    }

    int fd;

    if((fd = open(szBuf, O_RDWR | O_NONBLOCK, 0666)) < 0)
    {
        LOG_DEBUG("open fifo '%s' failed.", szBuf);

        throw CException(E_NOTIFY_INIT, strerror(errno));
    }
   
    return fd;
}

/**
 * fifoд
 */
int Util::Notify::notify_send(int fd)
{
    return write(fd, "Y", 1);
}

/**
 * fifo��
 */
int Util::Notify::notify_recv(int fd)
{
    static char szBuf[128];

    return read(fd, szBuf, sizeof(szBuf));
}


