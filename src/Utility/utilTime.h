#ifndef OpenKAI_src_Utility_utilTime_H_
#define OpenKAI_src_Utility_utilTime_H_

#include "../Base/platform.h"
#include "../Base/macro.h"
#include "../Base/constant.h"

namespace kai
{

	inline uint64_t getTbootMs(void)
	{
		struct timespec ts;
		clock_gettime(CLOCK_BOOTTIME, &ts);

		return (uint64_t)ts.tv_sec * MSEC_SEC + ts.tv_nsec / NSEC_MSEC;
	}

	inline uint64_t getTns(void)
	{
		struct timespec ts;
		clock_gettime(CLOCK_MONOTONIC, &ts);

		return (uint64_t)ts.tv_sec * NSEC_SEC + ts.tv_nsec;
	}

	inline bool bExpired(uint64_t tStamp, uint64_t tExpire)
	{
		return tStamp <= tExpire;
	}

	inline string tFormat(void)
	{
		time_t timer;
		char buffer[26];
		struct tm *tm_info;
		time(&timer);
		tm_info = localtime(&timer);
		strftime(buffer, 26, "%Y-%m-%d_%H-%M-%S", tm_info);
		puts(buffer);

		string str = buffer;
		return str;
	}

	template <typename T>
	inline T nsec2sec(uint64_t nsec)
	{
		return ((T)nsec) * SEC_NSEC;
	}

	template <typename T>
	inline uint64_t sec2nsec(T sec)
	{
		return (uint64_t)(sec * NSEC_SEC);
	}

}
#endif
