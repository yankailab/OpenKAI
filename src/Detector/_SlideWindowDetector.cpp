/*
 * _SlideWindowDetector.cpp
 *
 *  Created on: June 2, 2019
 *      Author: yankai
 */

#include "_SlideWindowDetector.h"

namespace kai
{

_SlideWindowDetector::_SlideWindowDetector()
{
	m_pC = NULL;
	m_pD = NULL;
	m_w = 0.2;
	m_dW = 0.1;
	m_nW = 0;
	m_maxD = 0.6;
	m_minArea = 0.3;
	m_nLevel = 10;
	m_dRange.init();
}

_SlideWindowDetector::~_SlideWindowDetector()
{
}

bool _SlideWindowDetector::init(void* pKiss)
{
	IF_F(!this->_DetectorBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	KISSm(pK,w);
	KISSm(pK,dW);
	KISSm(pK,maxD);
	KISSm(pK,minArea);
	KISSm(pK,nLevel);
	pK->v("dFrom",&m_dRange.x);
	pK->v("dTo",&m_dRange.y);

	string iName = "";
	F_ERROR_F(pK->v("_DNNclassifier", &iName));
	m_pC = (_DNNclassifier*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pC, iName + " not found");

	F_ERROR_F(pK->v("_DepthVisionBase", &iName));
	m_pD = (_DepthVisionBase*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pD, iName + " not found");

	m_nW = 0;
	while(m_nW*m_dW + m_w < 1.0)m_nW++;
	if (m_nW <= 0)
	{
		LOG_E("nW <=0");
		return false;
	}

	return true;
}

bool _SlideWindowDetector::start(void)
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

void _SlideWindowDetector::update(void)
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

int _SlideWindowDetector::check(void)
{
	IF__(!m_pVision,-1);
	IF__(m_pVision->BGR()->bEmpty(),-1);
	IF__(!m_pD,-1);
	IF__(m_pD->BGR()->bEmpty(),-1);

	return 0;
}

void _SlideWindowDetector::detect(void)
{
	IF_(check()<0);

	m_pVision->BGR()->m()->copyTo(m_mBGR);
	m_pD->BGR()->m()->copyTo(m_mD);
	cv::inRange(m_mD, 1, m_maxD*255.0, m_mDin);

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
		Mat mDinR = m_mDin(rbb);
		int nP = cv::countNonZero(mDinR);
		IF_CONT((float)nP/(float)rbb.area() < m_minArea);

//		o.m_dist = (hist(m_mD(rbb), 0, 255, m_nLevel, m_minArea)/255.0) * m_dRange.len() + m_dRange.x;
		o.m_dist = ((float)cv::mean(m_mD(rbb), mDinR).val[0]/255.0) * m_dRange.len() + m_dRange.x;
		if(m_pC->classify(m_mBGR(rbb),&o))
		{
			m_obj.add(&o);
		}
	}
}

bool _SlideWindowDetector::draw(void)
{
	IF_F(!this->_DetectorBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();

	IF_T(!m_bDebug);

	if(!m_mBGR.empty())
		imshow(*this->getName()+":BGR", m_mBGR);

	if(!m_mD.empty())
		imshow(*this->getName()+":Depth", m_mD);

	if(!m_mDin.empty())
		imshow(*this->getName()+":InRange", m_mDin);

	return true;
}

}
