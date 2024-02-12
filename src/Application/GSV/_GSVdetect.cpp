/*
 * _GSVdetect.cpp
 *
 *  Created on: Jan 14, 2024
 *      Author: yankai
 */

#include "_GSVdetect.h"

namespace kai
{

	_GSVdetect::_GSVdetect()
	{
	}

	_GSVdetect::~_GSVdetect()
	{
	}

	bool _GSVdetect::init(void *pKiss)
	{
		IF_F(!this->_PCgrid::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		return true;
	}

	bool _GSVdetect::link(void)
	{
		IF_F(!this->_PCgrid::link());
		Kiss *pK = (Kiss *)m_pKiss;

        vector<string> vN;
        pK->a("vPCstream", &vN);
		for(string n : vN)
		{
	        _PCstream* pS = (_PCstream *)(pK->getInst(n));
			IF_CONT(pS);

			m_vpS.push_back(pS);
		}

		return true;
	}

	bool _GSVdetect::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _GSVdetect::check(void)
	{
//        NULL__(m_pV, -1);

		return this->_PCgrid::check();
	}

	void _GSVdetect::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			detect();

			m_pT->autoFPSto();
		}
	}

	void _GSVdetect::detect(void)
	{
		IF_(check() < 0);

		for(_PCstream* pS : m_vpS)
		{

		}


/*
		Mat mR = *(m_pV->getFrameRGB()->m());

		vector<vector<Point>> vvContours;
		findContours(mR, vvContours, RETR_EXTERNAL, CHAIN_APPROX_NONE);

		_Object o;
		float kx = 1.0 / (float)mR.cols;
		float ky = 1.0 / (float)mR.rows;

		vector<Point> vPoly;
		for (unsigned int i = 0; i < vvContours.size(); i++)
		{
			vPoly.clear();
			approxPolyDP(vvContours[i], vPoly, 3, true);
			Rect r = boundingRect(vPoly);

			o.init();
			o.setTstamp(m_pT->getTfrom());
			o.setBB2D(rect2BB<vFloat4>(r));
			o.scale(kx, ky);
			o.setTopClass(0, o.area());
//			o.setDepth();

			m_pU->add(o);
			LOG_I("ID: " + i2str(o.getTopClass()));
		}

		m_pU->swap();
*/
	}

}
