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
	m_width = 0;
	m_height = 0;
	m_centerH = 0;
	m_centerV = 0;
	m_bCalibration = false;
	m_bGimbal = false;
	m_bFisheye = false;
	m_isoScale = 1.0;
	m_rotTime = 0;
	m_rotPrev = 0;
	m_angleH = 0;
	m_angleV = 0;
	m_bCrop = 0;
}

CamBase::~CamBase()
{
}

bool CamBase::setup(Config* pConfig, string name)
{
	if(pConfig==NULL)return false;
	if(name.empty())return false;

	string presetDir = "";
	string calibFile;

	CHECK_INFO(pConfig->obj("APP")->var("presetDir", &presetDir));

	Config* pCam = pConfig->obj(name);
	if(pCam->empty())return false;

	CHECK_FATAL(pCam->var("width", &m_width));
	CHECK_FATAL(pCam->var("height", &m_height));
	CHECK_FATAL(pCam->var("angleV", &m_angleV));
	CHECK_FATAL(pCam->var("angleH", &m_angleH));

	CHECK_INFO(pCam->var("isoScale", &m_isoScale));
	CHECK_INFO(pCam->var("bCalib", &m_bCalibration));
	CHECK_INFO(pCam->var("bGimbal", &m_bGimbal));
	CHECK_INFO(pCam->var("bFisheye", &m_bFisheye));

	CHECK_INFO(pCam->var("bCrop", &m_bCrop));
	if(m_bCrop!=0)
	{
		CHECK_FATAL(pCam->var("cropX", &m_cropBB.x));
		CHECK_FATAL(pCam->var("cropY", &m_cropBB.y));
		CHECK_FATAL(pCam->var("cropW", &m_cropBB.width));
		CHECK_FATAL(pCam->var("cropH", &m_cropBB.height));
	}

	if (pCam->var("calibFile", &calibFile))
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

bool CamBase::openCamera(void)
{
	return false;
}

GpuMat* CamBase::readFrame(void)
{
	return NULL;
}

GpuMat* CamBase::getDepthFrame(void)
{
	return NULL;
}

void CamBase::release(void)
{

}

void CamBase::setAttitude(double rollRad, double pitchRad, uint64_t timestamp)
{
	Point2f center(m_centerH, m_centerV);
	double deg = -rollRad * 180.0 * OneOvPI;

	m_rotRoll = getRotationMatrix2D(center, deg, m_isoScale);

	//TODO: add rot estimation

}

int CamBase::getType(void)
{
	return m_type;
}


} /* namespace kai */
