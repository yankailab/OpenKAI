/*
 * _Thermal.cpp
 *
 *  Created on: Jan 18, 2019
 *      Author: yankai
 */

#include "_Thermal.h"

namespace kai
{

_Thermal::_Thermal()
{
	m_rL = 200;
	m_rU = 255;
}

_Thermal::~_Thermal()
{
}

bool _Thermal::init(void* pKiss)
{
	IF_F(!this->_DetectorBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	pK->v<double>("rL",&m_rL);
	pK->v<double>("rU",&m_rU);

	m_nClass = 1;
	return true;
}

bool _Thermal::start(void)
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

void _Thermal::update(void)
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

int _Thermal::check(void)
{
	IF__(!m_pVision,-1);
	IF__(m_pVision->BGR()->bEmpty(),-1);

	return 0;
}

void _Thermal::detect(void)
{
	IF_(check()<0);

	Mat mBGR = *(m_pVision->BGR()->m());
	Mat mGray;
	cv::cvtColor(mBGR, mGray, COLOR_BGR2GRAY);
	cv::inRange(mGray, m_rL, m_rU, m_mR);

	vector< vector< Point > > vvContours;
	findContours(m_mR, vvContours, RETR_EXTERNAL, CHAIN_APPROX_NONE);

	vInt2 cs;
	m_pVision->info(&cs, NULL, NULL);

	OBJECT o;
	vector<Point> vPoly;
	for (unsigned int i=0; i<vvContours.size(); i++)
	{
		vPoly.clear();
		approxPolyDP( vvContours[i], vPoly, 3, true );
		Rect r = boundingRect(vPoly);

		o.init();
		o.m_tStamp = m_tStamp;
		o.setBB(convertBB<vFloat4>(r));
		o.normalizeBB(cs);
		o.setTopClass(0, o.area());

		add(&o);
		LOG_I("ID: "+ i2str(o.m_topClass));
	}
}

void _Thermal::draw(void)
{
	this->_DetectorBase::draw();
	if(!m_mR.empty())
	{
		imshow(*this->getName()+":Thr", m_mR);
	}
}

}
