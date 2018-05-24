/*
 * _VisionBase.cpp
 *
 *  Created on: Aug 22, 2015
 *      Author: yankai
 */

#include "_VisionBase.h"

namespace kai
{

_VisionBase::_VisionBase()
{
	m_bOpen = false;
	m_type = unknownVision;
	m_w = 1280;
	m_h = 720;
	m_cW = 640;
	m_cH = 360;
	m_bGimbal = false;
	m_isoScale = 1.0;
	m_rotTime = 0;
	m_rotPrev = 0;
	m_fovW = 60;
	m_fovH = 60;
	m_bFlip = false;

	m_pHSV = NULL;
	m_pGray = NULL;

	m_bCalibration = false;
	m_bFisheye = false;
	m_bCrop = false;

	m_pTPP = new _ThreadBase();
}

_VisionBase::~_VisionBase()
{
	DEL(m_pTPP);
	DEL(m_pHSV);
	DEL(m_pGray);
}

bool _VisionBase::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;
	pK->m_pInst = this;

	KISSm(pK,w);
	KISSm(pK,h);
	m_cW = m_w / 2;
	m_cH = m_h / 2;

	KISSm(pK,fovW);
	KISSm(pK,fovH);
	KISSm(pK,bGimbal);
	KISSm(pK,isoScale);
	KISSm(pK,bFlip);

	bool bParam = false;
	F_INFO(pK->v("bGray", &bParam));
	if (bParam)
		m_pGray = new Frame();

	bParam = false;
	F_INFO(pK->v("bHSV", &bParam));
	if (bParam)
		m_pHSV = new Frame();

	KISSm(pK, bCrop);
	if (m_bCrop != 0)
	{
		F_INFO(pK->v("cropX", &m_cropBB.x));
		F_INFO(pK->v("cropY", &m_cropBB.y));
		F_INFO(pK->v("cropW", &m_cropBB.width));
		F_INFO(pK->v("cropH", &m_cropBB.height));
	}

	KISSm(pK, bCalibration);
	KISSm(pK, bFisheye);
	string calibFile = "";
	F_INFO(pK->v("calibFile", &calibFile));

	if (!calibFile.empty())
	{
		FileStorage fs(calibFile, FileStorage::READ);
		if (!fs.isOpened())
		{
			LOG_E("Calibration file not found: " + calibFile);
			m_bCalibration = false;
		}
		else
		{
			Mat mCamera;
			Mat mDistCoeffs;

			fs["camera_matrix"] >> mCamera;
			fs["distortion_coefficients"] >> mDistCoeffs;
			fs.release();

			Mat map1, map2;
			cv::Size imSize(m_w, m_h);

			if (m_bFisheye)
			{
				Mat newCamMat;
				fisheye::estimateNewCameraMatrixForUndistortRectify(mCamera,
						mDistCoeffs, imSize, Matx33d::eye(), newCamMat, 1);
				fisheye::initUndistortRectifyMap(mCamera, mDistCoeffs,
						Matx33d::eye(), newCamMat, imSize,
						CV_32F/*CV_16SC2*/, map1, map2);
			}
			else
			{
				m_K = getOptimalNewCameraMatrix(mCamera, mDistCoeffs, imSize, 1, imSize, 0);
				initUndistortRectifyMap(mCamera, mDistCoeffs, Mat(), m_K, imSize, CV_32FC1, map1, map2);
			}

			m_fBGR.setRemap(map1,map2);

			m_bCalibration = true;
			LOG_I("camera calibration enabled");
		}
	}

	m_bOpen = false;
	return true;
}

void _VisionBase::postProcess(void)
{
	if (m_bCalibration)
		m_fBGR = m_fBGR.remap();

	if (m_bGimbal)
		m_fBGR = m_fBGR.warpAffine(m_rotRoll);

	if (m_bFlip)
		m_fBGR = m_fBGR.flip(-1);

	if (m_bCrop)
		m_fBGR = m_fBGR.crop(m_cropBB);

	if (m_pGray)
		*m_pGray = m_fBGR.cvtColor(CV_BGR2GRAY);

	if (m_pHSV)
		*m_pHSV = m_fBGR.cvtColor(CV_BGR2HSV);
}

Frame* _VisionBase::BGR(void)
{
	return &m_fBGR;
}

Frame* _VisionBase::HSV(void)
{
	return m_pHSV;
}

Frame* _VisionBase::Gray(void)
{
	return m_pGray;
}

void _VisionBase::info(vInt2* pSize, vInt2* pCenter, vInt2* pAngle)
{
	if(pSize)
	{
		pSize->x = m_w;
		pSize->y = m_h;
	}

	if(pCenter)
	{
		pCenter->x = m_cW;
		pCenter->y = m_cH;
	}

	if(pAngle)
	{
		pAngle->x = m_fovW;
		pAngle->y = m_fovH;
	}
}

void _VisionBase::setAttitude(double rollRad, double pitchRad, uint64_t timestamp)
{
	Point2f center(m_cW, m_cH);
	double deg = -rollRad * 180.0 * OneOvPI;

	m_rotRoll = getRotationMatrix2D(center, deg, m_isoScale);
	//TODO: add rot estimation

}

Mat* _VisionBase::K(void)
{
	return &m_K;
}

VISION_TYPE _VisionBase::getType(void)
{
	return m_type;
}

bool _VisionBase::isOpened(void)
{
	return m_bOpen;
}

bool _VisionBase::draw(void)
{
	NULL_F(m_pWindow);
	Frame* pFrame = ((Window*)m_pWindow)->getFrame();
	IF_F(m_fBGR.bEmpty());

	pFrame->copy(m_fBGR);

	return this->_ThreadBase::draw();
}

}
