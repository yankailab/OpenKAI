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
	m_rL = 0.0;
	m_rH = 1.0;
	m_rD = 0.1;
	m_rArea = 0.8;
}

_DepthSegment::~_DepthSegment()
{
}

bool _DepthSegment::init(void* pKiss)
{
	IF_F(!this->_DetectorBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	pK->v<float>("rL",&m_rL);
	pK->v<float>("rH",&m_rH);
	pK->v<float>("rD",&m_rD);
	pK->v<float>("rArea",&m_rArea);

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
		updateObj();

		if(m_bGoSleep)
		{
			m_pPrev->reset();
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
	m_pVision->BGR()->m()->copyTo(mD);
	vInt2 cs;
	m_pVision->info(&cs, NULL, NULL);

	vector< vector< Point > > vvC;
	for(float r=m_rL; r<m_rH; r+=m_rD)
	{
		cv::inRange(mD, 1, (r-m_rL)*255.0+2, m_mR);
		findContours(m_mR, vvC, RETR_EXTERNAL, CHAIN_APPROX_NONE);

		OBJECT o;
		vector<Point> vPoly;
		float rArea = 0.0;
		for (unsigned int i=0; i<vvC.size(); i++)
		{
			vPoly.clear();
			approxPolyDP( vvC[i], vPoly, 3, true );
			Rect rBB = boundingRect(vPoly);

			o.init();
			o.m_tStamp = m_tStamp;
			o.setBB(convertBB<vFloat4>(rBB));
			o.normalizeBB(cs);
			o.setTopClass(0, o.area());
			o.m_dist = r;

			//TODO: classify

			add(&o);
			rArea += o.area();
		}

		if(rArea > m_rArea)break;
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
