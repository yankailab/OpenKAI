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
	}

	_Contrast::~_Contrast()
	{
	}

	bool _Contrast::loadConfig(void)
	{
		IF_F(!_VisionBase::loadConfig());
		const json &j = *m_pJ;

		jKv(j, "alpha", m_alpha);
		jKv(j, "beta", m_beta);

		return true;
	}

	bool _Contrast::saveConfig(bool bExport)
	{
		if (!_VisionBase::saveConfig(false))
		{
			return false;
		}

		json &j = *m_pJ;
		j["alpha"] = m_alpha;
		j["beta"] = m_beta;

		if (!bExport)
		{
			return true;
		}
		return m_pJcfg->saveToFile();
	}

	bool _Contrast::link(void)
	{
		IF_F(!this->_VisionBase::link());
		const json &j = *m_pJ;

		string n = "";
		jKv(j, "_VisionBase", n);
		m_pV = (_VisionBase *)(m_pM->findModule(n));
		NULL_F(m_pV);

		return true;
	}

	bool _Contrast::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	void _Contrast::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			filter();
		}
	}

	void _Contrast::filter(void)
	{
		NULL_(m_pV);
		Mat mIn;
		m_pV->copyMatRGB(mIn);
		IF_(mIn.empty());

		std::lock_guard<std::mutex> lock(m_mutexRGB);
		mIn.convertTo(m_mRGB, -1, m_alpha, m_beta);
	}

}
