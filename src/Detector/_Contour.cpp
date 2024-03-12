/*
 * _Contour.cpp
 *
 *  Created on: Jan 18, 2019
 *      Author: yankai
 */

#include "_Contour.h"

namespace kai
{

	_Contour::_Contour()
	{
	}

	_Contour::~_Contour()
	{
	}

	bool _Contour::init(void *pKiss)
	{
		IF_F(!this->_DetectorBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		return true;
	}

	bool _Contour::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _Contour::check(void)
	{
		NULL__(m_pU, -1);
		NULL__(m_pV, -1);
		IF__(m_pV->getFrameRGB()->bEmpty(), -1);

		return this->_DetectorBase::check();
	}

	void _Contour::update(void)
	{
		while (m_pT->bThread())
		{
			m_pT->autoFPSfrom();

			detect();

			ON_SLEEP;
			m_pT->autoFPSto();
		}
	}

	void _Contour::detect(void)
	{
		IF_(check() < 0);

		Mat mBGR = *(m_pV->getFrameRGB()->m());	
		vector<vector<Point>> vvContours;
		findContours(mBGR, vvContours, RETR_EXTERNAL, CHAIN_APPROX_NONE);

		_Object o;
		vector<Point> vPoly;
		float kx = 1.0 / (float)mBGR.cols;
		float ky = 1.0 / (float)mBGR.rows;
		for (unsigned int i = 0; i < vvContours.size(); i++)
		{
			vPoly.clear();
			approxPolyDP(vvContours[i], vPoly, 3, true);
			Rect r = boundingRect(vPoly);

			o.clear();
			o.setTstamp(m_pT->getTfrom());
			o.setBB2D(rect2BB<vFloat4>(r));
			o.scale(kx, ky);
			o.setTopClass(0, o.area());

			m_pU->add(o);
			LOG_I("ID: " + i2str(o.getTopClass()));
		}

		m_pU->swap();
	}

	void _Contour::draw(void *pFrame)
	{
		NULL_(pFrame);
		this->_DetectorBase::draw(pFrame);
		IF_(check() < 0);
	}
}
