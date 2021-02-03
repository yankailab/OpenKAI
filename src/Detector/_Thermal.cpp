/*
 * _Thermal.cpp
 *
 *  Created on: Jan 18, 2019
 *      Author: yankai
 */

#include "_Thermal.h"

#ifdef USE_OPENCV

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

bool _Thermal::init(void *pKiss)
{
	IF_F(!this->_DetectorBase::init(pKiss));
	Kiss *pK = (Kiss*) pKiss;

	pK->v<double>("rL", &m_rL);
	pK->v<double>("rU", &m_rU);

	m_nClass = 1;
	return true;
}

bool _Thermal::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdate, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _Thermal::update(void)
{
	while(m_pT->bRun())
	{
		m_pT->autoFPSfrom();

		if (check() >= 0)
		{
			detect();

			if (m_bGoSleep)
				m_pU->m_pPrev->clear();
		}

		m_pT->autoFPSto();
	}
}

int _Thermal::check(void)
{
	NULL__(m_pU, -1);
	NULL__(m_pV, -1);
	IF__(m_pV->BGR()->bEmpty(), -1);

	return 0;
}

void _Thermal::detect(void)
{
	Mat mBGR = *(m_pV->BGR()->m());
	Mat mGray;
	cv::cvtColor(mBGR, mGray, COLOR_BGR2GRAY);
	cv::inRange(mGray, m_rL, m_rU, m_mR);

	vector<vector<Point> > vvContours;
	findContours(m_mR, vvContours, RETR_EXTERNAL, CHAIN_APPROX_NONE);

	_Object o;
	vector<Point> vPoly;
	for (unsigned int i = 0; i < vvContours.size(); i++)
	{
		vPoly.clear();
		approxPolyDP(vvContours[i], vPoly, 3, true);
		Rect r = boundingRect(vPoly);

		o.init();
		o.m_tStamp = m_tStamp;
		o.setBB2D(rect2BB < vFloat4 > (r));
		o.scale(m_mR.cols, m_mR.rows);
		o.setTopClass(0, o.area());

		m_pU->add(o);
		LOG_I("ID: " + i2str(o.getTopClass()));
	}

	m_pU->updateObj();
}

void _Thermal::draw(void)
{
	this->_DetectorBase::draw();

	IF_(!m_bDebug);

	if (!m_mR.empty())
	{
		imshow(*this->getName() + ":Thr", m_mR);
	}
}

}
#endif
