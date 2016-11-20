/*
 * Camera.h
 *
 *  Created on: Aug 22, 2015
 *      Author: yankai
 */

#ifndef SRC_CAMERA_H_
#define SRC_CAMERA_H_

#include "../Base/common.h"
#include "CamBase.h"

namespace kai
{

class Camera: public CamBase
{
public:
	Camera();
	virtual ~Camera();

	bool setup(Config* pConfig);
	bool openCamera(void);
	GpuMat* readFrame(void);
	GpuMat* getDepthFrame(void);
	void release(void);

	void setCameraID(int);

public:
	int m_camDeviceID;
	VideoCapture m_camera;

};

} /* namespace kai */

#endif /* SRC_CAMERA_H_ */
