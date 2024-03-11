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
		m_vDetRange.set(200,255);
	}

	_Thermal::~_Thermal()
	{
	}

	bool _Thermal::init(void *pKiss)
	{
		IF_F(!this->_DetectorBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("vDetRange", &m_vDetRange);

		return true;
	}

	bool _Thermal::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _Thermal::check(void)
	{
		NULL__(m_pU, -1);
		NULL__(m_pV, -1);
		IF__(m_pV->getFrameRGB()->bEmpty(), -1);

		return this->_DetectorBase::check();
	}

	void _Thermal::update(void)
	{
		while (m_pT->bThread())
		{
			m_pT->autoFPSfrom();

			detect();

			ON_SLEEP;
			m_pT->autoFPSto();
		}
	}

	void _Thermal::detect(void)
	{
		IF_(check() < 0);

		Mat mBGR = *(m_pV->getFrameRGB()->m());
		Mat mGray;
		cv::cvtColor(mBGR, mGray, COLOR_BGR2GRAY);
		cv::inRange(mGray, m_vDetRange.x, m_vDetRange.y, m_mR);

		vector<vector<Point>> vvContours;
		findContours(m_mR, vvContours, RETR_EXTERNAL, CHAIN_APPROX_NONE);

		_Object o;
		vector<Point> vPoly;
		for (unsigned int i = 0; i < vvContours.size(); i++)
		{
			vPoly.clear();
			approxPolyDP(vvContours[i], vPoly, 3, true);
			Rect r = boundingRect(vPoly);

			o.clear();
			o.setTstamp(m_pT->getTfrom());
			o.setBB2D(rect2BB<vFloat4>(r));
			o.scale(m_mR.cols, m_mR.rows);
			o.setTopClass(0, o.area());

			m_pU->add(o);
			LOG_I("ID: " + i2str(o.getTopClass()));
		}

		m_pU->swap();
	}

	void _Thermal::draw(void *pFrame)
	{
		NULL_(pFrame);
		this->_DetectorBase::draw(pFrame);
		IF_(check() < 0);

		IF_(m_mR.empty());
		IF_(!m_bDebug);

		Frame *pF = (Frame *)pFrame;
		pF->copy(m_mR);
	}
}
