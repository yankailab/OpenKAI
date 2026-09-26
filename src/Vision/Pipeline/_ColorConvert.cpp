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

	bool _ColorConvert::loadConfig(void)
	{
		IF_F(!_VisionBase::loadConfig());
		const json &j = *m_pJ;

		jKv(j, "code", m_code);

		return true;
	}

	bool _ColorConvert::saveConfig(bool bExport)
	{
		if (!_VisionBase::saveConfig(false))
		{
			return false;
		}

		json &j = *m_pJ;
		j["code"] = m_code;

		if (!bExport)
		{
			return true;
		}
		return m_pJcfg->saveToFile();
	}

	bool _ColorConvert::link(void)
	{
		IF_F(!this->_VisionBase::link());
		const json &j = *m_pJ;

		string n = "";
		jKv(j, "_VisionBase", n);
		m_pV = (_VisionBase *)(m_pM->findModule(n));
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
