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

	int _InRange::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	void _InRange::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			filter();
		}
	}

	void _InRange::filter(void)
	{
		NULL_(m_pV);
		Frame* pF = m_pV->getFrameRGB();
		NULL_(pF);
		IF_(pF->bEmpty());
//		IF_(m_fRGB.tStamp() >= pF->tStamp());

		Mat m;
		cv::inRange(*pF->m(),
					cv::Scalar(m_vL.x, m_vL.y, m_vL.z),
					cv::Scalar(m_vH.x, m_vH.y, m_vH.z), m);

		m_fRGB.copy(m);
	}

}
