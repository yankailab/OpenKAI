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

	int _Crop::init(void *pKiss)
	{
		CHECK_(_VisionBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("vRoi", &m_vRoi);

		return OK_OK;
	}

	int _Crop::link(void)
	{
		CHECK_(this->_VisionBase::link());
		Kiss *pK = (Kiss *)m_pKiss;

		string n;
		n = "";
		pK->v("_VisionBase", &n);
		m_pV = (_VisionBase *)(pK->findModule(n));
		NULL__(m_pV, OK_ERR_NOT_FOUND);

		return OK_OK;
	}

	int _Crop::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
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
