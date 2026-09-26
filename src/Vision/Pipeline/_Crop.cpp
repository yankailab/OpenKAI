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

		m_vRoi.setZero();
	}

	_Crop::~_Crop()
	{
	}

	bool _Crop::loadConfig(void)
	{
		IF_F(!_VisionBase::loadConfig());
		const json &j = *m_pJ;

		jKv<int>(j, "vRoi", m_vRoi);

		return true;
	}

	bool _Crop::link(void)
	{
		IF_F(!this->_VisionBase::link());
		const json &j = *m_pJ;

		string n = "";
		jKv(j, "_VisionBase", n);
		m_pV = (_VisionBase *)(m_pM->findModule(n));
		NULL_F(m_pV);

		return true;
	}

	bool _Crop::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	void _Crop::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			filter();
		}
	}

	void _Crop::filter(void)
	{
		NULL_(m_pV);
		Mat mIn;
		m_pV->copyMatRGB(mIn);
		IF_(mIn.empty());

		Rect r;
		r.x = constrain(m_vRoi.x(), 0, mIn.cols);
		r.y = constrain(m_vRoi.y(), 0, mIn.rows);
		r.width = m_vRoi.z() - r.x;
		r.height = m_vRoi.w() - r.y;

		m_vSizeRGB.x() = r.width;
		m_vSizeRGB.y() = r.height;

		std::lock_guard<std::mutex> lock(m_mutexRGB);
		mIn(r).copyTo(m_mRGB);
	}

}
