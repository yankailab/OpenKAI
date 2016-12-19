/*
 * Frame.h
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#ifndef SRC_STREAM_FRAME_H_
#define SRC_STREAM_FRAME_H_

#include "../Base/common.h"

#ifdef USE_OPENCV3
using namespace cv::cuda;
#elif USE_OPENCV4TEGRA
using namespace cv::gpu;
#endif

namespace kai
{

struct CPU_MAT
{
	uint64_t m_frameID;
	Mat m_mat;
};

#ifdef USE_CUDA
struct GPU_MAT
{
	//	//Time for last update, updated in each frame update
	uint64_t m_frameID;
	GpuMat m_mat;
};
#endif

#ifdef USE_OPENCL
struct U_MAT
{
	uint64_t m_frameID;
	UMat m_mat;
};
#endif



class Frame
{
public:
	Frame();
	virtual ~Frame();

	void allocate(int w, int h);

	void update(Mat* pFrame);
	void update(Frame* pFrame);
	inline void updatedCMat(void);

	Mat* getCMat(void);
	void getResizedOf(Frame* pFrom, int width, int height);
	void getGrayOf(Frame* pFrom);
	void getHSVOf(Frame* pFrom);
	void getBGRAOf(Frame* pFrom);
	void getRGBAOf(Frame* pFrom);
	void get8UC3Of(Frame* pFrom);
	void get32FC4Of(Frame* pFrom);

	Size getSize(void);

	bool isNewerThan(Frame* pFrame);
	uint64_t getFrameID(void);
	bool	 empty(void);

#ifdef USE_CUDA
	void update(GpuMat* pGpuFrame);
	void updatedGMat(void);
	GpuMat* getGMat(void);
#endif

private:
	CPU_MAT	m_CMat;

#ifdef USE_CUDA
	GPU_MAT	m_GMat;
#endif

};

}

#endif
