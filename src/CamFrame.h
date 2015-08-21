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

#define MAX_FRAME_ID 0xffffffff

namespace kai
{

class CamFrame
{
public:
	CamFrame();
	virtual ~CamFrame();

	bool init(void);
	void updateFrame(UMat* pFrame);
	void switchFrame(void);

	void resize(int width, int height, CamFrame* pResult);
	void getGray(CamFrame* pResult);
	void getHSV(CamFrame* pResult);

public:
	//Frame Index, updated in each frame update
	unsigned int m_frameID;
	UMat m_uFrame;
	unsigned char m_iFrame;

	unsigned int m_width;
	unsigned int m_height;

#ifdef USE_CUDA
	GpuMat* m_pNext;
	GpuMat* m_pPrev;
	GpuMat m_pFrame[2];

	GpuMat  m_Hsv;
	GpuMat  m_Hue;
	GpuMat  m_Sat;
	GpuMat  m_Val;
#else
	UMat* m_pNext;
	UMat* m_pPrev;
	UMat m_pFrame[2];

	UMat  m_Hsv;
	UMat  m_Hue;
	UMat  m_Sat;
	UMat  m_Val;
#endif

};

}/*namespace kai*/

#endif /* SRC_CAMFRAME_H_ */
