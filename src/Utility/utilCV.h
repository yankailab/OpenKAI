#ifndef OpenKAI_src_Utility_utilCV_H_
#define OpenKAI_src_Utility_utilCV_H_

#include "../Base/cv.h"
#include "../Base/platform.h"
#include "../Base/macro.h"
#include "../Base/constant.h"

namespace kai
{

	template <typename T>
	inline T rect2BB(cv::Rect r)
	{
		T v;
		v.x() = r.x;
		v.y() = r.y;
		v.z() = r.x + r.width;
		v.w() = r.y + r.height;

		return v;
	}

	template <typename T>
	inline cv::Rect bb2Rect(T v)
	{
		cv::Rect r;
		r.x = v.x();
		r.y = v.y();
		r.width = v.z() - v.x();
		r.height = v.w() - v.y();

		return r;
	}


}

#endif
