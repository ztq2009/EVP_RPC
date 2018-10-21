/**
  * FileName: ErrorCode.h
  * Author: Created by tyreezhang
  * History:
  */

/*
 * 错误码规则 : 3位模块ID + 3位错误ID
 */
#ifndef _ERRORCODE_HEAD_H_
#define _ERRORCODE_HEAD_H_

enum 
{
    E_OK = 0,
	E_FAIL = 1,
	E_NULL_POINTER = 2, 
	E_INVALIDATE_PARAM,
	E_PARSE_FILE,
	E_NO_MEMORY,
    E_SHM_ERROR,
    E_MMAP_ERROR,
    E_CREATE_EPOLL_ERROR,
    E_NO_CONNECT,
    E_NEED_CLOSE,
    E_NOT_FIND,
    E_READ_AGAIN,
    E_SEND_AGAIN,
    E_SHMMQ_FULL,
    E_SHMMQ_EMPTY,
    E_SHMMQ_CEHCK_FAIL,
    E_SHMMQ_NO_BUFFER,
    E_SHM_LOCK_FAIL,
    E_SEM_LOCK_INIT_FAIL,
    E_NOTIFY_INIT,
};

#endif
  



