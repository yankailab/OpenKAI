/*
 * _ColorConvert.cpp
 *
 *  Created on: March 12, 2019
 *      Author: yankai
 */

#include "_ColorConvert.h"

namespace kai
{

	_ColorConvert::_ColorConvert()
	{
		m_type = vision_colorConvert;
	}

	_ColorConvert::~_ColorConvert()
	{
	}

	bool _ColorConvert::init(const json &j)
	{
		IF_F(!_VisionBase::init(j));

		jKv(j, "code", m_code);

		return true;
	}

	bool _ColorConvert::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_VisionBase::link(j, pM));

		string n = "";
		jKv(j, "_VisionBase", n);
		m_pV = (_VisionBase *)(pM->findModule(n));
		NULL_F(m_pV);

		return true;
	}

	bool _ColorConvert::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	void _ColorConvert::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			filter();
		}
	}

	void _ColorConvert::filter(void)
	{
		NULL_(m_pV);
		Mat mIn;
		m_pV->copyMatRGB(mIn);
		IF_(mIn.empty());

		std::lock_guard<std::mutex> lock(m_mutexRGB);
		cv::cvtColor(mIn, m_mRGB, m_code);
	}

}
