#ifndef OPENKAI_SRC_UTILITY_UTIL_H_
#define OPENKAI_SRC_UTILITY_UTIL_H_

#include "../Base/platform.h"
#include "../Base/cv.h"
#include <time.h>
#include <sys/time.h>

using namespace std;

namespace kai
{

inline string uuid(void)
{
	uuid_t uuid;
	uuid_generate(uuid);

	char cUuid[64];
	uuid_unparse(uuid, cUuid);
	string strUuid = cUuid;

	return strUuid;
}

inline int small(int a, int b)
{
	if(a>b)return b;
	return a;
}

inline int big(int a, int b)
{
	if(a>b)return a;
	return b;
}

inline double Hdg(double anyDeg)
{
	while (anyDeg > 360)
		anyDeg -= 360;

	while (anyDeg < 0)
		anyDeg += 360;

	return anyDeg;
}

inline double dHdg(double hFrom, double hTo)
{
	hFrom = Hdg(hFrom);
	hTo = Hdg(hTo);

	double d = hTo - hFrom;

	if (d > 180)
		d -= 360;
	else if (d < -180)
		d += 360;

	return d;
}

inline bool isOverlapped(vInt4* pA, vInt4* pB)
{
	if (pA->z < pB->x || pA->x > pB->z)
		return false;

	if (pA->w < pB->y || pA->y > pB->w)
		return false;

	return true;
}

inline double overlapRatio(vInt4* pA, vInt4* pB)
{
	if (!isOverlapped(pA, pB))
		return 0.0;

	vInt4* pT;
	if (pA->area() < pB->area())
	{
		SWAP(pA, pB, pT);
	}

	//pA > pB
	double w = pA->width();
	if (pB->width() > w)
		w = pB->width();
	double h = pA->height();
	if (pB->height() > h)
		h = pB->height();

	double rX = pB->z - pA->x;
	if (pB->x > pA->x)
		rX = pA->z - pB->x;

	rX /= w;

	double rY = pB->w - pA->y;
	if (pB->y > pA->y)
		rY = pA->w - pB->y;

	rY /= h;

	return rX * rY;
}

inline void rect2vInt4(Rect* pR, vInt4* pV)
{
	pV->x = pR->x;
	pV->y = pR->y;
	pV->z = pR->x + pR->width;
	pV->w = pR->y + pR->height;
}

inline void vInt42rect(kai::vInt4* pV, Rect* pR)
{
	pR->x = pV->x;
	pR->y = pV->y;
	pR->width = pV->z - pV->x;
	pR->height = pV->w - pV->y;
}

inline int constrain(int v, int a, int b)
{
	int min, max;
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

inline double constrain(double v, double a, double b)
{
	double min, max;
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

inline uint64_t get_time_usec()
{
	struct timeval _time_stamp;
	gettimeofday(&_time_stamp, NULL);
	uint64_t time = (uint64_t) _time_stamp.tv_sec * (uint64_t) 1000000
			+ _time_stamp.tv_usec;

	return time;
}

union QBYTE
{
	uint32_t m_uint32;
	uint8_t m_uint8[4];
};

inline uint32_t makeUINT32(uint8_t* pBuf)
{
	QBYTE n;
	n.m_uint8[0] = pBuf[0];
	n.m_uint8[1] = pBuf[1];
	n.m_uint8[2] = pBuf[2];
	n.m_uint8[3] = pBuf[3];
	return n.m_uint32;
}

union OBYTE
{
	uint64_t m_uint32;
	uint8_t m_uint8[8];
	double m_double;
};

inline double makeDouble(uint8_t* pBuf)
{
	OBYTE n;
	n.m_uint8[0] = pBuf[0];
	n.m_uint8[1] = pBuf[1];
	n.m_uint8[2] = pBuf[2];
	n.m_uint8[3] = pBuf[3];
	n.m_uint8[4] = pBuf[4];
	n.m_uint8[5] = pBuf[5];
	n.m_uint8[6] = pBuf[6];
	n.m_uint8[7] = pBuf[7];
	return n.m_double;
}

inline void copyByte(uint32_t v, uint8_t* pBuf)
{
	QBYTE n;
	n.m_uint32 = v;
	pBuf[0] = n.m_uint8[0];
	pBuf[1] = n.m_uint8[1];
	pBuf[2] = n.m_uint8[2];
	pBuf[3] = n.m_uint8[3];
}

inline void copyByte(double v, uint8_t* pBuf)
{
	OBYTE n;
	n.m_double = v;
	pBuf[0] = n.m_uint8[0];
	pBuf[1] = n.m_uint8[1];
	pBuf[2] = n.m_uint8[2];
	pBuf[3] = n.m_uint8[3];
	pBuf[4] = n.m_uint8[4];
	pBuf[5] = n.m_uint8[5];
	pBuf[6] = n.m_uint8[6];
	pBuf[7] = n.m_uint8[7];
}

inline char* ftoa(char* pString, size_t Size, float Value, int FracDigits)
{
	if (pString == NULL)
		return NULL;

	char* pSource = pString;
	if (*(int32_t*) &Value < 0)
	{
		if ((size_t) (pString - pSource) >= Size)
			return NULL;
		*pString++ = '-';
		*(int32_t*) &Value ^= 0x80000000;
	}

	char* pBegin = pString;
	int Integer = (int) Value;

	for (int i = 10; i <= Integer; i *= 10, pString++)
		;

	if ((size_t) (pString - pSource) >= Size)
		return NULL;

	char* pCode = pString++;
	*pCode-- = '0' + (char) (Integer % 10);
	for (int i = Integer / 10; i != 0; i /= 10)
		*pCode-- = '0' + (char) (i % 10);
	if (FracDigits > 0)
	{
		if ((size_t) (pString - pSource) >= Size)
			return NULL;
		*pString++ = '.';
		while (FracDigits-- > 1)
		{
			Value -= Integer;
			Integer = (int) (Value *= 10.0f);
			if ((size_t) (pString - pSource) >= Size)
				return NULL;
			*pString++ = '0' + (char) Integer;
		}
		Integer = (int) ((Value - Integer) * 10.0f + 0.5f);
		if (Integer < 10)
		{
			if ((size_t) (pString - pSource) >= Size)
				return NULL;
			*pString++ = '0' + (char) Integer;
		}
		else
		{
			pCode = pString - 1;
			if ((size_t) (pString - pSource) >= Size)
				return NULL;
			*pString++ = '0';
			do
			{
				if (*pCode == '.')
					pCode--;
				if (*pCode != '9')
				{
					*pCode += 1;
					break;
				}
				else
					*pCode = '0';
				if (pCode == pBegin)
				{
					if ((size_t) (pString - pSource) >= Size)
						return NULL;
					char* pEnd = pString++;
					while (pEnd-- != pCode)
						*(pEnd + 1) = *pEnd;
					*pCode = '1';
				}
			} while (*pCode-- == '0');
		}
	}
	if ((size_t) (pString - pSource) >= Size)
		return NULL;
	*pString = '\0';
	return pSource;
}

inline string f2str(double val)
{
	char buf[128];
	sprintf(buf, "%.3f", val);
	return string(buf);
}

inline string i2str(int val)
{
	char buf[128];
	sprintf(buf, "%d", val);
	return string(buf);
}

inline string li2str(long val)
{
	char buf[128];
	sprintf(buf, "%ld", val);
	return string(buf);
}

inline void trimJson(string* pStr)
{
	unsigned int idx = pStr->rfind('}', pStr->size()) + 1;
	if (pStr->size() > idx)
	{
		pStr->erase(idx);
	}

	idx = pStr->find('{', 0);
	if (idx != string::npos && idx > 0)
	{
		pStr->erase(0, idx - 1);
	}
}

inline vector<string> splitBy(string str, char c)
{
	vector<string> v;
	string buf = "";
	stringstream ss;

	ss << str;
	while (getline(ss, buf, c))
	{
		v.push_back(buf);
	}

	return v;
}

}

#endif

