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
	 * ���ÿ�ʼʱ��
	 */
	void reset();
	
	/**
	 * �����ʱ
	 */
	int32_t costTime(int iType = E_TIME_MILLSECOND);

protected:
	/**
	 * ��ʼʱ��
	 */
	struct timeval  m_tStart;

	/**
	 * ����ʱ��
	 */
	struct timeval  m_tEnd;
};

#endif

