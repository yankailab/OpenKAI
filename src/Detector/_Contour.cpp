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

	bool _Contour::saveConfig(bool bExport)
	{
		IF_F(!_DetectorBase::saveConfig(false));

		json &j = *m_pJ;
		j["mode"] = m_mode;
		j["method"] = m_method;

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _Contour::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	bool _Contour::check(void)
	{
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

		Mat mBGR;
		m_pRGBin->get(mBGR);

		IF_(mBGR.empty());
		vector<vector<Point>> vvContours;
		findContours(mBGR, vvContours, m_mode, m_method);

		vector<BBOX_OBJ> vBB;
		for (unsigned int i = 0; i < vvContours.size(); i++)
		{
			vector<Point> vPoly;
			approxPolyDP(vvContours[i], vPoly, 3, true);
			Rect r = boundingRect(vPoly);

			BBOX_OBJ bb;
			bb.setType(obj_bbox);
			bb.setPos(Vector3f(r.x, r.y, 0));
			bb.setDim(Vector3f(r.width, r.height, 0));
			bb.addClass(0);

			vBB.push_back(bb);
		}

		m_pBBout->add(vBB);
	}
}
