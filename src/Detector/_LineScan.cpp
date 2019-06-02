/*
 * _LineScan.cpp
 *
 *  Created on: June 2, 2019
 *      Author: yankai
 */

#include "_LineScan.h"

namespace kai
{

_LineScan::_LineScan()
{
	m_pC = NULL;
	m_pD = NULL;
	m_dThr = 0.6;
	m_w = 0.2;
	m_dW = 0.1;
	m_nW = 0;
	m_minArea = 0.3;
}

_LineScan::~_LineScan()
{
}

bool _LineScan::init(void* pKiss)
{
	IF_F(!this->_DetectorBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	KISSm(pK,dThr);
	KISSm(pK,w);
	KISSm(pK,dW);
	KISSm(pK,minArea);

	m_nClass = 1;

	string iName = "";
	F_ERROR_F(pK->v("_DNNclassifer", &iName));
	m_pC = (_DNNclassifier*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pC, iName + " not found");

	F_ERROR_F(pK->v("_DepthVisionBase", &iName));
	m_pD = (_DepthVisionBase*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pD, iName + " not found");


	m_nW = ((1.0 - m_w) / m_dW) + 1;
	if (m_nW <= 0)
	{
		LOG_E("nW <=0");
		return false;
	}

	return true;
}

bool _LineScan::start(void)
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

void _LineScan::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		detect();
		m_obj.update();

		if(m_bGoSleep)
		{
			m_obj.m_pPrev->reset();
		}

		this->autoFPSto();
	}
}

int _LineScan::check(void)
{
	IF__(!m_pVision,-1);
	IF__(m_pVision->BGR()->bEmpty(),-1);
	IF__(!m_pD,-1);
	IF__(m_pD->Depth()->bEmpty(),-1);

	return 0;
}

void _LineScan::detect(void)
{
	IF_(check()<0);

	Mat mBGR;
	m_pVision->BGR()->m()->copyTo(mBGR);
	Mat mD;
	m_pD->Depth()->m()->copyTo(mD);
	vInt2 cs;
	m_pD->info(&cs, NULL, NULL);

	OBJECT o;
	for (int i = 0; i < m_nW; i++)
	{
		o.init();
		o.m_bb.x = i * m_dW;
		o.m_bb.z = o.m_bb.x + m_w;
		o.m_bb.y = 0.0;
		o.m_bb.w = 1.0;

		Rect rbb = o.getRect(cs);
		Mat mDr = mD(rbb);
		Mat mDin;
		cv::inRange(mDr, 1, m_dThr*255.0, mDin);
		int nP = cv::countNonZero(mDin);
		IF_CONT((float)nP/(float)rbb.area() < m_minArea);

		o.m_dist = (float)cv::mean(mDr, mDin).val[0] / 255.0;
		m_pC->classify(mBGR(rbb),&o);

		m_obj.add(&o);
	}
}

bool _LineScan::draw(void)
{
	IF_F(!this->_DetectorBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();

	if(!m_mR.empty())
	{
		imshow(*this->getName()+":Thr", m_mR);
	}

	return true;
}

}
