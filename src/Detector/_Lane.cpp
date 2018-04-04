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

	m_w = 0.1;
	m_h = 0.1;
	m_dW = 0.05;
	m_dH = 0.05;
	m_nBlockX = ((1.0 - m_w) / m_dW) + 1;
	m_nBlockY = ((1.0 - m_h) / m_dH) + 1;
	m_nBlock = m_nBlockX * m_nBlockY;
	m_pBlock = NULL;

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

	m_tileClahe = 8;
	m_clipLimLAB = 2.0;
	m_clipLimHSV = 6.0;
	m_clipLimHLS = 2.0;
	m_thrLAB = 150;
	m_thrHSV = 220;
	m_thrHLS = 210;

	m_nFilter = 0;

	m_bDrawOverhead = false;
	m_bDrawBin = false;
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
	KISSm(pK, bDrawBin);

	KISSm(pK, w);
	KISSm(pK, h);
	KISSm(pK, dW);
	KISSm(pK, dH);
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

	KISSm(pK, tileClahe);
	KISSdm(pK, clipLimLAB);
	KISSdm(pK, clipLimHSV);
	KISSdm(pK, clipLimHLS);
	KISSm(pK, thrLAB);
	KISSm(pK, thrHSV);
	KISSm(pK, thrHLS);

	m_pClaheLAB = cv::createCLAHE(m_clipLimLAB, Size(m_tileClahe, m_tileClahe));
	m_pClaheHSV = cv::createCLAHE(m_clipLimHSV, Size(m_tileClahe, m_tileClahe));
	m_pClaheHLS = cv::createCLAHE(m_clipLimHLS, Size(m_tileClahe, m_tileClahe));

	//create detection blocks
	m_nBlockX = ((1.0 - m_w) / m_dW) + 1;
	m_nBlockY = ((1.0 - m_h) / m_dH) + 1;
	if (m_nBlockX <= 0 || m_nBlockY <= 0)
	{
		LOG_E("nW <=0 || nH <= 0");
		return false;
	}

	m_nBlock = m_nBlockX * m_nBlockY;
	m_pBlock = new LANE_BLOCK[m_nBlock];

	int i, j;
	for (i = 0; i < m_nBlockY; i++)
	{
		for (j = 0; j < m_nBlockX; j++)
		{
			LANE_BLOCK* pB = &m_pBlock[i * m_nBlockY + j];
			pB->init();
			pB->m_fROI.x = j * m_dW;
			pB->m_fROI.z = pB->m_fROI.x + m_w;
			pB->m_fROI.y = i * m_dH;
			pB->m_fROI.w = pB->m_fROI.y + m_h;
		}
	}

	int nAvr=0;
	F_INFO(pK->v("nAvr", &nAvr));
	int nMed=0;
	F_INFO(pK->v("nMed", &nMed));

	m_laneL.init(nAvr,nMed);
	m_laneR.init(nAvr,nMed);

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
	getOverheadBin();

	//Update block occupancy
	updateBlock();

	//Update the brightest block in each lane detection
	updateLaneBlock();

	//Update lane
	updateLane();

}

void _Lane::updateLane()
{




}

void _Lane::updateLaneBlock(void)
{
	int i,j;
	int iX;
	int vX;

	for (i = 0; i < m_nBlockY; i++)
	{
		int iY = i*m_nBlockY;

		//L lane
		iX = 0;
		vX = 0;
		for (j = 0; j < m_nBlockX; j++)
		{
			LANE_BLOCK* pB = &m_pBlock[iY + j];
			IF_CONT(pB->m_v <= vX);

			vX = pB->m_v;
			iX = j;
		}

		m_laneL.m_laneDet[i].input(iX * m_dW + m_w * 0.5);

		//R lane
		iX = m_nBlockX - 1;
		vX = 0;
		for (j = m_nBlockX - 1; j >= 0; j--)
		{
			LANE_BLOCK* pB = &m_pBlock[iY + j];
			IF_CONT(pB->m_v <= vX);

			vX = pB->m_v;
			iX = j;
		}

		m_laneR.m_laneDet[i].input(iX * m_dW + m_w * 0.5);
	}
}

void _Lane::updateBlock(void)
{
	IF_(m_mBin.empty());

	int i, j;
	for (i = 0; i < m_nBlockY; i++)
	{
		int iY = i*m_nBlockY;

		for (j = 0; j < m_nBlockX; j++)
		{
			LANE_BLOCK* pB = &m_pBlock[iY + j];

			pB->m_v = cv::countNonZero(m_mBin(pB->m_iROI));
		}
	}
}

void _Lane::getOverheadBin(void)
{
	vector<Mat> vMat(3);
	Mat tMat;
	Mat tMatBin;
	Mat mAll;

	cv::cvtColor(m_mOverhead, m_mLAB, CV_BGR2Lab);
	split(m_mLAB, vMat);
	m_pClaheLAB->apply(vMat[2], tMat);
	cv::threshold(tMat, tMatBin, m_thrLAB, 1, cv::THRESH_BINARY);
	mAll = tMatBin;

	cv::cvtColor(m_mOverhead, m_mHSV, CV_BGR2HSV);
	split(m_mHSV, vMat);
	m_pClaheHSV->apply(vMat[2], tMat);
	cv::threshold(tMat, tMatBin, m_thrHSV, 1, cv::THRESH_BINARY);
	mAll += tMatBin;

	cv::cvtColor(m_mOverhead, m_mHLS, CV_BGR2HLS);
	split(m_mHLS, vMat);
	m_pClaheHLS->apply(vMat[1], tMat);
	cv::threshold(tMat, tMatBin, m_thrHLS, 1, cv::THRESH_BINARY);
	mAll += tMatBin;

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

	int i, j;
	for (i = 0; i < m_nBlockY; i++)
	{
		for (j = 0; j < m_nBlockX; j++)
		{
			m_pBlock[i * m_nBlockY + j].f2iROI(m_sizeOverhead);
		}
	}

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
		Mat mO = m_mOverhead;
		int i;

		for (i = 0; i < m_nBlockY; i++)
		{
			int pY = mO.rows * (i * m_dH + m_h*0.5);

			//L lane
			circle(mO,
				   Point(mO.cols * m_laneL.m_laneDet[i].v(), pY),
				   2, Scalar(255, 0, 0), 3);

			//R lane
			circle(mO,
				   Point(mO.cols * m_laneR.m_laneDet[i].v(), pY),
				   2, Scalar(0, 0, 255), 3);
		}

		imshow("Overhead", m_mOverhead);
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
