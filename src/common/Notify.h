#ifndef __UTIL_NOTIFY_HEAD_H_
#define __UTIL_NOTIFY_HEAD_H_

#include <stdio.h>

namespace Util
{
/**
 * fifo���д�������/д
 **/
class Notify
{
public:
	/**
     * fifo���д���
     */
	static int notify_init(int key);

	/**
     * fifoд
     */
	static int notify_send(int fd);
	
	/**
     * fifo��
     */
	static int notify_recv(int fd);
};

}


#endif

