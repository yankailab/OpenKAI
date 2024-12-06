/*
 * _Contrast.cpp
 *
 *  Created on: March 12, 2019
 *      Author: yankai
 */

#include "_Contrast.h"

namespace kai
{

	_Contrast::_Contrast()
	{
		m_type = vision_contrast;
		m_pV = nullptr;

		m_alpha = 1.0;
		m_beta = 0.0;
	}

	_Contrast::~_Contrast()
	{
		close();
	}

	int _Contrast::init(void *pKiss)
	{
		CHECK_(_VisionBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("alpha", &m_alpha);
		pK->v("beta", &m_beta);

		string n;
		n = "";
		pK->v("_VisionBase", &n);
		m_pV = (_VisionBase *)(pK->findModule(n));
		NULL__(m_pV, OK_ERR_NOT_FOUND);

		return OK_OK;
	}

	bool _Contrast::open(void)
	{
		NULL_F(m_pV);
		m_bOpen = m_pV->isOpened();

		return m_bOpen;
	}

	void _Contrast::close(void)
	{
		this->_VisionBase::close();
	}

	int _Contrast::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	void _Contrast::update(void)
	{
		while (m_pT->bAlive())
		{
			if (!m_bOpen)
				open();

			m_pT->autoFPS();

			if (m_bOpen)
			{
				filter();
			}

		}
	}

	void _Contrast::filter(void)
	{
		IF_(m_pV->getFrameRGB()->bEmpty());

		Mat m;
		m_pV->getFrameRGB()->m()->convertTo(m, -1, m_alpha, m_beta);
		m_fRGB.copy(m);
	}

}
