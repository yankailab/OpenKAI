/*
 * _Rotate.cpp
 *
 *  Created on: April 23, 2019
 *      Author: yankai
 */

#include "_Rotate.h"

namespace kai
{

	_Rotate::_Rotate()
	{
		m_type = vision_rotate;
		m_pV = nullptr;
		m_code = 0;
	}

	_Rotate::~_Rotate()
	{
		close();
	}

	int _Rotate::init(void *pKiss)
	{
		CHECK_(_VisionBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("code", &m_code);

		string n;
		n = "";
		pK->v("_VisionBase", &n);
		m_pV = (_VisionBase *)(pK->findModule(n));
		NULL__(m_pV, OK_ERR_NOT_FOUND);

		return OK_OK;
	}

	bool _Rotate::open(void)
	{
		NULL_F(m_pV);
		m_bOpen = m_pV->isOpened();

		return m_bOpen;
	}

	void _Rotate::close(void)
	{
		this->_VisionBase::close();
	}

	int _Rotate::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	void _Rotate::update(void)
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

	void _Rotate::filter(void)
	{
		IF_(m_pV->getFrameRGB()->bEmpty());

		Mat m;
		cv::rotate(*m_pV->getFrameRGB()->m(), m, m_code);
		m_fRGB.copy(m);
	}

}
