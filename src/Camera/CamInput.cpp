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

bool CamInput::setup(JSON* pJson, string camName)
{
	string calibFile;

	if(pJson->getVal("CAM_"+camName+"_CALIB", &calibFile))
	{
		FileStorage fs(calibFile, FileStorage::READ);
		if (!fs.isOpened())
		{
			LOG(INFO)<<"Camera calibration file not found";
			m_bCalibration = false;
		}
		else
		{
			LOG(INFO)<<"Camera setting file opened";

			fs["camera_matrix"] >> m_cameraMat;
			fs["distortion_coefficients"] >> m_distCoeffs;
			fs.release();
			m_bCalibration = true;
		}
	}

	return true;
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
