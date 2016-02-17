/*
 * CameraInput.h
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#ifndef SRC_CAMFRAME_H_
#define SRC_CAMFRAME_H_

#include <pthread.h>
#include "stdio.h"
#include "../Base/common.h"
#include "../Base/cvplatform.h"
#include "../Utility/util.h"

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



class CamFrame
{
public:
	CamFrame();
	virtual ~CamFrame();

	void update(Mat* pFrame);
	void update(CamFrame* pFrame);
	void updatedCMat(void);

	Mat* getCMat(void);
	void getResizedOf(CamFrame* pFrom, int width, int height);
	void getGrayOf(CamFrame* pFrom);
	void getHSVOf(CamFrame* pFrom);
	void getBGRAOf(CamFrame* pFrom);
	void get8UC3Of(CamFrame* pFrom);

	Size getSize(void);

	bool isNewerThan(CamFrame* pFrame);
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
	cuda::Stream	m_cudaStream;
#endif

};

}/*namespace kai*/

#endif /* SRC_CAMFRAME_H_ */
