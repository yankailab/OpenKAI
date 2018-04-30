/*
 * FrameBase.h
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision_FrameBase_H_
#define OpenKAI_src_Vision_FrameBase_H_

#include "../Base/common.h"

namespace kai
{

class FrameBase
{
public:
	FrameBase();
	virtual ~FrameBase();

	virtual void operator=(const Mat& m);
	virtual void allocate(int w, int h);
	virtual void copy(const FrameBase& f);
	virtual void copy(const Mat& m);

	virtual FrameBase warpAffine(Mat& mWA);
	virtual FrameBase flip(int iOpt);
	virtual void setRemap(Mat& mX, Mat& mY);
	virtual FrameBase remap(void);
	virtual FrameBase crop(Rect bb);
	virtual FrameBase resize(int w, int h);
	virtual FrameBase resize(double scaleW, double scaleH);
	virtual FrameBase gray(void);
	virtual FrameBase hsv(void);
	virtual FrameBase bgra(void);
	virtual FrameBase rgba(void);
	virtual FrameBase f8UC3(void);
	virtual FrameBase f32FC4(void);

	virtual Mat* m(void);
	virtual Size size(void);
	virtual bool bEmpty(void);

	uint64_t tStamp(void);

public:
	uint64_t m_tStamp;
	Mat m_mat;

	//calibration remap
	Mat m_mapX;
	Mat m_mapY;

};

}
#endif
