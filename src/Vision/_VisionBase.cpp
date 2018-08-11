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
	m_type = vision_unknown;
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
	m_bPers = false;
	m_bShowPers = false;
	m_vSize.init();
	m_pPers = NULL;

	m_pTPP = new _ThreadBase();
}

_VisionBase::~_VisionBase()
{
	DEL(m_pTPP);
	DEL(m_pHSV);
	DEL(m_pGray);
	DEL(m_pPers);
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

	KISSm(pK,bPers);
	if(m_bPers)
	{
		KISSm(pK,bShowPers);

		F_INFO(pK->v("persLTx", &m_persLT.x));
		F_INFO(pK->v("persLTy", &m_persLT.y));
		F_INFO(pK->v("persLBx", &m_persLB.x));
		F_INFO(pK->v("persLBy", &m_persLB.y));
		F_INFO(pK->v("persRTx", &m_persRT.x));
		F_INFO(pK->v("persRTy", &m_persRT.y));
		F_INFO(pK->v("persRBx", &m_persRB.x));
		F_INFO(pK->v("persRBy", &m_persRB.y));
		F_INFO(pK->v("persW", &m_persSize.x));
		F_INFO(pK->v("persH", &m_persSize.y));

		m_mPers = Mat(Size(m_persSize.x, m_persSize.y), CV_8UC3);
		m_pPers = new Frame();
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

	if (m_bPers)
	{
		Size s = m_fBGR.size();
		if (m_vSize.x != s.width || m_vSize.y != s.height)
		{
			m_vSize.x = s.width;
			m_vSize.y = s.height;
			updateVisionSize();
		}

		if(!m_mPersT.empty())
		{
			cv::warpPerspective(*m_fBGR.m(),
								m_mPers,
								m_mPersT,
								m_mPers.size(),
								cv::INTER_LINEAR);
			*m_pPers = m_mPers;
		}
	}
}

void _VisionBase::updateVisionSize(void)
{
	Point2f LT = Point2f((float) (m_persLT.x * m_vSize.x),
						 (float) (m_persLT.y * m_vSize.y));
	Point2f LB = Point2f((float) (m_persLB.x * m_vSize.x),
						 (float) (m_persLB.y * m_vSize.y));
	Point2f RT = Point2f((float) (m_persRT.x * m_vSize.x),
						 (float) (m_persRT.y * m_vSize.y));
	Point2f RB = Point2f((float) (m_persRB.x * m_vSize.x),
						 (float) (m_persRB.y * m_vSize.y));

	//LT, LB, RB, RT
	Point2f ptsFrom[] = { LT, LB, RB, RT };
	Point2f ptsTo[] =
	{ cv::Point2f(0, 0),
	  cv::Point2f(0, (float) m_persSize.y),
	  cv::Point2f((float) m_persSize.x, (float) m_persSize.y),
	  cv::Point2f((float) m_persSize.x, 0) };

	m_mPersT = getPerspectiveTransform(ptsFrom, ptsTo);
	m_mPersInvT = getPerspectiveTransform(ptsTo, ptsFrom);
}

Frame* _VisionBase::Pers(void)
{
	return m_pPers;
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

void _VisionBase::info(vInt2* pSize, vInt2* pCenter, vDouble2* pAngle)
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

vInt2 _VisionBase::getSize(void)
{
	vInt2 s;
	s.x = m_w;
	s.y = m_h;
	return s;
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

	if(!m_fBGR.bEmpty())
	{
		pFrame->copy(m_fBGR);
	}

	if(!m_mPers.empty() && m_bShowPers)
	{
		imshow(*this->getName()+":Perspective", m_mPers);
	}

	return this->_ThreadBase::draw();
}

}
