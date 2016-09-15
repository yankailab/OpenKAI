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

	F_INFO_(pConfig->o("APP")->v("presetDir", &presetDir));

	Config* pCam = pConfig->o(name);
	if(pCam->empty())return false;

	F_FATAL_F(pCam->v("width", &m_width));
	F_FATAL_F(pCam->v("height", &m_height));
	F_FATAL_F(pCam->v("angleV", &m_angleV));
	F_FATAL_F(pCam->v("angleH", &m_angleH));

	F_INFO_(pCam->v("isoScale", &m_isoScale));
	F_INFO_(pCam->v("bCalib", &m_bCalibration));
	F_INFO_(pCam->v("bGimbal", &m_bGimbal));
	F_INFO_(pCam->v("bFisheye", &m_bFisheye));

	F_INFO_(pCam->v("bCrop", &m_bCrop));
	if(m_bCrop!=0)
	{
		F_FATAL_F(pCam->v("cropX", &m_cropBB.x));
		F_FATAL_F(pCam->v("cropY", &m_cropBB.y));
		F_FATAL_F(pCam->v("cropW", &m_cropBB.width));
		F_FATAL_F(pCam->v("cropH", &m_cropBB.height));
	}

	if (pCam->v("calibFile", &calibFile))
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
