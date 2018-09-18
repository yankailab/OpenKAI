/*
 * _DepthEdge.cpp
 *
 *  Created on: Sept 14, 2018
 *      Author: yankai
 */

#include "_DepthEdge.h"

namespace kai
{

_DepthEdge::_DepthEdge()
{
	m_pDV = NULL;
	m_wD = 160;
	m_hD = 100;
	m_vPos.init();

	m_vBBL.x = 0.1;
	m_vBBL.y = 0.2;
	m_vBBL.z = 0.3;
	m_vBBL.w = 0.6;

	m_vBBR.x = 0.7;
	m_vBBR.y = 0.2;
	m_vBBR.z = 0.9;
	m_vBBR.w = 0.6;

	m_vBBedge.x = 0.2;
	m_vBBedge.y = 0.2;
	m_vBBedge.z = 0.8;
	m_vBBedge.w = 0.8;

	m_dR = 0.0;
	m_dL = 0.0;

	m_minEdgeD = 1.0;

	m_rMin.x = 0.1;
	m_rMin.y = 1.0;
	m_rMin.z = 1.0;
	m_rMin.w = 0.0;

	m_rMax.x = 0.9;
	m_rMax.y = 15.0;
	m_rMax.z = 50.0;
	m_rMax.w = 360.0;

	m_nMorphOpen = 0;
	m_nMorphClose = 0;
	m_sobelK = 1;

}

_DepthEdge::~_DepthEdge()
{

}

bool _DepthEdge::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	KISSm(pK,wD);
	KISSm(pK,hD);
	KISSm(pK,nMorphOpen);
	KISSm(pK,nMorphClose);
	KISSm(pK,sobelK);
	KISSm(pK,minEdgeD);

	Kiss* pB;

	pB = pK->o("rangeX");
	if(!pB->empty())
	{
		pB->v("min", &m_rMin.x);
		pB->v("max", &m_rMax.x);
	}

	pB = pK->o("rangeY");
	if(!pB->empty())
	{
		pB->v("min", &m_rMin.y);
		pB->v("max", &m_rMax.y);
	}

	pB = pK->o("bbL");
	if(!pB->empty())
	{
		pB->v("x", &m_vBBL.x);
		pB->v("y", &m_vBBL.y);
		pB->v("z", &m_vBBL.z);
		pB->v("w", &m_vBBL.w);
	}

	pB = pK->o("bbR");
	if(!pB->empty())
	{
		pB->v("x", &m_vBBR.x);
		pB->v("y", &m_vBBR.y);
		pB->v("z", &m_vBBR.z);
		pB->v("w", &m_vBBR.w);
	}

	pB = pK->o("bbEdge");
	if(!pB->empty())
	{
		pB->v("x", &m_vBBedge.x);
		pB->v("y", &m_vBBedge.y);
		pB->v("z", &m_vBBedge.z);
		pB->v("w", &m_vBBedge.w);
	}

	//link
	string iName;

	iName = "";
	F_INFO(pK->v("_DepthVisionBase", &iName));
	m_pDV = (_DepthVisionBase*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pDV, iName + ": not found");

//	m_pf.init(m_hD,2);

	return true;
}

bool _DepthEdge::start(void)
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

void _DepthEdge::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		detect();

		this->autoFPSto();
	}
}

void _DepthEdge::detect(void)
{
	NULL_(m_pDV);
	Frame* pfDepth = m_pDV->Depth();
	IF_(pfDepth->bEmpty());

	Mat mD = *(pfDepth->resize(m_wD, m_hD).m());
	IF_(mD.empty());

	m_dL = m_pDV->d(&m_vBBL);
	m_dR = m_pDV->d(&m_vBBR);

	double pX = -1.0;
	double pY = -1.0;

	//find edge thr
	int thrBin;
	if(m_dL < m_dR)
	{
		thrBin = THRESH_BINARY;
		pY = m_dL;
	}
	else
	{
		thrBin = THRESH_BINARY_INV;
		pY = m_dR;
	}

	if(abs(m_dL - m_dR) > m_minEdgeD)
	{
		//find edge
		vInt4 iBB;
		Rect r;
		iBB.x = m_vBBedge.x * mD.cols;
		iBB.y = m_vBBedge.y * mD.rows;
		iBB.z = m_vBBedge.z * mD.cols;
		iBB.w = m_vBBedge.w * mD.rows;
		vInt42rect(iBB, r);

		cv::threshold(mD(r), mD, (m_dL + m_dR)*0.5, 1.0, thrBin);

		if(m_nMorphClose>0)
			cv::morphologyEx(mD, mD, MORPH_CLOSE, cv::Mat(), cv::Point(-1,-1), m_nMorphClose);

		if(m_nMorphOpen>0)
			cv::morphologyEx(mD, mD, MORPH_OPEN, cv::Mat(), cv::Point(-1,-1), m_nMorphOpen);

		cv::Sobel(mD, mD, mD.type(), 1, 0, m_sobelK, 1, 0, BORDER_DEFAULT);

		Mat mFlip;
		cv::flip(mD,mFlip,0);
		m_mF = mD.mul(mFlip);

		cv::reduce(m_mF, mD, 0, CV_REDUCE_SUM, -1);

		double vMax;
		Point iMax;
		cv::minMaxLoc(mD, NULL, &vMax, NULL, &iMax);

		pX = (double)iMax.x/(double)mD.cols;
	}

	//check varidity
	if(pX < m_rMin.x || pX > m_rMax.x)
		pX = -1.0;
	if(pY < m_rMin.y || pY > m_rMax.y)
		pY = -1.0;

	m_vPos.x = pX;
	m_vPos.y = pY;

	//	int i,j;
	//	for(i=0; i<m_mF.rows; i++)
	//	{
	//		double dEdge = 0;
	//		double iEdge = 0;
	//
	//		for(j=0; j<m_mF.cols-1; j++)
	//		{
	//			double d = abs((double)m_mF.at<float>(i,j+1) - (double)m_mF.at<float>(i,j));
	//			IF_CONT(d < dEdge);
	//
	//			dEdge = d;
	//			iEdge = j;
	//		}
	//
	//		m_pf.x(i, iEdge);
	//	}
	//
	//	m_pf.fit();
	//	double ovW = 1.0/(double)m_mF.cols;
	//	m_edgeTop = m_pf.yPoly(0) * ovW;
	//	m_edgeBottom = m_pf.yPoly(m_mF.rows-1) * ovW;
	//
	//	return (m_edgeTop + m_edgeBottom)*0.5;
}

vDouble4* _DepthEdge::pos(void)
{
	return &m_vPos;
}

bool _DepthEdge::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();

	pWin->tabNext();

	string msg = "Edge pos = (" +
			f2str(m_vPos.x) + ", " +
			f2str(m_vPos.y) + ", " +
			f2str(m_vPos.z) + ", " +
			f2str(m_vPos.w) + ")";
	pWin->addMsg(&msg);

	msg = "dL = " +	f2str(m_dL) + ", dR = " + f2str(m_dR);
	pWin->addMsg(&msg);

	pWin->tabPrev();

	circle(*pMat, Point(m_vPos.x * pMat->cols, 0.5 * pMat->rows),
			pMat->cols * pMat->rows * 0.00002, Scalar(0, 255, 255), 2);

	//bbox
	vInt4 iBB;
	Rect r;

	iBB.x = m_vBBL.x * pMat->cols;
	iBB.y = m_vBBL.y * pMat->rows;
	iBB.z = m_vBBL.z * pMat->cols;
	iBB.w = m_vBBL.w * pMat->rows;
	vInt42rect(iBB, r);
	rectangle(*pMat, r, Scalar(0,255,0), 1);

	iBB.x = m_vBBR.x * pMat->cols;
	iBB.y = m_vBBR.y * pMat->rows;
	iBB.z = m_vBBR.z * pMat->cols;
	iBB.w = m_vBBR.w * pMat->rows;
	vInt42rect(iBB, r);
	rectangle(*pMat, r, Scalar(0,255,0), 1);

	iBB.x = m_vBBedge.x * pMat->cols;
	iBB.y = m_vBBedge.y * pMat->rows;
	iBB.z = m_vBBedge.z * pMat->cols;
	iBB.w = m_vBBedge.w * pMat->rows;
	vInt42rect(iBB, r);
	rectangle(*pMat, r, Scalar(0,255,0), 2);

	if(!m_mF.empty())
	{
		imshow(*this->getName()+":Filter", m_mF);
	}

	return true;
}

bool _DepthEdge::cli(int& iY)
{
	IF_F(!this->_ThreadBase::cli(iY));

	string msg = "Edge pos = (" +
			f2str(m_vPos.x) + ", " +
			f2str(m_vPos.y) + ", " +
			f2str(m_vPos.z) + ", " +
			f2str(m_vPos.w) + ")";
	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	msg = "dL = " +	f2str(m_dL) + ", dR = " + f2str(m_dR);
	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	return true;
}

}
