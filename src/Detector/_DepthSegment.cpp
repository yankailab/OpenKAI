/*
 * _DepthSegment.cpp
 *
 *  Created on: May 29, 2019
 *      Author: yankai
 */

#include "_DepthSegment.h"

namespace kai
{

_DepthSegment::_DepthSegment()
{
	m_rL = 200;
	m_rH = 255;
	m_rD = 0.05;
}

_DepthSegment::~_DepthSegment()
{
}

bool _DepthSegment::init(void* pKiss)
{
	IF_F(!this->_DetectorBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	KISSm(pK,rL);
	KISSm(pK,rH);
	KISSm(pK,rD);
	m_nClass = 1;

	return true;
}

bool _DepthSegment::start(void)
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

void _DepthSegment::update(void)
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

int _DepthSegment::check(void)
{
	IF__(!m_pVision,-1);
	IF__(m_pVision->BGR()->bEmpty(),-1);

	return 0;
}

void _DepthSegment::detect(void)
{
	IF_(check()<0);

	Mat mD;
	float d = 255.0/(m_rH - m_rL);
	m_pVision->BGR()->m()->convertTo(mD, CV_8UC1, d, -m_rL*d);

	vInt2 cs;
	m_pVision->info(&cs, NULL, NULL);

	vector< vector< Point > > vvC;
	int dR = m_rD * 255.0;
	for(int r=0; r<255; r+=dR)
	{
		cv::inRange(mD, 0, r, m_mR);
		findContours(m_mR, vvC, RETR_EXTERNAL, CHAIN_APPROX_NONE);

		OBJECT o;
		vector<Point> vPoly;
		for (unsigned int i=0; i<vvC.size(); i++)
		{
			vPoly.clear();
			approxPolyDP( vvC[i], vPoly, 3, true );
			Rect rBB = boundingRect(vPoly);

			o.init();
			o.m_tStamp = m_tStamp;
			o.setBB(rBB, cs);
			o.setTopClass(0, o.area());

			add(&o);
		}
	}
}

bool _DepthSegment::draw(void)
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
