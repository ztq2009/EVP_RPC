#ifndef __UTIL_NOTIFY_HEAD_H_
#define __UTIL_NOTIFY_HEAD_H_

#include <stdio.h>

namespace Util
{
/**
 * fifo队列创建、读/写
 **/
class Notify
{
public:
	/**
     * fifo队列创建
     */
	static int notify_init(int key);

	/**
     * fifo写
     */
	static int notify_send(int fd);
	
	/**
     * fifo读
     */
	static int notify_recv(int fd);
};

}


#endif

