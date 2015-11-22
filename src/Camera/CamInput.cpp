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
	m_bCalibration = false;
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

	FileStorage fs("/Users/yankai/Documents/workspace/LAB/src/logitech_c930e.xml", FileStorage::READ);
	if (!fs.isOpened())
	{
		printf("Cannot open camera setting file");
		m_bCalibration = false;
		return true;
	}

	fs["camera_matrix"] >> m_cameraMat;
	fs["distortion_coefficients"] >> m_distCoeffs;

	printf("Camera setting file opened");
	cout << endl << "camMatrix = " << m_cameraMat << endl;
	cout << "distMat = " << m_distCoeffs << endl << endl;

	m_bCalibration = true;
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

	m_bCalibration = false;
	if(m_bCalibration)
	{
		Mat tmp;
		undistort(m_frame, tmp, m_cameraMat, m_distCoeffs);
		m_frame = tmp;
	}

	pFrame->updateFrame(&m_frame);
}

} /* namespace kai */
