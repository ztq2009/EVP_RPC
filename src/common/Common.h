#ifndef __COMMON_HEAD_H_
#define __COMMON_HEAD_H_

#include "Exception.h"
#include "ErrorCode.h"
#include "CLog.h"
#include <sys/time.h>


#define   PAGE_SIZE    2048       // page size 4096, ∞¥2048∑÷≈‰

#if !__GLIBC_PREREQ(2, 3)
#define __builtin_expect(x, expected_value) (x)
#endif
#ifndef likely
#define likely(x)  __builtin_expect(!!(x), 1)
#endif
#ifndef unlikely
#define unlikely(x)  __builtin_expect(!!(x), 0)
#endif

enum
{
   E_TIME_SECOND,
   E_TIME_MILLSECOND,
   E_TIME_MICROSECOND,
};

inline int alignPageSize(int size)
{
    int allocSize = (size + PAGE_SIZE - 1) & (unsigned int)~(PAGE_SIZE - 1);

	return allocSize;
}

inline int64_t getTime(int iType = E_TIME_MILLSECOND)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

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

#endif

