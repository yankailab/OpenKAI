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
	}

	_Rotate::~_Rotate()
	{
	}

	bool _Rotate::init(const json &j)
	{
		IF_F(!_VisionBase::init(j));

		jKv(j, "code", m_code);

		return true;
	}

	bool _Rotate::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_VisionBase::link(j, pM));

		string n = "";
		jKv(j, "_VisionBase", n);
		m_pV = (_VisionBase *)(pM->findModule(n));
		NULL_F(m_pV);

		return true;
	}

	bool _Rotate::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	void _Rotate::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			filter();
		}
	}

	void _Rotate::filter(void)
	{
		NULL_(m_pV);
		Mat mIn;
		m_pV->copyMatRGB(mIn);
		IF_(mIn.empty());

		std::lock_guard<std::mutex> lock(m_mutexRGB);
		cv::rotate(mIn, m_mRGB, m_code);
	}

}
