/*
 * _Thermal2RGB.cpp
 *
 *  Created on: April 23, 2019
 *      Author: yankai
 */

#include "_Thermal2RGB.h"

namespace kai
{

	_Thermal2RGB::_Thermal2RGB()
	{
		m_type = vision_depth2Gray;
		m_pV = nullptr;
		m_vTrange.set(0, 40);
	}

	_Thermal2RGB::~_Thermal2RGB()
	{
	}

	bool _Thermal2RGB::init(const json &j)
	{
		IF_F(!_VisionBase::init(j));

		jKv<float>(j, "vTrange", m_vTrange);

		return true;
	}

	bool _Thermal2RGB::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_VisionBase::link(j, pM));

		string n = "";
		jKv(j, "_VisionBase", n);
		m_pV = (_VisionBase *)(pM->findModule(n));
		NULL_F(m_pV);

		return true;
	}

	bool _Thermal2RGB::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _Thermal2RGB::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			filter();
		}
	}

	void _Thermal2RGB::filter(void)
	{
		NULL_(m_pV);
		Frame *pF = m_pV->getFrameRGB();
		NULL_(pF);

		Mat mT = *pF->m();
		IF_(mT.empty());
		IF_(mT.type() != CV_32FC1);

		Mat mClip;
		cv::min(cv::max(mT, m_vTrange.x), m_vTrange.y, mClip);

		Mat mGray;
		float tR = m_vTrange.y - m_vTrange.x;
		mClip.convertTo(mGray, CV_8UC1, 255.0 / tR, -m_vTrange.x * 255.0 / tR);

		Mat mC;
		applyColorMap(mGray, mC, cv::COLORMAP_JET);

		m_fRGB.copy(mC);
	}
}
