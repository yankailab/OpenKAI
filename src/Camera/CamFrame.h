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




namespace kai
{

class CamFrame
{
public:
	CamFrame();
	virtual ~CamFrame();

	bool init(void);
	void updateFrame(GpuMat* pGpuFrame);
	void switchFrame(void);

	void resize(int width, int height, CamFrame* pResult);

	void getGray(CamFrame* pResult);
	void getHSV(CamFrame* pResult);
	void getBGRA(CamFrame* pResult);

public:
	//Frame Index, updated in each frame update
	unsigned int m_frameID;

	//index to m_pFrame
	unsigned char m_iFrame;

	GpuMat* m_pNext;
	GpuMat* m_pPrev;
	GpuMat  m_pFrame[2];

};

}/*namespace kai*/

#endif /* SRC_CAMFRAME_H_ */
