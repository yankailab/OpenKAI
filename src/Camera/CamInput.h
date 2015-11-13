/*
 * CamInput.h
 *
 *  Created on: Aug 22, 2015
 *      Author: yankai
 */

#ifndef SRC_CAMINPUT_H_
#define SRC_CAMINPUT_H_

#include "../Base/common.h"
#include "stdio.h"
#include "../Base/cvplatform.h"

#include "../Camera/CamFrame.h"

namespace kai
{

class CamInput
{
public:
	CamInput();
	virtual ~CamInput();

	void setCameraID(int);
	bool openCamera(void);
	bool setSize(void);

	void readFrame(CamFrame* pFrame);

public:
	int m_camDeviceID;
	VideoCapture m_camera;

	int m_width;
	int m_height;

	Mat	m_frame;


};

} /* namespace kai */

#endif /* SRC_CAMINPUT_H_ */
