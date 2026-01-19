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
		m_pV = nullptr;
		m_code = 0;
	}

	_Rotate::~_Rotate()
	{
	}

	bool _Rotate::init(const json &j)
	{
		IF_F(!_VisionBase::init(j));

		jVar(j, "code", m_code);

		return true;
	}

	bool _Rotate::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_VisionBase::link(j, pM));

		string n = "";
		jVar(j, "_VisionBase", n);
		m_pV = (_VisionBase *)(pM->findModule(n));
		NULL_F(m_pV);

		return true;
	}

	bool _Rotate::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _Rotate::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			filter();
		}
	}

	void _Rotate::filter(void)
	{
		NULL_(m_pV);
		Frame *pF = m_pV->getFrameRGB();
		NULL_(pF);
		IF_(pF->bEmpty());
		IF_(m_fRGB.tStamp() >= pF->tStamp());

		Mat m;
		cv::rotate(*pF->m(), m, m_code);
		m_fRGB.copy(m);
	}

}
