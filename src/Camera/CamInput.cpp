/*
 * CamInput.cpp
 *
 *  Created on: Aug 22, 2015
 *      Author: yankai
 */

#include "CamInput.h"

namespace kai
{

CamInput::CamInput()
{
	// TODO Auto-generated constructor stub
	m_width = 0;
	m_height = 0;
	m_camDeviceID = 0;
}

CamInput::~CamInput()
{
	// TODO Auto-generated destructor stub
}

bool CamInput::openCamera(void)
{
	m_camera.open(m_camDeviceID);
	if (!m_camera.isOpened())
	{
		LOG(ERROR)<< "Cannot open camera:" << m_camDeviceID;
		return false;
	}

	return true;
}

bool CamInput::setSize(void)
{
	m_camera.set(CV_CAP_PROP_FRAME_WIDTH, m_width);
	m_camera.set(CV_CAP_PROP_FRAME_HEIGHT, m_height);

	return true;
}

void CamInput::readFrame(CamFrame* pFrame)
{
	while (!m_camera.read(m_frame));

	pFrame->updateFrame(&m_frame);
}

} /* namespace kai */
