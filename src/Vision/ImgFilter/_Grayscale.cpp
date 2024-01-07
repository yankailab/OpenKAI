/*
 * _Grayscale.cpp
 *
 *  Created on: March 12, 2019
 *      Author: yankai
 */

#include "_Grayscale.h"

namespace kai
{

	_Grayscale::_Grayscale()
	{
		m_type = vision_grayscale;
		m_pV = NULL;
	}

	_Grayscale::~_Grayscale()
	{
		close();
	}

	bool _Grayscale::init(void *pKiss)
	{
		IF_F(!_VisionBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		string n;
		n = "";
		pK->v("_VisionBase", &n);
		m_pV = (_VisionBase *)(pK->getInst(n));
		IF_Fl(!m_pV, n + ": not found");

		return true;
	}

	bool _Grayscale::open(void)
	{
		NULL_F(m_pV);
		m_bOpen = m_pV->isOpened();

		return m_bOpen;
	}

	void _Grayscale::close(void)
	{
		this->_VisionBase::close();
	}

	bool _Grayscale::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _Grayscale::update(void)
	{
		while (m_pT->bRun())
		{
			if (!m_bOpen)
				open();

			m_pT->autoFPSfrom();

			if (m_bOpen)
			{
				if (m_fRGB.tStamp() < m_pV->getFrameRGB()->tStamp())
				{
					filter();
				}
			}

			m_pT->autoFPSto();
		}
	}

	void _Grayscale::filter(void)
	{
		IF_(m_pV->getFrameRGB()->bEmpty());

		m_fRGB.copy(m_pV->getFrameRGB()->cvtColor(COLOR_RGB2GRAY));
	}

}
