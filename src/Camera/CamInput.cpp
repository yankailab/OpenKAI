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
	m_centerH = 0;
	m_centerV = 0;
	m_camDeviceID = 0;
	m_bCalibration = false;
	m_bGimbal = false;
	m_isoScale = 1.0;
	m_rotTime = 0;
	m_rotPrev = 0;
}

CamInput::~CamInput()
{
}

bool CamInput::setup(JSON* pJson, string camName)
{
	string calibFile;

	CHECK_FATAL(pJson->getVal("CAM_"+camName+"_ID", &m_camDeviceID));
	CHECK_FATAL(pJson->getVal("CAM_"+camName+"_WIDTH", &m_width));
	CHECK_FATAL(pJson->getVal("CAM_"+camName+"_HEIGHT", &m_height));
	CHECK_INFO(pJson->getVal("CAM_"+camName+"_ISOSCALE", &m_isoScale));
	CHECK_INFO(pJson->getVal("CAM_"+camName+"_CALIB", &m_bCalibration));

	if(pJson->getVal("CAM_"+camName+"_CALIBFILE", &calibFile))
	{
		FileStorage fs(calibFile, FileStorage::READ);
		if (!fs.isOpened())
		{
			LOG(INFO)<<"Camera calibration file not found";
			m_bCalibration = false;
		}
		else
		{
			fs["camera_matrix"] >> m_cameraMat;
			fs["distortion_coefficients"] >> m_distCoeffs;
			fs.release();

			Mat map1, map2;
			cv::Size imSize(m_width,m_height);

	        initUndistortRectifyMap(
	        		m_cameraMat,
	        		m_distCoeffs,
				Mat(),
	            getOptimalNewCameraMatrix(
	            		m_cameraMat,
					m_distCoeffs,
					imSize,
					1,
					imSize,
					0),
				imSize,
				CV_32FC1,
				map1,
				map2);

	        m_Gmap1.upload(map1);
	        m_Gmap2.upload(map2);

//			m_bCalibration = true;
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

	m_camera.set(CV_CAP_PROP_FRAME_WIDTH, m_width);
	m_camera.set(CV_CAP_PROP_FRAME_HEIGHT, m_height);

	m_centerH = m_width * 0.5;
	m_centerV = m_height * 0.5;

	return true;
}

void CamInput::readFrame(CamFrame* pFrame)
{
	GpuMat* pSrc;
	GpuMat* pDest;
	GpuMat* pTmp;

	while (!m_camera.read(m_frame));
	m_Gframe.upload(m_frame);

	pSrc = &m_Gframe;
	pDest = &m_Gframe2;

	if(m_bCalibration)
	{
        cuda::remap(*pSrc, *pDest, m_Gmap1, m_Gmap2, INTER_LINEAR);
        SWAP(pSrc,pDest,pTmp);
	}

	if(m_bGimbal)
	{
		cuda::warpAffine(*pSrc, *pDest, m_rotRoll, m_Gframe.size());
        SWAP(pSrc,pDest,pTmp);
	}

	pFrame->updateFrame(pSrc);

}

void CamInput::setAttitude(double rollRad, double pitchRad, uint64_t timestamp)
{
    Point2f center(m_centerH, m_centerV);
    double deg = -rollRad * 180.0 * OneOvPI;

    m_rotRoll = getRotationMatrix2D(center, deg, m_isoScale);

    //TODO: add rot estimation

}


} /* namespace kai */
