/*
 * _DepthShow.cpp
 *
 *  Created on: April 23, 2019
 *      Author: yankai
 */

#include "_DepthShow.h"

namespace kai
{

	_DepthShow::_DepthShow()
	{
		m_type = vision_RSdepth;
		m_pV = nullptr;
	}

	_DepthShow::~_DepthShow()
	{
	}

	int _DepthShow::init(void *pKiss)
	{
		CHECK_(_VisionBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		return OK_OK;
	}

	int _DepthShow::link(void)
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

	int _DepthShow::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	void _DepthShow::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			filter();
		}
	}

	void _DepthShow::filter(void)
	{
		NULL_(m_pV);
		Frame *pF = m_pV->getFrameRGB();
		IF_(pF->bEmpty());
		IF_(m_fRGB.tStamp() >= pF->tStamp());

		//		IF_(m_pV->DepthShow()->bEmpty());

		//		Mat mD = *m_pV->DepthShow()->m();
		//		m_fRGB.copy(mD);
	}

}
