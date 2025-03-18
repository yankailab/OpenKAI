#ifndef OpenKAI_src_Utility_utilTime_H_
#define OpenKAI_src_Utility_utilTime_H_

#include "../Base/platform.h"
#include "../Base/macro.h"
#include "../Base/constant.h"

namespace kai
{

	inline uint64_t getTbootMs(void)
	{
		// get number of milliseconds since boot
		struct timespec tFromBoot;
		clock_gettime(CLOCK_BOOTTIME, &tFromBoot);

		return tFromBoot.tv_sec * 1000 + tFromBoot.tv_nsec / 1000000;
	}

	inline uint64_t getApproxTbootUs(void)
	{
		// get number of micro sec since boot
		struct timespec tFromBoot;
		clock_gettime(CLOCK_BOOTTIME, &tFromBoot);

		return tFromBoot.tv_sec * SEC_2_USEC + (tFromBoot.tv_nsec >> 10); // / 1000;
	}

	inline uint64_t getTbootUs(void)
	{
		// get number of micro sec since boot
		struct timespec tFromBoot;
		clock_gettime(CLOCK_BOOTTIME, &tFromBoot);

		return tFromBoot.tv_sec * SEC_2_USEC + (tFromBoot.tv_nsec / 1000);
	}

	inline uint64_t getTbootNs(void)
	{
		// get number of nano sec since boot
		struct timespec tFromBoot;
		clock_gettime(CLOCK_BOOTTIME, &tFromBoot);

		return tFromBoot.tv_sec * SEC_2_USEC * 1000 + (tFromBoot.tv_nsec);
	}

	inline bool bExpired(uint64_t tStamp, uint64_t tExpire, uint64_t tNow)
	{
		return (tStamp + tExpire < tNow);
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
	inline T usec2sec(uint64_t usec)
	{
		return ((T)usec) * USEC_2_SEC;
	}

	template <typename T>
	inline uint64_t sec2usec(T sec)
	{
		return (uint64_t)(sec * SEC_2_USEC);
	}

}
#endif
