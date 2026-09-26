/*
 * _Resize.cpp
 *
 *  Created on: April 23, 2019
 *      Author: yankai
 */

#include "_Resize.h"

namespace kai
{

	_Resize::_Resize()
	{
		m_type = vision_resize;
	}

	_Resize::~_Resize()
	{
	}

	bool _Resize::loadConfig(void)
	{
		IF_F(!_VisionBase::loadConfig());

		return true;
	}

	bool _Resize::saveConfig(bool bExport)
	{
		IF_F(!_VisionBase::saveConfig(false));

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _Resize::link(void)
	{
		IF_F(!this->_VisionBase::link());
		const json &j = *m_pJ;

		string n = "";
		jKv(j, "_VisionBase", n);
		m_pV = (_VisionBase *)(m_pM->findModule(n));
		NULL_F(m_pV);

		return true;
	}

	bool _Resize::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	void _Resize::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			filter();
		}
	}

	void _Resize::filter(void)
	{
		NULL_(m_pV);
		Mat mIn;
		m_pV->copyMatRGB(mIn);
		IF_(mIn.empty());

		std::lock_guard<std::mutex> lock(m_mutexRGB);
		cv::resize(mIn, m_mRGB, cv::Size(m_vSizeRGB.x(), m_vSizeRGB.y()));
	}

}
