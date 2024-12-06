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
		close();
	}

	int _DepthShow::init(void *pKiss)
	{
		CHECK_(_VisionBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		string n;
		n = "";
		pK->v("_RGBDbase", &n);
		m_pV = (_RGBDbase *)(pK->findModule(n));
		NULL__(m_pV, OK_ERR_NOT_FOUND);

		return OK_OK;
	}

	bool _DepthShow::open(void)
	{
		NULL_F(m_pV);
		m_bOpen = m_pV->isOpened();

		return m_bOpen;
	}

	void _DepthShow::close(void)
	{
		this->_VisionBase::close();
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
			if (!m_bOpen)
				open();

			m_pT->autoFPS();

			if (m_bOpen)
			{
				filter();
			}

		}
	}

	void _DepthShow::filter(void)
	{
//		IF_(m_pV->DepthShow()->bEmpty());

//		Mat mD = *m_pV->DepthShow()->m();
//		m_fRGB.copy(mD);
	}

}
