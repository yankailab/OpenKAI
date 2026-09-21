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

	bool _Contrast::init(const json &j)
	{
		IF_F(!_VisionBase::init(j));

		jKv(j, "alpha", m_alpha);
		jKv(j, "beta", m_beta);

		return true;
	}

	bool _Contrast::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_VisionBase::link(j, pM));

		string n = "";
		jKv(j, "_VisionBase", n);
		m_pV = (_VisionBase *)(pM->findModule(n));
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
