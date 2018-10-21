#ifndef __COM_UTIL_TIME_ELAPSE_H__
#define __COM_UTIL_TIME_ELAPSE_H__

#include <sys/time.h>
#include <stdint.h>
#include <stdio.h>
#include "Common.h"


class CTimeElapse
{
public:
	CTimeElapse();

	~CTimeElapse();

	/**
	 * 重置开始时间
	 */
	void reset();
	
	/**
	 * 计算耗时
	 */
	int32_t costTime(int iType = E_TIME_MILLSECOND);

protected:
	/**
	 * 开始时间
	 */
	struct timeval  m_tStart;

	/**
	 * 结束时间
	 */
	struct timeval  m_tEnd;
};

#endif

