/*
 * _ColorConvert.cpp
 *
 *  Created on: March 12, 2019
 *      Author: yankai
 */

#include "_ColorConvert.h"

namespace kai
{

	_ColorConvert::_ColorConvert()
	{
		m_type = vision_ColorConvert;
		m_pV = NULL;
		m_code = COLOR_RGB2GRAY;
	}

	_ColorConvert::~_ColorConvert()
	{
		close();
	}

	bool _ColorConvert::init(void *pKiss)
	{
		IF_F(!_VisionBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("code", &m_code);

		return true;
	}

	bool _ColorConvert::link(void)
	{
		IF_F(!this->_VisionBase::link());
		Kiss *pK = (Kiss *)m_pKiss;

		string n;
		n = "";
		pK->v("_VisionBase", &n);
		m_pV = (_VisionBase *)(pK->findModule(n));
		IF_Fl(!m_pV, n + ": not found");

		return true;
	}

	bool _ColorConvert::open(void)
	{
		NULL_F(m_pV);
		m_bOpen = m_pV->isOpened();

		return m_bOpen;
	}

	void _ColorConvert::close(void)
	{
		this->_VisionBase::close();
	}

	bool _ColorConvert::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _ColorConvert::check(void)
	{
		NULL__(m_pV, -1);
		IF__(m_pV->getFrameRGB()->bEmpty(), -1);

		return _VisionBase::check();
	}

	void _ColorConvert::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPSfrom();

			if (!m_bOpen)
			{
				open();
			}

			filter();

			m_pT->autoFPSto();
		}
	}

	void _ColorConvert::filter(void)
	{
		IF_(check() < 0);

		m_fRGB.copy(m_pV->getFrameRGB()->cvtColor(m_code));
	}

}
