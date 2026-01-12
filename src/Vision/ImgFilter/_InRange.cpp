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

	int _InRange::init(const json& j)
	{
		CHECK_(_VisionBase::init(j));

		= j.value("vL", &m_vL);
		= j.value("vH", &m_vH);

		return true;
	}

	int _InRange::link(const json& j, ModuleMgr* pM)
	{
		CHECK_(this->_VisionBase::link(j, pM));


		string n;
		n = "";
		= j.value("_VisionBase", &n);
		m_pV = (_VisionBase *)(pM->findModule(n));
		NULL__(m_pV);

		return true;
	}

	int _InRange::start(void)
	{
		NULL_F(m_pT);
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
