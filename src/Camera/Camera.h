/*
 * CamInput.h
 *
 *  Created on: Aug 22, 2015
 *      Author: yankai
 */

#ifndef SRC_CAMERA_H_
#define SRC_CAMERA_H_

#include "stdio.h"
#include "../Base/common.h"
#include "../Base/cvplatform.h"
#include "../Stream/Frame.h"
#include "CamBase.h"

namespace kai
{

class Camera: public CamBase
{
public:
	Camera();
	virtual ~Camera();

	bool setup(Config* pConfig, string camName);
	bool openCamera(void);
	GpuMat* readFrame(void);
	GpuMat* getDepthFrame(void);
	void release(void);

	void setCameraID(int);

public:
	int m_camType;
	int m_camDeviceID;
	VideoCapture m_camera;

};

} /* namespace kai */

#endif /* SRC_CAMERA_H_ */
