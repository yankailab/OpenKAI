/*
 * CamBase.cpp
 *
 *  Created on: Aug 22, 2015
 *      Author: yankai
 */

#include "CamBase.h"

namespace kai
{

CamBase::CamBase()
{
//	m_camType = CAM_GENERAL;
	m_width = 0;
	m_height = 0;
	m_centerH = 0;
	m_centerV = 0;
//	m_camDeviceID = 0;
	m_bCalibration = false;
	m_bGimbal = false;
	m_bFisheye = false;
	m_isoScale = 1.0;
	m_rotTime = 0;
	m_rotPrev = 0;
	m_angleH = 0;
	m_angleV = 0;
	m_bCrop = 0;

#ifdef USE_ZED
	m_zedResolution = (int)sl::zed::VGA;
	m_zedMinDist = 1000;
#endif
}

CamBase::~CamBase()
{
}

bool CamBase::setup(JSON* pJson, string name)
{
	string presetDir = "";
	string calibFile;

	CHECK_INFO(pJson->getVal("PRESET_DIR", &presetDir));

//	CHECK_FATAL(pJson->getVal("CAM_" + name + "_ID", &m_camDeviceID));
	CHECK_FATAL(pJson->getVal("CAM_" + name + "_WIDTH", &m_width));
	CHECK_FATAL(pJson->getVal("CAM_" + name + "_HEIGHT", &m_height));
	CHECK_FATAL(pJson->getVal("CAM_" + name + "_ANGLE_V", &m_angleV));
	CHECK_FATAL(pJson->getVal("CAM_" + name + "_ANGLE_H", &m_angleH));

	CHECK_INFO(pJson->getVal("CAM_" + name + "_ISOSCALE", &m_isoScale));
	CHECK_INFO(pJson->getVal("CAM_" + name + "_CALIB", &m_bCalibration));
	CHECK_INFO(pJson->getVal("CAM_" + name + "_GIMBAL", &m_bGimbal));
	CHECK_INFO(pJson->getVal("CAM_" + name + "_FISHEYE", &m_bFisheye));
//	CHECK_INFO(pJson->getVal("CAM_" + name + "_TYPE", &m_camType));

#ifdef USE_ZED
	CHECK_INFO(pJson->getVal("CAM_" + camName + "_ZED_RESOLUTION", &m_zedResolution));
	CHECK_INFO(pJson->getVal("CAM_" + camName + "_ZED_MIN_DISTANCE", &m_zedMinDist));
#endif

	CHECK_INFO(pJson->getVal("CAM_" + name + "_CROP", &m_bCrop));
	if(m_bCrop!=0)
	{
		CHECK_FATAL(pJson->getVal("CAM_" + name + "_CROP_X", &m_cropBB.x));
		CHECK_FATAL(pJson->getVal("CAM_" + name + "_CROP_Y", &m_cropBB.y));
		CHECK_FATAL(pJson->getVal("CAM_" + name + "_CROP_W", &m_cropBB.width));
		CHECK_FATAL(pJson->getVal("CAM_" + name + "_CROP_H", &m_cropBB.height));
	}

	if (pJson->getVal("CAM_" + name + "_CALIBFILE", &calibFile))
	{
		FileStorage fs(presetDir + calibFile, FileStorage::READ);
		if (!fs.isOpened())
		{
			LOG(ERROR)<<"Camera calibration file not found";
			m_bCalibration = false;
		}
		else
		{
			fs["camera_matrix"] >> m_cameraMat;
			fs["distortion_coefficients"] >> m_distCoeffs;
			fs.release();

			Mat map1, map2;
			cv::Size imSize(m_width,m_height);

			if(m_bFisheye)
			{
				Mat newCamMat;
				fisheye::estimateNewCameraMatrixForUndistortRectify(m_cameraMat, m_distCoeffs, imSize,
						Matx33d::eye(), newCamMat, 1);
				fisheye::initUndistortRectifyMap(m_cameraMat, m_distCoeffs, Matx33d::eye(), newCamMat, imSize,
						CV_32F/*CV_16SC2*/, map1, map2);
			}
			else
			{
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
			}

			m_Gmap1.upload(map1);
			m_Gmap2.upload(map2);

			LOG(INFO)<<"Camera calibration initialized";

		}
	}

	return true;
}


void CamBase::setAttitude(double rollRad, double pitchRad, uint64_t timestamp)
{
	Point2f center(m_centerH, m_centerV);
	double deg = -rollRad * 180.0 * OneOvPI;

	m_rotRoll = getRotationMatrix2D(center, deg, m_isoScale);

	//TODO: add rot estimation

}

} /* namespace kai */
