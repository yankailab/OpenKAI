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
	m_width = 0;
	m_height = 0;
	m_camDeviceID = 0;
	m_bCalibration = false;
	m_bGimbal = false;
}

CamInput::~CamInput()
{
	// TODO Auto-generated destructor stub
}

bool CamInput::setup(JSON* pJson, string camName)
{
	string calibFile;

	CHECK_FATAL(pJson->getVal("CAM_"+camName+"_ID", &m_camDeviceID));
	CHECK_FATAL(pJson->getVal("CAM_"+camName+"_WIDTH", &m_width));
	CHECK_FATAL(pJson->getVal("CAM_"+camName+"_HEIGHT", &m_height));

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

	m_centerH = m_width * 0.5;
	m_centerV = m_height * 0.5;

	return true;
}

void CamInput::readFrame(CamFrame* pFrame)
{
	while (!m_camera.read(m_frame));

	m_Gframe.upload(m_frame);

	if(m_bCalibration)
	{
		//TODO: to GPU
//		Mat tmp;
//		undistort(m_frame, tmp, m_cameraMat, m_distCoeffs);
//		m_frame = tmp;
	}

	if(m_bGimbal)
	{
		cuda::warpAffine(m_Gframe, m_Gframe2, m_rotRoll, m_Gframe.size());
		pFrame->updateFrame(&m_Gframe2);
	}
	else
	{
		pFrame->updateFrame(&m_Gframe);
	}

}

void CamInput::setAttitude(double rollRad, double pitchRad, uint64_t timestamp)
{
    Point2f center(m_centerH, m_centerV);
    double deg = -rollRad * 180.0 * OneOvPI;

    m_rotRoll = getRotationMatrix2D(center, deg, 1.5);

}


} /* namespace kai */
