/*
 * _Invert.cpp
 *
 *  Created on: March 14, 2019
 *      Author: yankai
 */

#include "_Invert.h"

namespace kai
{

	_Invert::_Invert()
	{
		m_type = vision_invert;
		m_pV = NULL;
	}

	_Invert::~_Invert()
	{
		close();
	}

	bool _Invert::init(void *pKiss)
	{
		IF_F(!_VisionBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		string n;
		n = "";
		pK->v("_VisionBase", &n);
		m_pV = (_VisionBase *)(pK->findModule(n));
		IF_Fl(!m_pV, n + ": not found");

		return true;
	}

	bool _Invert::open(void)
	{
		NULL_F(m_pV);
		m_bOpen = m_pV->isOpened();

		return m_bOpen;
	}

	void _Invert::close(void)
	{
		this->_VisionBase::close();
	}

	bool _Invert::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _Invert::update(void)
	{
		while (m_pT->bAlive())
		{
			if (!m_bOpen)
				open();

			m_pT->autoFPSfrom();

			if (m_bOpen)
			{
				filter();
			}

			m_pT->autoFPSto();
		}
	}

	void _Invert::filter(void)
	{
		IF_(m_pV->getFrameRGB()->bEmpty());

		Mat m;
		cv::bitwise_not(*m_pV->getFrameRGB()->m(), m);
		m_fRGB.copy(m);
	}

}
