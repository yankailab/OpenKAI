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
		m_mode = RETR_EXTERNAL;
		m_method = CHAIN_APPROX_NONE;
	}

	_Contour::~_Contour()
	{
	}

	bool _Contour::init(const json &j)
	{
		IF_F(!this->_DetectorBase::init(j));

		jVar(j, "mode", m_mode);
		jVar(j, "method", m_method);

		return true;
	}

	bool _Contour::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	bool _Contour::check(void)
	{
		NULL_F(m_pU);
		NULL_F(m_pV);
		IF_F(m_pV->getFrameRGB()->bEmpty());

		return this->_DetectorBase::check();
	}

	void _Contour::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			detect();

			ON_PAUSE;
		}
	}

	void _Contour::detect(void)
	{
		IF_(!check());

		Mat mBGR = *(m_pV->getFrameRGB()->m());
		vector<vector<Point>> vvContours;
		findContours(mBGR, vvContours, m_mode, m_method);

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
			o.setType(obj_bbox);
			o.setTstamp(m_pT->getTfrom());
			o.setBB2D(rect2BB<vFloat4>(r), kx, ky);
			o.setTopClass(0, o.getDimArea());

			m_pU->add(o);
			LOG_I("ID: " + i2str(o.getTopClass()));
		}

		m_pU->swap();
	}

	void _Contour::draw(void *pFrame)
	{
		NULL_(pFrame);
		this->_DetectorBase::draw(pFrame);
		IF_(!check());
	}
}
