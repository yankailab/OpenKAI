/*
 * FrameGPU.h
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision_FrameGPU_H_
#define OpenKAI_src_Vision_FrameGPU_H_

#include "../Base/common.h"

#ifdef USE_OPENCV
#ifdef USE_CUDA

namespace kai
{

class FrameGPU
{
public:
	FrameGPU();
	virtual ~FrameGPU();

	virtual void operator=(const Mat& m);
	virtual void allocate(int w, int h);
	virtual void copy(const FrameGPU& f);
	virtual void copy(const Mat& m);

	virtual FrameGPU warpAffine(Mat& mWA);
	virtual FrameGPU flip(int iOpt);
	virtual FrameGPU remap(const Mat& mX, const Mat& mY);
	virtual FrameGPU crop(Rect bb);
	virtual FrameGPU resize(int w, int h);
	virtual FrameGPU resize(double scaleW, double scaleH);
	virtual FrameGPU cvtTo(int rType);
	virtual FrameGPU cvtColor(int code);

	virtual Mat* m(void);
	virtual Size size(void);
	virtual bool bEmpty(void);

	virtual void operator=(const GpuMat& m);
	virtual void copy(const GpuMat& m);
	virtual GpuMat* gm(void);
	virtual uint64_t tStamp(void);

private:
	void sync(void);
	void updateG(void);
	void update(void);

	void updateTstamp(uint64_t t);
	void updateTstampG(uint64_t t);

public:
	uint64_t m_tStamp;
	Mat m_mat;

	uint64_t m_tStampG;
	GpuMat m_matG;
};

}
#endif
#endif
#endif
