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

	bool _Resize::init(const json &j)
	{
		IF_F(!_VisionBase::init(j));

		return true;
	}

	bool _Resize::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_VisionBase::link(j, pM));

		string n = "";
		jKv(j, "_VisionBase", n);
		m_pV = (_VisionBase *)(pM->findModule(n));
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
		Mat *pM = m_pV->getMatRGB();
		NULL_(pM);
		IF_(pM->empty());

		cv::resize(*pM, m_mRGB, cv::Size(m_vSizeRGB.x(), m_vSizeRGB.y()));
	}

}
