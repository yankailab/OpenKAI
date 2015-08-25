/*
 * CameraInput.h
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#ifndef SRC_CAMFRAME_H_
#define SRC_CAMFRAME_H_

#include "common.h"
#include "stdio.h"
#include "cvplatform.h"

namespace kai
{

class CamFrame
{
public:
	CamFrame();
	virtual ~CamFrame();

	bool init(void);
	void updateFrame(Mat* pFrame);
	void switchFrame(void);

	void resize(int width, int height, CamFrame* pResult);
	void getGray(CamFrame* pResult);
	void getHSV(CamFrame* pResult);

public:
	//Frame Index, updated in each frame update
	unsigned int m_frameID;

	Mat m_uFrame;
	unsigned char m_iFrame;

	unsigned int m_width;
	unsigned int m_height;

#ifdef USE_CUDA
	GpuMat* m_pNext;
	GpuMat* m_pPrev;
	GpuMat m_pFrame[2];

#else
	UMat* m_pNext;
	UMat* m_pPrev;
	UMat m_pFrame[2];
#endif

};

}/*namespace kai*/

#endif /* SRC_CAMFRAME_H_ */
