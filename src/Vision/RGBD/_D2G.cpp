/*
 * _D2G.cpp
 *
 *  Created on: April 23, 2019
 *      Author: yankai
 */

#include "_D2G.h"

namespace kai
{

	_D2G::_D2G()
	{
		m_type = vision_depth2Gray;
		m_pV = nullptr;
	}

	_D2G::~_D2G()
	{
	}

	bool _D2G::init(const json& j)
	{
		IF_F(!_VisionBase::init(j));

		return true;
	}

	bool _D2G::link(const json& j, ModuleMgr* pM)
	{
		IF_F(!this->_VisionBase::link(j, pM));

		string n;
		n = "";
		n = j.value("_RGBDbase", "");
		m_pV = (_RGBDbase *)(pM->findModule(n));
		NULL__(m_pV);

		return true;
	}

	bool _D2G::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _D2G::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			filter();
		}
	}

	void _D2G::filter(void)
	{
		NULL_(m_pV);
		Frame* pF = m_pV->getFrameD();
		NULL_(pF);
		IF_(pF->bEmpty());
//		IF_(m_fRGB.tStamp() >= pF->tStamp());

		Mat mD = *pF->m();
		Mat mGray;
		float scale = 255.0 / m_pV->getRangeD().len();
		mD.convertTo(mGray,
					 CV_8UC1,
					 scale,
					 -m_pV->getRangeD().x * scale);
		m_fRGB.copy(mGray);
	}
}
