/*
 * _Resize.cpp
 *
 *  Created on: April 23, 2019
 *      Author: yankai
 */

#include "_Resize.h"

namespace kai
{

	_Resize::_Resize()
	{
		m_type = vision_resize;
		m_pV = nullptr;
	}

	_Resize::~_Resize()
	{
		close();
	}

	int _Resize::init(void *pKiss)
	{
		CHECK_(_VisionBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		string n;
		n = "";
		pK->v("_VisionBase", &n);
		m_pV = (_VisionBase *)(pK->findModule(n));
		NULL__(m_pV, OK_ERR_NULLPTR);

		return OK_OK;
	}

	bool _Resize::open(void)
	{
		NULL_F(m_pV);
		m_bOpen = m_pV->isOpened();

		return m_bOpen;
	}

	void _Resize::close(void)
	{
		this->_VisionBase::close();
	}

	int _Resize::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	void _Resize::update(void)
	{
		while (m_pT->bAlive())
		{
			if (!m_bOpen)
				open();

			m_pT->autoFPS();

			if (m_bOpen)
			{
				if (m_fRGB.tStamp() < m_pV->getFrameRGB()->tStamp())
					filter();
			}

		}
	}

	void _Resize::filter(void)
	{
		IF_(m_pV->getFrameRGB()->bEmpty());

		m_fRGB.copy(m_pV->getFrameRGB()->resize(m_vSizeRGB.x, m_vSizeRGB.y));
	}

}
