#ifndef OpenKAI_src_Utility_util_H_
#define OpenKAI_src_Utility_util_H_

#include "../Base/platform.h"
#include "../Base/macro.h"
#include "../Base/constant.h"

namespace kai
{

#define HIGH16(x) ((x >> 16) & 0x0000ffff)
#define LOW16(x) (x & 0x0000ffff)
#define MAKE32(x, y) (((((uint32_t)x) << 16) & 0xffff0000) | y)
#define EQUAL(x, y, z) ((abs(x - y) <= z) ? true : false)

	template <typename T>
	inline T map(T x, T inMin, T inMax, T outMin, T outMax)
	{
		return (x - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
	}

	inline double dEarth(double lat1, double lon1, double lat2, double lon2)
	{
		double p = 0.017453292519943295; // Math.PI / 180
		double a = 0.5 - cos((lat2 - lat1) * p) * 0.5 +
				   cos(lat1 * p) * cos(lat2 * p) *
					   (1 - cos((lon2 - lon1) * p)) * 0.5;

		return 12742000.0 * asin(sqrt(a)); // 2 * R; R = 6371000 m
	}

	inline double NormRand(void)
	{
		return ((double)rand()) / ((double)RAND_MAX);
	}

	inline string uuid(void)
	{
		uuid_t uuid;
		uuid_generate(uuid);

		char cUuid[64];
		uuid_unparse(uuid, cUuid);
		string strUuid = cUuid;

		return strUuid;
	}

	template <typename T>
	inline T small(T a, T b)
	{
		if (a > b)
			return b;
		return a;
	}

	template <typename T>
	inline T big(T a, T b)
	{
		if (a > b)
			return a;
		return b;
	}

	template <typename T>
	inline T Hdg(T deg)
	{
		while (deg >= 360)
			deg -= 360;

		while (deg < 0)
			deg += 360;

		return deg;
	}

	template <typename T>
	inline T dHdg(T hFrom, T hTo)
	{
		hFrom = Hdg(hFrom);
		hTo = Hdg(hTo);

		T d = hTo - hFrom;

		if (d > 180)
			d -= 360;
		else if (d < -180)
			d += 360;

		return d;
	}

	inline double bearing(double lat1, double lon1, double lat2, double lon2)
	{
		lat1 *= DEG_2_RAD;
		lon1 *= DEG_2_RAD;
		lat2 *= DEG_2_RAD;
		lon2 *= DEG_2_RAD;

		double dLon = lon2 - lon1;
		double y = sin(dLon) * cos(lat2);
		double x = cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(dLon);

		double bearing = atan2(y, x);
		bearing *= RAD_2_DEG;
		bearing = Hdg(bearing);
		//    bearing = 360 - bearing; // count degrees counter-clockwise - remove to make clockwise

		return bearing;
	}

	template <typename T>
	inline T bbExpand(T &bb, float k)
	{
		float s = abs(1.0 - k) * 0.5;
		float dW = bb.width() * s;
		float dH = bb.height() * s;

		T B = bb;

		if (k > 1.0)
		{
			B.x -= dW;
			B.y -= dH;
			B.z += dW;
			B.w += dH;
		}
		else
		{
			B.x += dW;
			B.y += dH;
			B.z -= dW;
			B.w -= dH;
		}

		return B;
	}

	template <typename T>
	inline T bbScale(T bb, float kx, float ky)
	{
		T v;
		v.x = bb.x * kx;
		v.y = bb.y * ky;
		v.z = bb.z * kx;
		v.w = bb.w * ky;
		return v;
	}

	template <typename T>
	inline bool bOverlap(T &pA, T &pB)
	{
		IF_F(pA.z < pB.x || pA.x > pB.z);
		IF_F(pA.w < pB.y || pA.y > pB.w);

		return true;
	}

	template <typename T>
	inline T constrain(T v, T a, T b)
	{
		T min, max;
		if (a < b)
		{
			min = a;
			max = b;
		}
		else
		{
			max = a;
			min = b;
		}

		if (v > max)
			v = max;
		else if (v < min)
			v = min;

		return v;
	}

	template <typename T>
	inline T bOdd(T v)
	{
		return v & 1;
	}

	template <typename T>
	inline T bEven(T v)
	{
		return 1 - (v & 1);
	}

	inline void pack_int16(void *pB, int16_t v, bool bOrder = true)
	{
		if (bOrder)
			v = (int16_t)htons((uint16_t)v);

		memcpy(pB, &v, sizeof(int16_t));
	}

	inline int16_t unpack_int16(const void *pB, bool bOrder = true)
	{
		int16_t v = 0;
		memcpy(&v, pB, sizeof(int16_t));

		if (bOrder)
			v = (int16_t)ntohs((uint16_t)v);

		return v;
	}

	inline void pack_uint16(void *pB, uint16_t v, bool bOrder = true)
	{
		if (bOrder)
			v = htons(v);

		memcpy(pB, &v, sizeof(uint16_t));
	}

	inline uint16_t unpack_uint16(const void *pB, bool bOrder = true)
	{
		uint16_t v = 0;
		memcpy(&v, pB, sizeof(uint16_t));

		if (bOrder)
			v = ntohs(v);

		return v;
	}

	inline void pack_int32(void *pB, int32_t v, bool bOrder = true)
	{
		if (bOrder)
			v = (int32_t)htonl((uint32_t)v);

		memcpy(pB, &v, sizeof(int32_t));
	}

	inline int32_t unpack_int32(const void *pB, bool bOrder = true)
	{
		int32_t v = 0;
		memcpy(&v, pB, sizeof(int32_t));

		if (bOrder)
			v = (int32_t)ntohl((uint32_t)v);

		return v;
	}

	inline void pack_uint32(void *pB, uint32_t v, bool bOrder = true)
	{
		if (bOrder)
			v = htonl(v);

		memcpy(pB, &v, sizeof(uint32_t));
	}

	inline uint32_t unpack_uint32(const void *pB, bool bOrder = true)
	{
		uint32_t v = 0;
		memcpy(&v, pB, sizeof(uint32_t));

		if (bOrder)
			v = ntohl(v);

		return v;
	}

	inline void f2b(uint8_t *pB, float f)
	{
		uint8_t *pF = (uint8_t *)&f;

		for (uint8_t i = 0; i < sizeof(float); i++)
		{
			*pB = *pF;
			pF++;
			pB++;
		}
	}

	inline uint16_t swap_uint16(uint16_t val)
	{
		return (val << 8) | (val >> 8);
	}

	inline int16_t swap_int16(int16_t val)
	{
		return (val << 8) | ((val >> 8) & 0xFF);
	}

	inline uint32_t swap_uint32(uint32_t val)
	{
		val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
		return (val << 16) | (val >> 16);
	}

	inline int32_t swap_int32(int32_t val)
	{
		val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
		return (val << 16) | ((val >> 16) & 0xFFFF);
	}

	inline int64_t swap_int64(int64_t val)
	{
		val = ((val << 8) & 0xFF00FF00FF00FF00ULL) | ((val >> 8) & 0x00FF00FF00FF00FFULL);
		val = ((val << 16) & 0xFFFF0000FFFF0000ULL) | ((val >> 16) & 0x0000FFFF0000FFFFULL);
		return (val << 32) | ((val >> 32) & 0xFFFFFFFFULL);
	}

	inline uint64_t swap_uint64(uint64_t val)
	{
		val = ((val << 8) & 0xFF00FF00FF00FF00ULL) | ((val >> 8) & 0x00FF00FF00FF00FFULL);
		val = ((val << 16) & 0xFFFF0000FFFF0000ULL) | ((val >> 16) & 0x0000FFFF0000FFFFULL);
		return (val << 32) | (val >> 32);
	}

	inline uint8_t checksum(uint8_t *pB, int N)
	{
		NULL_F(pB);

		uint32_t s = 0;
		for (int i = 0; i < N; i++)
		{
			s += pB[i];
		}
		s &= 0xFF;
		s = 0xFF - s;
		return s;
	}

	inline uint8_t CRC8(const uint8_t *pD, int nB)
	{
		char crc = 0x00;
		char extract;
		char sum;
		for (int i = 0; i < nB; i++)
		{
			extract = pD[i];
			for (char tempI = 8; tempI; tempI--)
			{
				sum = (crc ^ extract) & 0x01;
				crc >>= 1;
				if (sum)
					crc ^= 0x8C;
				extract >>= 1;
			}
		}

		return crc;
	}

	inline uint8_t crc8_MAXIM(uint8_t *pD, uint8_t nB)
	{
		uint8_t crc = 0x00;

		while (nB--)
		{
			crc ^= *pD++;
			for (int i = 0; i < 8; i++)
			{
				if (crc & 0x01)
				{
					crc = (crc >> 1) ^ 0x8c;
				}
				else
					crc >>= 1;
			}
		}

		return crc;
	}

#define POP2_READ 0
#define POP2_WRITE 1

	inline pid_t popen2(const char *command, int *infp, int *outfp)
	{
		int p_stdin[2], p_stdout[2];
		pid_t pid;

		if (pipe(p_stdin) != 0 || pipe(p_stdout) != 0)
			return -1;

		pid = fork();

		if (pid < 0)
			return pid;
		else if (pid == 0)
		{
			close(p_stdin[POP2_WRITE]);
			dup2(p_stdin[POP2_READ], POP2_READ);
			close(p_stdout[POP2_READ]);
			dup2(p_stdout[POP2_WRITE], POP2_WRITE);

			execl("/bin/sh", "sh", "-c", command, NULL);
			perror("execl");
			exit(1);
		}

		if (infp == NULL)
			close(p_stdin[POP2_WRITE]);
		else
			*infp = p_stdin[POP2_WRITE];

		if (outfp == NULL)
			close(p_stdout[POP2_READ]);
		else
			*outfp = p_stdout[POP2_READ];

		return pid;
	}


}
#endif
