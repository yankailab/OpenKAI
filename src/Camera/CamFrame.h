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

class CamFrame
{
public:
	CamFrame();
	virtual ~CamFrame();

	void update(Mat* pFrame);
	void update(CamFrame* pFrame);

	void switchFrame(void);
	void updateSwitch(Mat* pFrame);
	void updateSwitch(CamFrame* pFrame);

	void getResizedOf(CamFrame* pFrom, int width, int height);
	void getGrayOf(CamFrame* pFrom);
	void getHSVOf(CamFrame* pFrom);
	void getBGRAOf(CamFrame* pFrom);
	void get8UC3Of(CamFrame* pFrom);

	bool isNewerThan(CamFrame* pFrame);
	uint64_t getFrameID(void);
	bool	 empty(void);

//	Mat* getCurrent(void);
//	Mat* getPrevious(void);

#ifdef USE_CUDA
	void update(GpuMat* pGpuFrame);
	void updateSwitch(GpuMat* pGpuFrame);
	GpuMat* getCurrent(void);
	GpuMat* getPrevious(void);
#endif


private:

//	//index to m_pFrame
//	unsigned char m_iFrame_CPU;
//	//Time for last update, updated in each frame update
//	uint64_t m_frameID_CPU;
//	Mat* m_pNext;
//	Mat* m_pPrev;
//	Mat  m_pFrame[2];
//	Mat  m_Mat;

#ifdef USE_CUDA
	unsigned char m_iFrame_GPU;
	uint64_t m_frameID_GPU;
	GpuMat* m_pNext;
	GpuMat* m_pPrev;
	GpuMat  m_pFrame[2];
	cuda::Stream		m_cudaStream;
#endif

};

}/*namespace kai*/

#endif /* SRC_CAMFRAME_H_ */
