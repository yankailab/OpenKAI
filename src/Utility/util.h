#ifndef OPENKAI_SRC_UTILITY_UTIL_H_
#define OPENKAI_SRC_UTILITY_UTIL_H_

#include "../Base/platform.h"
#include "../Base/cv.h"
#include <time.h>
#include <sys/time.h>

using namespace std;

namespace kai
{

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
	if (pA->m_z < pB->m_x || pA->m_x > pB->m_z)
		return false;

	if (pA->m_w < pB->m_y || pA->m_y > pB->m_w)
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
	double w = pA->w();
	if (pB->w() > w)
		w = pB->w();
	double h = pA->h();
	if (pB->h() > h)
		h = pB->h();

	double rX = pB->m_z - pA->m_x;
	if (pB->m_x > pA->m_x)
		rX = pA->m_z - pB->m_x;

	rX /= w;

	double rY = pB->m_w - pA->m_y;
	if (pB->m_y > pA->m_y)
		rY = pA->m_w - pB->m_y;

	rY /= h;

	return rX * rY;
}

inline void rect2vInt4(Rect* pR, vInt4* pV)
{
	pV->m_x = pR->x;
	pV->m_y = pR->y;
	pV->m_z = pR->x + pR->width;
	pV->m_w = pR->y + pR->height;
}

inline void vInt42rect(kai::vInt4* pV, Rect* pR)
{
	pR->x = pV->m_x;
	pR->y = pV->m_y;
	pR->width = pV->m_z - pV->m_x;
	pR->height = pV->m_w - pV->m_y;
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
	sprintf(buf, "%f", val);
	return string(buf);
}

inline string i2str(int val)
{
	char buf[128];
	sprintf(buf, "%d", val);
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

//// Checks if a matrix is a valid rotation matrix.
//bool isRotationMatrix(Mat &R)
//{
//    Mat Rt;
//    transpose(R, Rt);
//    Mat shouldBeIdentity = Rt * R;
//    Mat I = Mat::eye(3,3, shouldBeIdentity.type());
//
//    return  norm(I, shouldBeIdentity) < 1e-6;
//
//}
//
//// Calculates rotation matrix to euler angles
//// The result is the same as MATLAB except the order
//// of the euler angles ( x and z are swapped ).
//Vec3f rotationMatrixToEulerAngles(Mat &R)
//{
//    assert(isRotationMatrix(R));
//
//    float sy = sqrt(R.at<double>(0,0) * R.at<double>(0,0) +  R.at<double>(1,0) * R.at<double>(1,0) );
//
//    bool singular = sy < 1e-6; // If
//
//    float x, y, z;
//    if (!singular)
//    {
//        x = atan2(R.at<double>(2,1) , R.at<double>(2,2));
//        y = atan2(-R.at<double>(2,0), sy);
//        z = atan2(R.at<double>(1,0), R.at<double>(0,0));
//    }
//    else
//    {
//        x = atan2(-R.at<double>(1,2), R.at<double>(1,1));
//        y = atan2(-R.at<double>(2,0), sy);
//        z = 0;
//    }
//    return Vec3f(x, y, z);
//}

// Converts a given Rotation Matrix to Euler angles
inline cv::Mat rot2euler(const cv::Mat & rotationMatrix)
{
	cv::Mat euler(3, 1, CV_64F);

	double m00 = rotationMatrix.at<double>(0, 0);
	double m02 = rotationMatrix.at<double>(0, 2);
	double m10 = rotationMatrix.at<double>(1, 0);
	double m11 = rotationMatrix.at<double>(1, 1);
	double m12 = rotationMatrix.at<double>(1, 2);
	double m20 = rotationMatrix.at<double>(2, 0);
	double m22 = rotationMatrix.at<double>(2, 2);

	double x, y, z;

	// Assuming the angles are in radians.
	if (m10 > 0.998)
	{ // singularity at north pole
		x = 0;
		y = CV_PI / 2;
		z = atan2(m02, m22);
	}
	else if (m10 < -0.998)
	{ // singularity at south pole
		x = 0;
		y = -CV_PI / 2;
		z = atan2(m02, m22);
	}
	else
	{
		x = atan2(-m12, m11);
		y = asin(m10);
		z = atan2(-m20, m00);
	}

	euler.at<double>(0) = x;
	euler.at<double>(1) = y;
	euler.at<double>(2) = z;

	return euler;
}

// Converts a given Euler angles to Rotation Matrix
inline cv::Mat euler2rot(const cv::Mat & euler)
{
	cv::Mat rotationMatrix(3, 3, CV_64F);

	double x = euler.at<double>(0);
	double y = euler.at<double>(1);
	double z = euler.at<double>(2);

	// Assuming the angles are in radians.
	double ch = cos(z);
	double sh = sin(z);
	double ca = cos(y);
	double sa = sin(y);
	double cb = cos(x);
	double sb = sin(x);

	double m00, m01, m02, m10, m11, m12, m20, m21, m22;

	m00 = ch * ca;
	m01 = sh * sb - ch * sa * cb;
	m02 = ch * sa * sb + sh * cb;
	m10 = sa;
	m11 = ca * cb;
	m12 = -ca * sb;
	m20 = -sh * ca;
	m21 = sh * sa * cb + ch * sb;
	m22 = -sh * sa * sb + ch * cb;

	rotationMatrix.at<double>(0, 0) = m00;
	rotationMatrix.at<double>(0, 1) = m01;
	rotationMatrix.at<double>(0, 2) = m02;
	rotationMatrix.at<double>(1, 0) = m10;
	rotationMatrix.at<double>(1, 1) = m11;
	rotationMatrix.at<double>(1, 2) = m12;
	rotationMatrix.at<double>(2, 0) = m20;
	rotationMatrix.at<double>(2, 1) = m21;
	rotationMatrix.at<double>(2, 2) = m22;

	return rotationMatrix;
}

}

#endif

