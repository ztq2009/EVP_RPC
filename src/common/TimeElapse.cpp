#include "TimeElapse.h"


CTimeElapse::CTimeElapse()
{
    reset();
}

CTimeElapse::~CTimeElapse()
{
}

/**
 * ���ÿ�ʼʱ��
 */
void CTimeElapse::reset()
{
    gettimeofday(&m_tStart, NULL);
}

/**
 * �����ʱ
 */
int32_t CTimeElapse::costTime(int iType)
{
    struct timeval tv;
    
    gettimeofday(&m_tEnd, NULL);
    timersub(&m_tEnd, &m_tStart, &tv);
    
    if(iType == E_TIME_SECOND)
    {
        return tv.tv_sec;
    }
    else if(iType == E_TIME_MILLSECOND)
    {
        return tv.tv_sec * 1000 + tv.tv_usec / 1000;
    }

    return tv.tv_sec * 1000000 + tv.tv_usec;
}
