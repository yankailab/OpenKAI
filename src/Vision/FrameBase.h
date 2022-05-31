/*
 * FrameBase.h
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision_FrameBase_H_
#define OpenKAI_src_Vision_FrameBase_H_

#include "../Base/common.h"
#include "../Base/cv.h"

namespace kai
{
	class FrameBase
	{
	public:
		FrameBase();
		virtual ~FrameBase();

		virtual void operator=(const Mat &m);
		virtual void allocate(int w, int h);
		virtual void copy(const FrameBase &f);
		virtual void copy(const Mat &m);

		virtual FrameBase warpAffine(Mat &mWA);
		virtual FrameBase flip(int iOpt);
		virtual FrameBase remap(const Mat &mX, const Mat &mY);
		virtual FrameBase crop(Rect bb);
		virtual FrameBase resize(int w, int h);
		virtual FrameBase resize(double scaleW, double scaleH);
		virtual FrameBase cvtTo(int rType);
		virtual FrameBase cvtColor(int code);

		virtual Mat *m(void);
		virtual Size size(void);
		virtual bool bEmpty(void);

		uint64_t tStamp(void);

	public:
		uint64_t m_tStamp;
		Mat m_mat;
	};

}
#endif
