#ifndef OpenKAI_src_Utility_util_H_
#define OpenKAI_src_Utility_util_H_

#ifdef USE_OPENCV
#include "../Base/cv.h"
#endif

#include "../Base/platform.h"

namespace kai
{

#define HIGH16(x) ((x >> 16) & 0x0000ffff)
#define LOW16(x) (x & 0x0000ffff)
#define MAKE32(x,y) (((((uint32_t)x)<<16)&0xffff0000) | y)
#define EAQ(x,y,z) ((abs(x-y)<z)?true:false)

template <typename T> inline T map(T x, T inMin, T inMax, T outMin, T outMax)
{
  return (x - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
}

inline string deleteNonASCII(const char* pStr)
{
	string asc = "";
	if(!pStr)return asc;

	int i=0;
	while(pStr[i]!=0)
	{
		char c = pStr[i++];
		IF_CONT(c<0);
		IF_CONT(c>=128);
		asc += c;
	}
	return asc;
}

inline string deleteNonNumber(const char* pStr)
{
	string asc = "";
	if(!pStr)return asc;

	int i=0;
	while(pStr[i]!=0)
	{
		char c = pStr[i++];
		IF_CONT(c<48);
		IF_CONT(c>57);
		asc += c;
	}
	return asc;
}

inline double dEarth(double lat1, double lon1, double lat2, double lon2)
{
  double p = 0.017453292519943295;    // Math.PI / 180
  double a = 0.5 - cos((lat2 - lat1) * p)*0.5 +
          cos(lat1 * p) * cos(lat2 * p) *
          (1 - cos((lon2 - lon1) * p))*0.5;

  return 12742000.0 * asin(sqrt(a)); // 2 * R; R = 6371000 m
}

inline string tFormat(void)
{
	time_t timer;
	char buffer[26];
	struct tm* tm_info;
	time(&timer);
	tm_info = localtime(&timer);
	strftime(buffer, 26, "%Y-%m-%d_%H-%M-%S", tm_info);
	puts(buffer);

	string str = buffer;
	return str;
}

inline string getFileDir(string file)
{
	return file.erase(file.find_last_of('/') + 1, string::npos);
}

inline string checkDirName(string& dir)
{
	if (dir.at(dir.length() - 1) != '/')
		dir.push_back('/');

	return dir;
}

inline string getFileExt(string& file)
{
	size_t extPos = file.find_last_of(".");
	if(extPos == std::string::npos)return "";
	return file.substr(extPos);
}

inline uint32_t getTimeBootMs()
{
	// get number of milliseconds since boot
	struct timespec tFromBoot;
	clock_gettime(CLOCK_BOOTTIME, &tFromBoot);

	return tFromBoot.tv_sec * 1000 + tFromBoot.tv_nsec / 1000000;
}

inline uint64_t getTimeUsec()
{
	struct timeval tStamp;
	gettimeofday(&tStamp, NULL);
	uint64_t time = (uint64_t) tStamp.tv_sec * (uint64_t) 1000000
			+ tStamp.tv_usec;

	return time;
}

inline double NormRand(void)
{
	return ((double) rand()) / ((double) RAND_MAX);
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

template <typename T> inline T small(T a, T b)
{
	if (a > b)
		return b;
	return a;
}

template <typename T> inline T big(T a, T b)
{
	if (a > b)
		return a;
	return b;
}

template <typename T> inline T Hdg(T deg)
{
	while (deg >= 360)
		deg -= 360;

	while (deg < 0)
		deg += 360;

	return deg;
}

template <typename T> inline T dHdg(T hFrom, T hTo)
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

template <typename T> inline T bbExpand(T& bb, float k)
{
	float s = abs(1.0 - k) * 0.5;
	float dW = bb.width() * s;
	float dH = bb.height() * s;

	T B = bb;

	if(k > 1.0)
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

template <typename T> inline T bbScale(T bb, float kx, float ky)
{
	T v;
	v.x = bb.x * kx;
	v.y = bb.y * ky;
	v.z = bb.z * kx;
	v.w = bb.w * ky;
	return v;
}

template <typename T> inline bool bOverlap(T& pA, T& pB)
{
	IF_F(pA.z < pB.x || pA.x > pB.z);
	IF_F(pA.w < pB.y || pA.y > pB.w);

	return true;
}

template <typename T> inline T constrain(T v, T a, T b)
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

inline void pack_int16(void* pB, int16_t v, bool bOrder = true)
{
	if(bOrder)
		v = (int16_t)htons((uint16_t)v);

	memcpy(pB, &v, sizeof(int16_t));
}

inline int16_t unpack_int16(const void* pB, bool bOrder = true)
{
	int16_t v = 0;
	memcpy(&v, pB, sizeof(int16_t));

	if(bOrder)
		v = (int16_t)ntohs((uint16_t)v);

	return v;
}

inline void pack_uint16(void* pB, uint16_t v, bool bOrder = true)
{
	if(bOrder)
		v = htons(v);

	memcpy(pB, &v, sizeof(uint16_t));
}

inline uint16_t unpack_uint16(const void* pB, bool bOrder = true)
{
	uint16_t v = 0;
	memcpy(&v, pB, sizeof(uint16_t));

	if(bOrder)
		v = ntohs(v);

	return v;
}

inline void pack_int32(void* pB, int32_t v, bool bOrder = true)
{
	if(bOrder)
		v = (int32_t)htonl((uint32_t)v);

	memcpy(pB, &v, sizeof(int32_t));
}

inline int32_t unpack_int32(const void* pB, bool bOrder = true)
{
	int32_t v = 0;
	memcpy(&v, pB, sizeof(int32_t));

	if(bOrder)
		v = (int32_t)ntohl((uint32_t)v);

	return v;
}

inline void pack_uint32(void* pB, uint32_t v, bool bOrder = true)
{
	if(bOrder)
		v = htonl(v);

	memcpy(pB, &v, sizeof(uint32_t));
}

inline uint32_t unpack_uint32(const void* pB, bool bOrder = true)
{
	uint32_t v = 0;
	memcpy(&v, pB, sizeof(uint32_t));

	if(bOrder)
		v = ntohl(v);

	return v;
}

inline void f2b(uint8_t *pB, float f)
{
	uint8_t *pF = (uint8_t*) &f;

	for (uint8_t i = 0; i < sizeof(float); i++)
	{
		*pB = *pF;
		pF++;
		pB++;
	}
}

#define UTIL_BUF 32

inline string i2str(int v)
{
	char buf[UTIL_BUF];
	snprintf(buf, UTIL_BUF, "%d", v);
	return string(buf);
}

inline string li2str(long v)
{
	char buf[UTIL_BUF];
	snprintf(buf, UTIL_BUF, "%ld", v);
	return string(buf);
}

inline string f2str(float v)
{
	char buf[UTIL_BUF];
	snprintf(buf, UTIL_BUF, "%.3f", v);
	return string(buf);
}

inline string f2str(float v, int nDigit)
{
	char buf[UTIL_BUF];
	string format = "%."+i2str(nDigit)+"f";
	snprintf(buf, UTIL_BUF, format.c_str(), (float)v);
	return string(buf);
}

inline string lf2str(double v)
{
	char buf[UTIL_BUF];
	snprintf(buf, UTIL_BUF, "%.3f", (double)v);
	return string(buf);
}

inline string lf2str(double v, int nDigit)
{
	char buf[UTIL_BUF];
	string format = "%."+i2str(nDigit)+"f";
	snprintf(buf, UTIL_BUF, format.c_str(), (double)v);
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

#ifdef USE_OPENCV
inline float hist(Mat m, float rFrom, float rTo, int nLevel, float nMin)
{
	IF__(m.empty(), -1.0);

    vector<int> vHistLev = { nLevel };
	vector<float> vRange = { rFrom, rTo };
	vector<int> vChannel = { 0 };

	vector<Mat> vRoi = {m};
	Mat mHist;
	cv::calcHist(vRoi, vChannel, Mat(),
	            mHist, vHistLev, vRange,
	            true	//accumulate
				);

	int nMinHist = nMin * m.cols * m.rows;
	int i;
	for(i=0; i<nLevel; i++)
	{
		if(mHist.at<float>(i) >= (float)nMinHist)break;
	}

	return rFrom + (((float)i)/(float)nLevel) * (rTo - rFrom);
}

template <typename T> inline T rect2BB(Rect r)
{
	T v;
	v.x = r.x;
	v.y = r.y;
	v.z = r.x + r.width;
	v.w = r.y + r.height;

	return v;
}

template <typename T> inline Rect bb2Rect(T v)
{
	Rect r;
	r.x = v.x;
	r.y = v.y;
	r.width = v.z - v.x;
	r.height = v.w - v.y;

	return r;
}

inline float IoU(Rect2f& r1, Rect2f& r2)
{
	Rect2f rOR = r1 | r2;
	Rect2f rAND = r1 & r2;
	return rAND.area() / rOR.area();
}

inline float IoU(vFloat4& bb1, vFloat4& bb2)
{
	Rect2f r1 = bb2Rect(bb1);
	Rect2f r2 = bb2Rect(bb2);
	return IoU(r1,r2);
}

inline float nIoU(Rect2f& r1, Rect2f& r2)
{
	Rect2f rAND = r1 & r2;
	return rAND.area() / small(r1.area(), r2.area());
}

inline float nIoU(vFloat4& bb1, vFloat4& bb2)
{
	Rect2f r1 = bb2Rect(bb1);
	Rect2f r2 = bb2Rect(bb2);
	return nIoU(r1,r2);
}

#endif

}
#endif
