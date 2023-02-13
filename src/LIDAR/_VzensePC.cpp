/*
 * _VzensePC.cpp
 *
 *  Created on: Feb 13, 2023
 *      Author: yankai
 */

#include "_VzensePC.h"

namespace kai
{

	_VzensePC::_VzensePC()
	{
		m_rsSN = "";
		m_vPreset = "Short Range";
		m_bRsRGB = true;
		m_rsFPS = 30;
		m_rsDFPS = 30;
		m_fFilterManitude = 0.0;
		m_fHolesFill = 0.0;
		m_bAlign = false;
		m_fEmitter = 1.0;
		m_fLaserPower = 1.0;

		m_bOpen = false;
		m_vWHc.set(1280, 720);
		m_vWHd.set(640, 480);
		m_vRz.set(0.0, FLT_MAX);
	}

	_VzensePC::~_VzensePC()
	{
	}

	bool _VzensePC::init(void *pKiss)
	{
		IF_F(!_PCframe::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("rsSN", &m_rsSN);
		pK->v("rsFPS", &m_rsFPS);
		pK->v("rsDFPS", &m_rsDFPS);
		pK->v("vPreset", &m_vPreset);
		pK->v("bRsRGB", &m_bRsRGB);
		pK->v("fDec", &m_fFilterManitude);
		pK->v("fSpat", &m_fHolesFill);
		pK->v("bAlign", &m_bAlign);
		pK->v("fEmitter", &m_fEmitter);
		pK->v("fLaserPower", &m_fLaserPower);

		pK->v("vRz", &m_vRz);
		pK->v("vWHcol", &m_vWHc);
		pK->v("vWHd", &m_vWHd);

		return true;
	}

	bool _VzensePC::open(void)
	{
		IF_T(m_bOpen);




		m_spImg = std::make_shared<geometry::Image>();
		m_spImg->Prepare(m_vWHc.x, m_vWHc.y, 3, 1);

		m_bOpen = true;
		return true;
	}

	void _VzensePC::sensorReset(void)
	{
	}

	bool _VzensePC::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _VzensePC::check(void)
	{
		return this->_PCframe::check();
	}

	void _VzensePC::update(void)
	{
		while (m_pT->bRun())
		{
			if (!m_bOpen)
			{
				if (!open())
				{
					LOG_E("Cannot open Vzense");
					sensorReset();
					m_pT->sleepT(SEC_2_USEC);
					continue;
				}
			}

			m_pT->autoFPSfrom();

			if (updateRS())
			{
				updatePC();
			}
			else
			{
				sensorReset();
				m_pT->sleepT(SEC_2_USEC);
				m_bOpen = false;
			}

			m_pT->autoFPSto();
		}
	}

	bool _VzensePC::updateRS(void)
	{
		IF_T(check() < 0);


		return true;
	}

}
