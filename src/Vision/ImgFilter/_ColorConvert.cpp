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
		m_pV = nullptr;
		m_code = COLOR_RGB2GRAY;
	}

	_ColorConvert::~_ColorConvert()
	{
		close();
	}

	int _ColorConvert::init(void *pKiss)
	{
		CHECK_(_VisionBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("code", &m_code);

		return OK_OK;
	}

	int _ColorConvert::link(void)
	{
		CHECK_(this->_VisionBase::link());
		Kiss *pK = (Kiss *)m_pKiss;

		string n;
		n = "";
		pK->v("_VisionBase", &n);
		m_pV = (_VisionBase *)(pK->findModule(n));
		NULL__(m_pV, OK_ERR_NOT_FOUND);

		return OK_OK;
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

	int _ColorConvert::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	int _ColorConvert::check(void)
	{
		NULL__(m_pV, OK_ERR_NULLPTR);
		IF__(m_pV->getFrameRGB()->bEmpty(), OK_ERR_NOT_READY);

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
		IF_(check() != OK_OK);

		m_fRGB.copy(m_pV->getFrameRGB()->cvtColor(m_code));
	}

}
