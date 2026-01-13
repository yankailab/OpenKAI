/*
 * _Crop.cpp
 *
 *  Created on: April 23, 2019
 *      Author: yankai
 */

#include "_Crop.h"

namespace kai
{

	_Crop::_Crop()
	{
		m_type = vision_crop;
		m_pV = nullptr;

		m_vRoi.clear();
	}

	_Crop::~_Crop()
	{
	}

	bool _Crop::init(const json& j)
	{
		IF_F(!_VisionBase::init(j));

		m_vRoi = j.value("vRoi", "");

		return true;
	}

	bool _Crop::link(const json& j, ModuleMgr* pM)
	{
		IF_F(!this->_VisionBase::link(j, pM));

		string n;
		n = "";
		n = j.value("_VisionBase", "");
		m_pV = (_VisionBase *)(pM->findModule(n));
		NULL_F(m_pV);

		return true;
	}

	bool _Crop::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _Crop::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			filter();
		}
	}

	void _Crop::filter(void)
	{
		NULL_(m_pV);
		Frame* pF = m_pV->getFrameRGB();
		NULL_(pF);
		IF_(pF->bEmpty());
//		IF_(m_fRGB.tStamp() >= pF->tStamp());

		Mat mIn = *pF->m();
		Rect r;
		r.x = constrain(m_vRoi.x, 0, mIn.cols);
		r.y = constrain(m_vRoi.y, 0, mIn.rows);
		r.width = m_vRoi.z - r.x;
		r.height = m_vRoi.w - r.y;

		m_vSizeRGB.x = r.width;
		m_vSizeRGB.y = r.height;

		m_fRGB.copy(mIn(r));
	}

}
