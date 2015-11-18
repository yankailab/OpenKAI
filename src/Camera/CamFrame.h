/*
 * CameraInput.h
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#ifndef SRC_CAMFRAME_H_
#define SRC_CAMFRAME_H_

#include "../Base/common.h"
#include "stdio.h"
#include "../Base/cvplatform.h"

#define PI 3.141592653589793
#define OneOvPI 0.31830988618

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
	void getBGRA(CamFrame* pResult);

	void rotate(double radian);

public:
	//Frame Index, updated in each frame update
	unsigned int m_frameID;

	Mat m_uFrame;
	unsigned char m_iFrame;

	unsigned int m_width;
	unsigned int m_height;
	unsigned int m_centerH;
	unsigned int m_centerV;

#ifdef USE_CUDA
	GpuMat* m_pNext;
	GpuMat* m_pPrev;
	GpuMat m_pFrame[2];
	GpuMat m_tmpMat;

#else
	UMat* m_pNext;
	UMat* m_pPrev;
	UMat m_pFrame[2];
#endif

};

}/*namespace kai*/

#endif /* SRC_CAMFRAME_H_ */
