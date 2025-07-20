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

	int _D2G::init(void *pKiss)
	{
		CHECK_(_VisionBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		return OK_OK;
	}

	int _D2G::link(void)
	{
		CHECK_(this->_VisionBase::link());
		Kiss *pK = (Kiss *)m_pKiss;

		string n;
		n = "";
		pK->v("_RGBDbase", &n);
		m_pV = (_RGBDbase *)(pK->findModule(n));
		NULL__(m_pV, OK_ERR_NOT_FOUND);

		return OK_OK;
	}

	int _D2G::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
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
