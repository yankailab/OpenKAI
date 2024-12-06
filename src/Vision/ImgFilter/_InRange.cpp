/*
 * _InRange.cpp
 *
 *  Created on: April 23, 2019
 *      Author: yankai
 */

#include "_InRange.h"

namespace kai
{

	_InRange::_InRange()
	{
		m_type = vision_inRange;
		m_pV = nullptr;
		m_vL.set(0, 0, 0);
		m_vH.set(255, 255, 255);
	}

	_InRange::~_InRange()
	{
		close();
	}

	int _InRange::init(void *pKiss)
	{
		CHECK_(_VisionBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("vL", &m_vL);
		pK->v("vH", &m_vH);

		return OK_OK;
	}

	int _InRange::link(void)
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

	bool _InRange::open(void)
	{
		NULL_F(m_pV);
		m_bOpen = m_pV->isOpened();

		return m_bOpen;
	}

	void _InRange::close(void)
	{
		this->_VisionBase::close();
	}

	int _InRange::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	void _InRange::update(void)
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

	void _InRange::filter(void)
	{
		IF_(m_pV->getFrameRGB()->bEmpty());

		Mat m;
		cv::inRange(*m_pV->getFrameRGB()->m(),
					cv::Scalar(m_vL.x, m_vL.y, m_vL.z),
					cv::Scalar(m_vH.x, m_vH.y, m_vH.z), m);

		m_fRGB.copy(m);
	}

}
