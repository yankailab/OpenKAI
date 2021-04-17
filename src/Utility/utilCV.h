#ifndef OpenKAI_src_Utility_utilCV_H_
#define OpenKAI_src_Utility_utilCV_H_

#ifdef USE_OPENCV
#include "../Base/cv.h"
#include "../Base/platform.h"
#include "../Base/macro.h"
#include "../Base/constant.h"

namespace kai
{

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

}
#endif
#endif
