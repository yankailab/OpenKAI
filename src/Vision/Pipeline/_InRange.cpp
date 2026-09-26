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
		m_vL = Vector3i(0, 0, 0);
		m_vH = Vector3i(255, 255, 255);
	}

	_InRange::~_InRange()
	{
	}

	bool _InRange::loadConfig(void)
	{
		IF_F(!_VisionBase::loadConfig());
		const json &j = *m_pJ;

		jKv<int>(j, "vL", m_vL);
		jKv<int>(j, "vH", m_vH);

		return true;
	}

	bool _InRange::link(void)
	{
		IF_F(!this->_VisionBase::link());
		const json &j = *m_pJ;

		string n = "";
		jKv(j, "_VisionBase", n);
		m_pV = (_VisionBase *)(m_pM->findModule(n));
		NULL_F(m_pV);

		return true;
	}

	bool _InRange::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	void _InRange::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			filter();
		}
	}

	void _InRange::filter(void)
	{
		NULL_(m_pV);
		Mat mIn;
		m_pV->copyMatRGB(mIn);
		IF_(mIn.empty());

		std::lock_guard<std::mutex> lock(m_mutexRGB);
		cv::inRange(mIn,
					cv::Scalar(m_vL.x(), m_vL.y(), m_vL.z()),
					cv::Scalar(m_vH.x(), m_vH.y(), m_vH.z()), m_mRGB);
	}

}
