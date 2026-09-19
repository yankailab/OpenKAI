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

	bool _Crop::init(const json &j)
	{
		IF_F(!_VisionBase::init(j));

		jKv<int>(j, "vRoi", m_vRoi);

		return true;
	}

	bool _Crop::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_VisionBase::link(j, pM));

		string n = "";
		jKv(j, "_VisionBase", n);
		m_pV = (_VisionBase *)(pM->findModule(n));
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
		Mat *pM = m_pV->getMatRGB();
		NULL_(pM);
		IF_(pM->empty());

		Mat mIn = *pM;
		Rect r;
		r.x = constrain(m_vRoi.x(), 0, mIn.cols);
		r.y = constrain(m_vRoi.y(), 0, mIn.rows);
		r.width = m_vRoi.z() - r.x;
		r.height = m_vRoi.w() - r.y;

		m_vSizeRGB.x() = r.width;
		m_vSizeRGB.y() = r.height;

		mIn(r).copyTo(m_mRGB);
	}

}
