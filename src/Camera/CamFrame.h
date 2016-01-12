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

	void updateFrame(GpuMat* pGpuFrame);
	void updateFrame(Mat* pFrame);
	void updateFrame(CamFrame* pFrame);

	void switchFrame(void);
	void updateFrameSwitch(GpuMat* pGpuFrame);
	void updateFrameSwitch(Mat* pFrame);
	void updateFrameSwitch(CamFrame* pFrame);

	void getResized(int width, int height, CamFrame* pResult);
	void getGray(CamFrame* pResult);
	void getHSV(CamFrame* pResult);
	void getBGRA(CamFrame* pResult);
	void get8UC3(CamFrame* pResult);
	GpuMat* getCurrentFrame(void);
	GpuMat* getPreviousFrame(void);

	bool isNewerThan(CamFrame* pFrame);
	uint64_t getFrameID(void);
	bool	 empty(void);

private:
	//Time for last update, updated in each frame update
	uint64_t m_frameID;

	//index to m_pFrame
	unsigned char m_iFrame;
	GpuMat* m_pNext;
	GpuMat* m_pPrev;
	GpuMat  m_pFrame[2];
	GpuMat  m_GMat;

	cuda::Stream		m_cudaStream;


};

}/*namespace kai*/

#endif /* SRC_CAMFRAME_H_ */
