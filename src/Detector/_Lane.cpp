/*
 * _Lane.cpp
 *
 *  Created on: Mar 5, 2018
 *      Author: yankai
 */

#include "_Lane.h"

namespace kai
{

_Lane::_Lane()
{
	m_pVision = NULL;

	m_roiLT.x = 0.2;
	m_roiLT.y = 0.5;
	m_roiLB.x = 0.0;
	m_roiLB.y = 1.0;
	m_roiRT.x = 0.8;
	m_roiRT.y = 0.5;
	m_roiRB.x = 1.0;
	m_roiRB.y = 1.0;

	m_sizeOverhead.x = 400;
	m_sizeOverhead.y = 300;
	m_vSize.init();

	m_nFilter = 0;
	m_bDrawOverhead = false;
	m_bDrawFilter = false;
}

_Lane::~_Lane()
{
}

bool _Lane::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	KISSm(pK, bDrawOverhead);
	KISSm(pK, bDrawFilter);

	F_INFO(pK->v("roiLTx", &m_roiLT.x));
	F_INFO(pK->v("roiLTy", &m_roiLT.y));
	F_INFO(pK->v("roiLBx", &m_roiLB.x));
	F_INFO(pK->v("roiLBy", &m_roiLB.y));
	F_INFO(pK->v("roiRTx", &m_roiRT.x));
	F_INFO(pK->v("roiRTy", &m_roiRT.y));
	F_INFO(pK->v("roiRBx", &m_roiRB.x));
	F_INFO(pK->v("roiRBy", &m_roiRB.y));
	F_INFO(pK->v("overheadW", &m_sizeOverhead.x));
	F_INFO(pK->v("overheadH", &m_sizeOverhead.y));

	m_mOverhead = Mat(Size(m_sizeOverhead.x, m_sizeOverhead.y), CV_8UC3);

	//filter parameters
	Kiss** pItrFilter = pK->getChildItr();
	m_nFilter = 0;
	while (pItrFilter[m_nFilter])
	{
		IF_F(m_nFilter >= N_LANE_FILTER);
		Kiss* pFK = pItrFilter[m_nFilter];

		LANE_FILTER* pF = &m_pFilter[m_nFilter];
		F_INFO(pFK->v("iColorSpace", &pF->m_iColorSpace));
		F_INFO(pFK->v("iChannel", &pF->m_iChannel));
		F_INFO(pFK->v("nTile", &pF->m_nTile));
		F_INFO(pFK->v("thr", &pF->m_thr));
		F_INFO(pFK->v("clipLim", &pF->m_clipLim));
		pF->init();

		m_nFilter++;
	}

	int nAvr=0;
	F_INFO(pK->v("nAvr", &nAvr));
	int nMed=0;
	F_INFO(pK->v("nMed", &nMed));

	m_laneL.init(m_sizeOverhead.y,nAvr,nMed);
	m_laneR.init(m_sizeOverhead.y,nAvr,nMed);

	return true;
}

bool _Lane::link(void)
{
	IF_F(!this->_ThreadBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	string iName = "";
	F_ERROR_F(pK->v("_VisionBase", &iName));
	m_pVision = (_VisionBase*) (pK->root()->getChildInstByName(&iName));
	NULL_Fl(m_pVision, "_VisionBase is NULL");

	return true;
}

bool _Lane::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _Lane::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		detect();

		this->autoFPSto();
	}
}

void _Lane::detect(void)
{
	NULL_(m_pVision);
	Mat* pInput = m_pVision->bgr()->getCMat();
	IF_(pInput->empty());

	//Warp transform to get overhead view
	if (m_vSize.x != pInput->cols || m_vSize.y != pInput->rows)
	{
		m_vSize.x = pInput->cols;
		m_vSize.y = pInput->rows;
		updateVisionSize();
	}

	cv::warpPerspective(*pInput, m_mOverhead, m_mPerspective,
			m_mOverhead.size(), cv::INTER_LINEAR);

	//Filter image to get binary view
	filterBin();

	findLane();

}

void _Lane::findLane(void)
{
	double vMax;
	Point iMax;
	Rect ROI;
	ROI.width = m_sizeOverhead.x / 2;
	ROI.height = 1;

	for(int i=0; i<m_sizeOverhead.y; i++)
	{
		ROI.y = i;

		//L lane
		ROI.x = 0;
		cv::minMaxLoc(m_mBin(ROI),NULL,&vMax,NULL,&iMax);
		if(vMax > 0)
		{
			m_laneL.input(i,iMax.x);
		}

		//R lane
		ROI.x = ROI.width;
		cv::minMaxLoc(m_mBin(ROI),NULL,&vMax,NULL,&iMax);
		if(vMax > 0)
		{
			m_laneR.input(i,ROI.x + iMax.x);
		}
	}
}

void _Lane::filterBin(void)
{
	Mat mAll = Mat::zeros(m_mOverhead.size(), CV_8UC1);

	for(int i=0; i<m_nFilter; i++)
	{
		mAll += m_pFilter[i].filter(m_mOverhead);
	}

	cv::normalize(mAll, m_mBin, 0, 255, cv::NORM_MINMAX);
}

void _Lane::updateVisionSize(void)
{
	Point2f LT = Point2f((float) (m_roiLT.x * m_vSize.x),
						 (float) (m_roiLT.y * m_vSize.y));
	Point2f LB = Point2f((float) (m_roiLB.x * m_vSize.x),
						 (float) (m_roiLB.y * m_vSize.y));
	Point2f RT = Point2f((float) (m_roiRT.x * m_vSize.x),
						 (float) (m_roiRT.y * m_vSize.y));
	Point2f RB = Point2f((float) (m_roiRB.x * m_vSize.x),
						 (float) (m_roiRB.y * m_vSize.y));

	//LT, LB, RB, RT
	Point2f ptsFrom[] = { LT, LB, RB, RT };
	Point2f ptsTo[] =
	{ cv::Point2f(0, 0),
	  cv::Point2f(0, (float) m_sizeOverhead.y),
	  cv::Point2f((float) m_sizeOverhead.x, (float) m_sizeOverhead.y),
	  cv::Point2f((float) m_sizeOverhead.x, 0) };

	m_mPerspective = getPerspectiveTransform(ptsFrom, ptsTo);
}

bool _Lane::draw(void)
{
	IF_F(!this->_ThreadBase::draw());

	Window* pWin = (Window*) this->m_pWindow;
	Frame* pFrame = pWin->getFrame();
	Mat* pMat = pFrame->getCMat();
	IF_F(pMat->empty());

	if(m_bDrawOverhead && !m_mOverhead.empty())
	{
		Mat mO = m_mOverhead.clone();
		int i;

		for (i = 0; i < m_sizeOverhead.y; i++)
		{
			//L lane
			circle(mO,
				   Point(m_laneL.v(i), i),
				   1, Scalar(255, 0, 0), 1);

			//R lane
			circle(mO,
				   Point(m_laneR.v(i)+1, i),
				   1, Scalar(0, 0, 255), 1);
		}

		imshow("Overhead", mO);
	}

	if(m_bDrawOverhead && !m_mBin.empty())
	{
		imshow("Bin", m_mBin);
	}


//	pWin->addMsg();
//	Mat output;
//	vector<Point> m_vPolyPoints;
//
//	// Create the transparent polygon for a better visualization of the lane
//	pMat->copyTo(output);
//	m_vPolyPoints.push_back(m_vLane[2]);
//	m_vPolyPoints.push_back(m_vLane[0]);
//	m_vPolyPoints.push_back(m_vLane[1]);
//	m_vPolyPoints.push_back(m_vLane[3]);
//	fillConvexPoly(output, m_vPolyPoints, Scalar(0, 255, 0), CV_AA, 0);
//	cv::addWeighted(output, 0.1, *pMat, 1.0, 0, *pMat);
//
//	// Plot both lines of the lane boundary
//	line(*pMat, m_vLane[0], m_vLane[1], Scalar(0, 255, 255), 2, CV_AA);
//	line(*pMat, m_vLane[2], m_vLane[3], Scalar(0, 255, 255), 2, CV_AA);

	return true;
}

}
