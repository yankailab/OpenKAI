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

	bool _Contour::loadConfig(void)
	{
		IF_F(!this->_DetectorBase::loadConfig());
		const json &j = *m_pJ;

		jKv(j, "mode", m_mode);
		jKv(j, "method", m_method);

		return true;
	}

	bool _Contour::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	bool _Contour::check(void)
	{
		NULL_F(m_pCanvas);
		NULL_F(m_pV);
		IF_F(m_pV->getMatRGB()->empty());

		return this->_DetectorBase::check();
	}

	void _Contour::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			detect();

			ON_PAUSE;
		}
	}

	void _Contour::detect(void)
	{
		IF_(!check());

		Mat mBGR = *m_pV->getMatRGB();
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
			o.setTstamp(m_pT->getTfromNs());
			o.setBB2D(rect2BB<Vector4f>(r), kx, ky);
			o.setTopClass(0, o.getDimArea());

			m_pCanvas->add(o);
			LOG_I("ID: " + i2str(o.getTopClass()));
		}

		m_pCanvas->swap();
	}

	void _Contour::draw(void *pMat)
	{
		NULL_(pMat);
		this->_DetectorBase::draw(pMat);
		IF_(!check());
	}
}
